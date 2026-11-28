#include "loginwindow.h"
#include <QApplication>

LoginWindow::LoginWindow(ApiManager *api,QWidget *parent) : QWidget(parent), apiManager(api){
    this->setWindowTitle("Вход в систему");
    this->setFixedSize(400,500);
    this->setStyleSheet("background-color: #f5f6fa;");


    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(40, 40, 40, 40); //отстутпы от краев окна

    //1 заголовок
    QLabel *titleLabel = new QLabel("Авторизация", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #2c3e50;");

    //2 поля ввода (логин и пароль)
    QString inputStyle = "QLineEdit { padding: 10px; border: 2px solid #bdc3c7; border-radius: 5px; font-size: 16px; background-color: white; color: black; }"
                         "QLineEdit:focus { border: 2px solid #3498db; }";

    loginInput = new QLineEdit(this);
    loginInput->setPlaceholderText("Логин");
    loginInput->setStyleSheet(inputStyle);

    passwordInput = new QLineEdit(this);
    passwordInput->setPlaceholderText("Пароль");
    passwordInput->setStyleSheet(inputStyle);

    //скрываем вводимые символы пароля точками
    passwordInput->setEchoMode(QLineEdit::Password);

    //метка для вывода ошибок
    errorLabel = new QLabel("", this);
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->setStyleSheet("color: #e74c3c; font-weight: bold; font-size: 14px;");

    //4 кнопка войти
    QPushButton *btnLogin = new QPushButton("Войти", this);
    btnLogin->setCursor(Qt::PointingHandCursor);
    btnLogin->setStyleSheet("QPushButton { background-color: #2c3e50; color: white; border-radius: 5px; padding: 12px; font-size: 16px; font-weight: bold; }"
                            "QPushButton:hover { background-color: #34495e; }");

    //5 кнопка регистрации
    QPushButton *btnRegister = new QPushButton("Нет аккаунта? Зарегистрироваться", this);
    btnRegister->setCursor(Qt::PointingHandCursor);
    btnRegister->setStyleSheet("QPushButton { color: #3498db; border: none; background: transparent; font-size: 14px; text-decoration: underline; }"
                               "QPushButton:hover { color: #2980b9; }");

    mainLayout->addStretch(); // Пружина сверху
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(loginInput);
    mainLayout->addWidget(passwordInput);
    mainLayout->addWidget(errorLabel); // Ошибка выводится прямо под паролем
    mainLayout->addWidget(btnLogin);
    mainLayout->addWidget(btnRegister);
    mainLayout->addStretch(); // Пружина снизу



    connect(btnLogin, &QPushButton::clicked, this, [this, btnLogin]() {
        QString username = loginInput->text().trimmed();
        QString password = passwordInput->text();

        errorLabel->setText("");

        if (username.isEmpty() || password.isEmpty()) {
            errorLabel->setText("Заполните логин и пароль");
            return;
        }

        btnLogin->setEnabled(false);
        btnLogin->setText("Вход...");

        apiManager->login(username, password);
    });

    connect(btnRegister, &QPushButton::clicked, this, [this]() {
        qDebug() << "[LOGIN] Нажата кнопка перехода к регистрации";
        emit registerRequested();
    });

    connect(apiManager, &ApiManager::loginSuccess, this, [this, btnLogin]() {
        btnLogin->setEnabled(true);
        btnLogin->setText("Войти");
        emit loginSuccessful();
    });

    connect(apiManager, &ApiManager::loginFailed, this, [this, btnLogin](const QString &errorMsg) {
        btnLogin->setEnabled(true);
        btnLogin->setText("Войти");
        errorLabel->setText(errorMsg);
    });
}

void LoginWindow::resetFields(){
    loginInput->clear();
    passwordInput->clear();
    errorLabel->clear();

}
