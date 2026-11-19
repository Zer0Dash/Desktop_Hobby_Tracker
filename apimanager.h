#ifndef APIMANAGER_H
#define APIMANAGER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

class ApiManager : public QObject {
  Q_OBJECT;

public:
  explicit ApiManager(QObject *parent = nullptr);
  void login(const QString &username, const QString &password);
  void registerUser(const QString &username, const QString &password);

signals:
  void loginSuccess();
  void loginFailed(const QString &errorMessage);

  void registerSuccess();
  void registerFailed(const QString &errorMessage);

private:
  QNetworkAccessManager *networkManager;
  QString baseUrl;
  QString accessToken;
  bool useMock = true;
};

#endif