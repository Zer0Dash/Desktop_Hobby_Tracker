#include "loginwindow.h"
#include "mainwindow.h"
#include "registerwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  MainWindow mainWin;
  LoginWindow logWin;
  RegisterWindow regWin;

  QObject::connect(&logWin, &LoginWindow::loginSuccessful, [&]() {
    logWin.hide();
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
    logWin.show();
  });

  logWin.show();

  return a.exec();
}