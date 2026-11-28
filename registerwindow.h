#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include "apimanager.h"

class RegisterWindow : public QWidget {
    Q_OBJECT

public:
    explicit RegisterWindow(ApiManager *api,QWidget *parent = nullptr);

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

#endif // REGISTERWINDOW_H
