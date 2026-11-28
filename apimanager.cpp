#include "apimanager.h"
#include <QSettings>

ApiManager::ApiManager(QObject *parent) : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    baseUrl = "http://localhost:8000";

    QSettings settings("HobbyTracker", "DesktopClient");
    accessToken = settings.value("auth/access_token", "").toString();
    refreshToken = settings.value("auth/refresh_token", "").toString();
}

void ApiManager::saveTokens(const QString &access, const QString &refresh)
{
    accessToken = access;
    refreshToken = refresh;
    QSettings settings("HobbyTracker", "DesktopClient");
    settings.setValue("auth/access_token", accessToken);
    settings.setValue("auth/refresh_token", refreshToken);
    settings.sync();
}

void ApiManager::clearTokens()
{
    accessToken.clear();
    refreshToken.clear();
    QSettings settings("HobbyTracker", "DesktopClient");
    settings.remove("auth/access_token");
    settings.remove("auth/refresh_token");
    settings.sync();
}

QNetworkRequest ApiManager::createAuthorizedRequest(const QUrl &url) const
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!accessToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(accessToken).toUtf8());
    }
    return request;
}

QString ApiManager::extractErrorMessage(QNetworkReply *reply, const QByteArray &data, const QString &fallback) const
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("detail")) {
            if (obj["detail"].isString()) {
                return obj["detail"].toString();
            }
            if (obj["detail"].isArray() && !obj["detail"].toArray().isEmpty()) {
                QJsonObject firstErr = obj["detail"].toArray().first().toObject();
                if (firstErr.contains("msg")) {
                    return firstErr["msg"].toString();
                }
            }
        }
    }
    if (reply->error() != QNetworkReply::NoError) {
        return fallback;
    }
    return fallback;
}

void ApiManager::refreshSession()
{
    if (refreshToken.isEmpty()) {
        emit sessionExpired();
        return;
    }

    QUrl url(baseUrl + "/auth/refresh");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["refresh_token"] = refreshToken;

    QNetworkReply *reply = networkManager->post(request, QJsonDocument(json).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray responseData = reply->readAll();

        if (reply->error() == QNetworkReply::NoError) {
            QJsonObject jsonObj = QJsonDocument::fromJson(responseData).object();
            if (jsonObj.contains("access_token")) {
                QString newRefresh = jsonObj.contains("refresh_token")
                ? jsonObj["refresh_token"].toString()
                : refreshToken;
                saveTokens(jsonObj["access_token"].toString(), newRefresh);
                emit sessionRestored();
            } else {
                clearTokens();
                emit sessionExpired();
            }
        } else {
            if (statusCode == 401 || statusCode == 403 || statusCode == 422) {
                clearTokens();
            }
            emit sessionExpired();
        }
        reply->deleteLater();
    });
}

void ApiManager::login(const QString &username, const QString &password)
{
    if (useMock) {
        if (username == "admin" && password == "1234") {
            accessToken = "mock_jwt_token_12345";
            emit loginSuccess();
        } else {
            emit loginFailed("Неверный логин или пароль");
        }
        return;
    }

    QUrl url(baseUrl + "/auth/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["username"] = username;
    json["password"] = password;

    QNetworkReply *reply = networkManager->post(request, QJsonDocument(json).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        QByteArray responseData = reply->readAll();

        if (reply->error() == QNetworkReply::NoError) {
            QJsonObject jsonObj = QJsonDocument::fromJson(responseData).object();
            if (jsonObj.contains("access_token")) {
                saveTokens(
                    jsonObj["access_token"].toString(),
                    jsonObj["refresh_token"].toString()
                    );
                emit loginSuccess();
            } else {
                emit loginFailed("Сервер не вернул токен доступа");
            }
        } else {
            emit loginFailed(extractErrorMessage(reply, responseData, "Неверный логин или пароль"));
        }
        reply->deleteLater();
    });
}

void ApiManager::registerUser(const QString &username, const QString &password)
{
    QUrl url(baseUrl + "/auth/register");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["username"] = username;
    json["password"] = password;

    QNetworkReply *reply = networkManager->post(request, QJsonDocument(json).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        QByteArray responseData = reply->readAll();

        if (reply->error() == QNetworkReply::NoError) {
            QJsonObject jsonObj = QJsonDocument::fromJson(responseData).object();
            if (jsonObj.contains("access_token") && jsonObj.contains("refresh_token")) {
                saveTokens(
                    jsonObj["access_token"].toString(),
                    jsonObj["refresh_token"].toString()
                    );
            }
            emit registerSuccess();
        } else {
            emit registerFailed(extractErrorMessage(reply, responseData, "Ошибка регистрации"));
        }
        reply->deleteLater();
    });
}

void ApiManager::logout()
{
    QString tokenToRevoke = refreshToken;
    clearTokens();

    if (!tokenToRevoke.isEmpty()) {
        QUrl url(baseUrl + "/auth/logout");
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QJsonObject json;
        json["refresh_token"] = tokenToRevoke;

        QNetworkReply *reply = networkManager->post(request, QJsonDocument(json).toJson());
        connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    }
}

bool ApiManager::hasSavedSession() const
{
    return !refreshToken.isEmpty();
}

