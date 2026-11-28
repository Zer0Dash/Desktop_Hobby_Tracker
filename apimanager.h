#ifndef APIMANAGER_H
#define APIMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDate>
#include <QDateTime>
#include <QString>

class ApiManager : public QObject
{
    Q_OBJECT
public:
    explicit ApiManager(QObject *parent = nullptr);

    void login(const QString &username, const QString &password);
    void registerUser(const QString &username, const QString &password);
    void logout();
    bool hasSavedSession() const;
    void refreshSession();

    void createHobby(const QString &name);
    void renameHobby(const QString &hobbyId, const QString &newName);
    void deleteHobby(const QString &hobbyId);
    void fetchHobbiesStats(const QDate &dateFrom, const QDate &dateTo);
    void fetchHobbyStats(const QString &hobbyId, const QDate &dateFrom, const QDate &dateTo);
    void addActivity(const QString &hobbyId, int durationMinutes, const QDateTime &startedAt, const QString &note = QString());

signals:
    void loginSuccess();
    void loginFailed(const QString &errorMessage);
    void sessionRestored();
    void sessionExpired();

    void registerSuccess();
    void registerFailed(const QString &errorMessage);

    void hobbyCreated(const QString &hobbyId, const QString &name);
    void hobbyRenamed(const QString &hobbyId, const QString &newName);
    void hobbyDeleted(const QString &hobbyId);
    void hobbiesStatsLoaded(const QJsonObject &response);
    void hobbyStatsLoaded(const QJsonObject &response);
    void activityAdded();
    void apiError(const QString &errorMessage);

private:
    QNetworkAccessManager *networkManager;
    QString baseUrl;
    QString accessToken;
    QString refreshToken;
    bool useMock = false;

    void saveTokens(const QString &access, const QString &refresh);
    void clearTokens();
    QNetworkRequest createAuthorizedRequest(const QUrl &url) const;
    QString extractErrorMessage(QNetworkReply *reply, const QByteArray &data, const QString &fallback) const;
};

#endif