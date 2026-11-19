#include "loginwindow.h"
#include <QApplication>

LoginWindow::LoginWindow(QWidget *parent) : QWidget(parent) {
  this->setWindowTitle("Вход в систему");
  this->setFixedSize(400, 500);
  this->setStyleSheet("background-color: #f5f6fa;");

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setAlignment(Qt::AlignCenter);
  mainLayout->setSpacing(20);
  mainLayout->setContentsMargins(40, 40, 40, 40);

  QLabel *titleLabel = new QLabel("Авторизация", this);
  titleLabel->setAlignment(Qt::AlignCenter);
  QFont titleFont = titleLabel->font();
  titleFont.setPointSize(24);
  titleFont.setBold(true);
  titleLabel->setFont(titleFont);
  titleLabel->setStyleSheet("color: #2c3e50;");

  QString inputStyle =
      "QLineEdit { padding: 10px; border: 2px solid #bdc3c7; border-radius: "
      "5px; font-size: 16px; background-color: white; color: black; }"
      "QLineEdit:focus { border: 2px solid #3498db; }";

  loginInput = new QLineEdit(this);
  loginInput->setPlaceholderText("Логин");
  loginInput->setStyleSheet(inputStyle);

  passwordInput = new QLineEdit(this);
  passwordInput->setPlaceholderText("Пароль");
  passwordInput->setStyleSheet(inputStyle);

  passwordInput->setEchoMode(QLineEdit::Password);

  errorLabel = new QLabel("", this);
  errorLabel->setAlignment(Qt::AlignCenter);
  errorLabel->setStyleSheet(
      "color: #e74c3c; font-weight: bold; font-size: 14px;");

  QPushButton *btnLogin = new QPushButton("Войти", this);
  btnLogin->setCursor(Qt::PointingHandCursor);
  btnLogin->setStyleSheet(
      "QPushButton { background-color: #2c3e50; color: white; border-radius: "
      "5px; padding: 12px; font-size: 16px; font-weight: bold; }"
      "QPushButton:hover { background-color: #34495e; }");

  QPushButton *btnRegister =
      new QPushButton("Нет аккаунта? Зарегистрироваться", this);
  btnRegister->setCursor(Qt::PointingHandCursor);
  btnRegister->setStyleSheet(
      "QPushButton { color: #3498db; border: none; background: transparent; "
      "font-size: 14px; text-decoration: underline; }"
      "QPushButton:hover { color: #2980b9; }");

  mainLayout->addStretch();
  mainLayout->addWidget(titleLabel);
  mainLayout->addSpacing(30);
  mainLayout->addWidget(loginInput);
  mainLayout->addWidget(passwordInput);
  mainLayout->addWidget(errorLabel);
  mainLayout->addWidget(btnLogin);
  mainLayout->addWidget(btnRegister);
  mainLayout->addStretch();

  connect(btnLogin, &QPushButton::clicked, this, [this]() {
    QString username = loginInput->text();
    QString password = passwordInput->text();

    errorLabel->setText("");

    if (username == "admin" && password == "1234") {
      emit loginSuccessful();
    } else {
      errorLabel->setText("Неверный Логин или пароль");
    }
  });

  connect(btnRegister, &QPushButton::clicked, this,
          [this]() { emit registerRequested(); });
}
