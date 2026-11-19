#include "apimanager.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrl>

ApiManager::ApiManager(QObject *parent) : QObject(parent) {
  networkManager = new QNetworkAccessManager(this);
  // заменить на url своего бэкенда
  baseUrl = "http://localhost:8000";
}

void ApiManager::login(const QString &username, const QString &password) {
  if (useMock) {
    if (username == "admin" && password == "1234") {
      accessToken = "mock_jwt_token_12345";
      emit loginSuccess();
    } else {
      emit loginFailed("Неверный логин или пароль (Mock)");
    }
    return;
  }

  QUrl url(baseUrl + "/auth/login");
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject json;
  json["username"] = username;
  json["password"] = password;

  QJsonDocument doc(json);
  QByteArray data = doc.toJson();

  QNetworkReply *reply = networkManager->post(request, data);

  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    if (reply->error() == QNetworkReply::NoError) {
      QByteArray responseData = reply->readAll();
      QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
      QJsonObject jsonObj = jsonResponse.object();

      if (jsonObj.contains("access_token")) {
        accessToken = jsonObj["access_token"].toString();
        emit loginSuccess();
      } else {
        emit loginFailed("Ошибка: сервер не прислал токен");
      }
    } else {
      emit loginFailed("Неверный Логин или Пароль");
    }
    reply->deleteLater();
  });
}

void ApiManager::registerUser(const QString &username,
                              const QString &password) {
  QUrl url(baseUrl + "/auth/register");
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject json;
  json["username"] = username;
  json["password"] = password;
  QByteArray data = QJsonDocument(json).toJson();
  QNetworkReply *reply = networkManager->post(request, data);

  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    if (reply->error() == QNetworkReply::NoError) {
      emit registerSuccess();
    } else {
      emit registerFailed("Ошибка регистрации (возможно, логин занят)");
    }
    reply->deleteLater();
  });
}