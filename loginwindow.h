#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include "apimanager.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include <QJsonDocument>
#include <QJsonObject>

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class MainWindow;

class LoginWindow : public QWidget {
  Q_OBJECT;

public:
  explicit LoginWindow(QWidget *parent = nullptr);

signals:
  void loginSuccessful();
  void *registerRequested();

private:
  QLineEdit *loginInput;
  QLineEdit *passwordInput;
  QLabel *errorLabel;
  ApiManager *apiManager;
};

#endif