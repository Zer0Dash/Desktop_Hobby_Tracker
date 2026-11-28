#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include "apimanager.h"

//заголовочные файлы для работы с сетью
#include <QNetworkAccessManager> //главный сетевой движок, управляет отправкой запросов и работает асинхронно
#include <QNetworkRequest> //класс для настройки самого запроса. С его помощью зададим URL эндпоинта и хттп заголовки
#include <QNetworkReply> //класс, хранящий ответ от сервера, из него будет считывать статус код
#include <QUrl> //безопасный парсер ссылок
// заголовочные файлы для работы с JSON

#include <QJsonObject> //для создания структуры "ключ-значение", куда мы положим логин и пароль
#include <QJsonDocument> //для конвертации объекта QJsonObject в чистый текстовый формат, который можно отправить по сети, и для обратного парсинга ответа сервера

#include <QWidget>
#include <QLineEdit>    // Поля ввода (логин, пароль)
#include <QPushButton>  // Кнопки
#include <QLabel>       // Текстовые метки
#include <QVBoxLayout>  // Вертикальный слой разметки

class MainWindow;

class LoginWindow: public QWidget {
    Q_OBJECT;
public:
    explicit LoginWindow(ApiManager *api, QWidget *parent = nullptr);
    void resetFields();

signals:
    void loginSuccessful(); // Наш кастомный сигнал для переключения на главное окно
    void *registerRequested(); //сигнал о том, что юзер хочет зарегестрироваться

private:
    QLineEdit *loginInput;
    QLineEdit *passwordInput;
    QLabel *errorLabel;
    ApiManager *apiManager;
};

#endif // LOGINWINDOW_H