void ApiManager::createHobby(const QString &name)
{
    QUrl url(baseUrl + "/hobby");
    QNetworkRequest request = createAuthorizedRequest(url);

    QJsonObject json;
    json["name"] = name;

    QNetworkReply *reply = networkManager->post(request, QJsonDocument(json).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray responseData = reply->readAll();

        if (statusCode == 401) {
            emit sessionExpired();
        } else if (reply->error() == QNetworkReply::NoError) {
            QJsonObject obj = QJsonDocument::fromJson(responseData).object();
            emit hobbyCreated(obj["hobby_id"].toString(), obj["name"].toString());
        } else {
            emit apiError(extractErrorMessage(reply, responseData, "Не удалось создать хобби"));
        }
        reply->deleteLater();
    });
}

void ApiManager::renameHobby(const QString &hobbyId, const QString &newName)
{
    QUrl url(QString("%1/hobby/%2/name").arg(baseUrl, hobbyId));
    QNetworkRequest request = createAuthorizedRequest(url);

    QJsonObject json;
    json["name"] = newName;

    QNetworkReply *reply = networkManager->sendCustomRequest(request, "PATCH", QJsonDocument(json).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray responseData = reply->readAll();

        if (statusCode == 401) {
            emit sessionExpired();
        } else if (reply->error() == QNetworkReply::NoError) {
            QJsonObject obj = QJsonDocument::fromJson(responseData).object();
            emit hobbyRenamed(obj["hobby_id"].toString(), obj["name"].toString());
        } else {
            emit apiError(extractErrorMessage(reply, responseData, "Не удалось переименовать хобби"));
        }
        reply->deleteLater();
    });
}

void ApiManager::deleteHobby(const QString &hobbyId)
{
    QUrl url(QString("%1/hobby/%2").arg(baseUrl, hobbyId));
    QNetworkRequest request = createAuthorizedRequest(url);

    QNetworkReply *reply = networkManager->deleteResource(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, hobbyId]() {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray responseData = reply->readAll();

        if (statusCode == 401) {
            emit sessionExpired();
        } else if (reply->error() == QNetworkReply::NoError) {
            emit hobbyDeleted(hobbyId);
        } else {
            emit apiError(extractErrorMessage(reply, responseData, "Не удалось удалить хобби"));
        }
        reply->deleteLater();
    });
}

void ApiManager::fetchHobbiesStats(const QDate &dateFrom, const QDate &dateTo)
{
    QUrl url(baseUrl + "/hobby/stats");
    QUrlQuery query;
    query.addQueryItem("date_from", dateFrom.toString("yyyy-MM-dd"));
    query.addQueryItem("date_to", dateTo.toString("yyyy-MM-dd"));
    url.setQuery(query);

    QNetworkRequest request = createAuthorizedRequest(url);
    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray responseData = reply->readAll();

        if (statusCode == 401) {
            emit sessionExpired();
        } else if (reply->error() == QNetworkReply::NoError) {
            emit hobbiesStatsLoaded(QJsonDocument::fromJson(responseData).object());
        } else {
            emit apiError(extractErrorMessage(reply, responseData, "Не удалось загрузить статистику"));
        }
        reply->deleteLater();
    });
}

void ApiManager::fetchHobbyStats(const QString &hobbyId, const QDate &dateFrom, const QDate &dateTo)
{
    QUrl url(QString("%1/hobby/%2/stats").arg(baseUrl, hobbyId));
    QUrlQuery query;
    query.addQueryItem("date_from", dateFrom.toString("yyyy-MM-dd"));
    query.addQueryItem("date_to", dateTo.toString("yyyy-MM-dd"));
    url.setQuery(query);

    QNetworkRequest request = createAuthorizedRequest(url);
    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray responseData = reply->readAll();

        if (statusCode == 401) {
            emit sessionExpired();
        } else if (reply->error() == QNetworkReply::NoError) {
            emit hobbyStatsLoaded(QJsonDocument::fromJson(responseData).object());
        } else {
            emit apiError(extractErrorMessage(reply, responseData, "Не удалось загрузить статистику хобби"));
        }
        reply->deleteLater();
    });
}

void ApiManager::addActivity(const QString &hobbyId, int durationMinutes, const QDateTime &startedAt, const QString &note)
{
    QUrl url(baseUrl + "/activity");
    QNetworkRequest request = createAuthorizedRequest(url);

    QJsonObject json;
    json["hobby_id"] = hobbyId;
    json["duration_minutes"] = durationMinutes;
    json["started_at"] = startedAt.toUTC().toString(Qt::ISODate);
    if (note.trimmed().isEmpty()) {
        json["note"] = QJsonValue::Null;
    } else {
        json["note"] = note.trimmed();
    }

    QNetworkReply *reply = networkManager->post(request, QJsonDocument(json).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray responseData = reply->readAll();

        if (statusCode == 401) {
            emit sessionExpired();
        } else if (reply->error() == QNetworkReply::NoError) {
            emit activityAdded();
        } else {
            emit apiError(extractErrorMessage(reply, responseData, "Не удалось сохранить активность"));
        }
        reply->deleteLater();
    });
}