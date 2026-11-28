#include <QApplication>
#include "mainwindow.h"
#include "loginwindow.h"
#include "registerwindow.h"
#include "apimanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ApiManager apiManager;
    MainWindow mainWin(&apiManager);
    LoginWindow logWin(&apiManager);
    RegisterWindow regWin(&apiManager);

    QObject::connect(&mainWin, &MainWindow::logoutRequested, [&]() {
        apiManager.logout();
        mainWin.hide();
        logWin.resetFields();
        logWin.show();
    });

    QObject::connect(&logWin, &LoginWindow::loginSuccessful, [&]() {
        logWin.hide();
        mainWin.loadInitialData();
        mainWin.show();
    });

    QObject::connect(&logWin, &LoginWindow::registerRequested, [&]() {
        logWin.hide();
        regWin.show();
    });

    QObject::connect(&regWin, &RegisterWindow::loginRequested, [&]() {
        regWin.hide();
        logWin.show();
    });

    QObject::connect(&regWin, &RegisterWindow::registerSuccessful, [&]() {
        regWin.hide();
        if (apiManager.hasSavedSession()) {
            mainWin.loadInitialData();
            mainWin.show();
        } else {
            logWin.show();
        }
    });

    QObject::connect(&apiManager, &ApiManager::sessionRestored, [&]() {
        mainWin.loadInitialData();
        mainWin.show();
    });

    QObject::connect(&apiManager, &ApiManager::sessionExpired, [&]() {
        mainWin.hide();
        logWin.resetFields();
        logWin.show();
    });

    if (apiManager.hasSavedSession()) {
        apiManager.refreshSession();
    } else {
        logWin.show();
    }

    return a.exec();
}