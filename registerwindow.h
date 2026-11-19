#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include "apimanager.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class RegisterWindow : public QWidget {
  Q_OBJECT

public:
  explicit RegisterWindow(QWidget *parent = nullptr);

signals:
  void registerSuccessful();
  void loginRequested();

private:
  QLineEdit *loginInput;
  QLineEdit *passwordInput;
  QLineEdit *passwordConfirmInput;
  QLabel *errorLabel;
  ApiManager *apiManager;
};

#endif
