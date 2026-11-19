#include "registerwindow.h"

RegisterWindow::RegisterWindow(QWidget *parent) : QWidget(parent) {

  this->setWindowTitle("Регистрация");
  this->setFixedSize(400, 550);
  this->setStyleSheet("background-color: #f5f6fa;");

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setAlignment(Qt::AlignCenter);
  mainLayout->setSpacing(20);
  mainLayout->setContentsMargins(40, 40, 40, 40);

  QLabel *titleLabel = new QLabel("Регистрация", this);
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
  loginInput->setPlaceholderText("Придумайте логин");
  loginInput->setStyleSheet(inputStyle);

  passwordInput = new QLineEdit(this);
  passwordInput->setPlaceholderText("Придумайте пароль");
  passwordInput->setStyleSheet(inputStyle);
  passwordInput->setEchoMode(QLineEdit::Password);

  passwordConfirmInput = new QLineEdit(this);
  passwordConfirmInput->setPlaceholderText("Повторите пароль");
  passwordConfirmInput->setStyleSheet(inputStyle);
  passwordConfirmInput->setEchoMode(QLineEdit::Password);

  errorLabel = new QLabel("", this);
  errorLabel->setAlignment(Qt::AlignCenter);
  errorLabel->setStyleSheet(
      "color: #e74c3c; font-weight: bold; font-size: 14px;");

  QPushButton *btnRegister = new QPushButton("Зарегистрироваться", this);
  btnRegister->setCursor(Qt::PointingHandCursor);
  btnRegister->setStyleSheet(
      "QPushButton { background-color: #27ae60; color: white; border-radius: "
      "5px; padding: 12px; font-size: 16px; font-weight: bold; }"
      "QPushButton:hover { background-color: #2ecc71; }");

  QPushButton *btnLogin = new QPushButton("Уже есть аккаунт? Войти", this);
  btnLogin->setCursor(Qt::PointingHandCursor);
  btnLogin->setStyleSheet(
      "QPushButton { color: #3498db; border: none; background: transparent; "
      "font-size: 14px; text-decoration: underline; }"
      "QPushButton:hover { color: #2980b9; }");

  mainLayout->addStretch();
  mainLayout->addWidget(titleLabel);
  mainLayout->addSpacing(20);
  mainLayout->addWidget(loginInput);
  mainLayout->addWidget(passwordInput);
  mainLayout->addWidget(passwordConfirmInput);
  mainLayout->addWidget(errorLabel);
  mainLayout->addWidget(btnRegister);
  mainLayout->addWidget(btnLogin);
  mainLayout->addStretch();

  apiManager = new ApiManager(this);

  connect(apiManager, &ApiManager::registerSuccess, this,
          [this]() { emit registerSuccessful(); });

  connect(apiManager, &ApiManager::registerFailed, this,
          [this, btnRegister](const QString &errorMsg) {
            errorLabel->setText(errorMsg);
            btnRegister->setEnabled(true);
          });

  connect(btnRegister, &QPushButton::clicked, this, [this, btnRegister]() {
    QString username = loginInput->text();
    QString password = passwordInput->text();
    QString confirm = passwordConfirmInput->text();
    if (username.isEmpty() || password.isEmpty() || confirm.isEmpty()) {
      errorLabel->setText("Заполните все поля");
      return;
    }
    if (password != confirm) {
      errorLabel->setText("Пароли не совпадают");
      return;
    }

    errorLabel->setText("");
    btnRegister->setEnabled(false);
    apiManager->registerUser(username, password);
  });

  connect(btnLogin, &QPushButton::clicked, this,
          [this]() { emit loginRequested(); });
}
