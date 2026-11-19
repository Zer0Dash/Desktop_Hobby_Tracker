#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QHeaderView>
#include <QList>
#include <QPointF>
#include <QPushButton>
#include <QStackedWidget>
#include <QString>
#include <QTableWidget>
#include <QTime>
#include <QTimer>
#include <QWidget>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QValueAxis>

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>

class MainWindow : public QWidget {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  virtual ~MainWindow();

private:
  QStackedWidget *stackedWidget;

  QPushButton *btnMain;
  QPushButton *btnHobby;
  QPushButton *btnProgramming;

  void setupUI();
  void applyStyles();

  QChartView *createChart(const QString &title, const QList<QPointF> &points);
  void switchTab(int index);

  QWidget *createProgrammingTab();
  QChartView *createPieChart(const QString &title);
  QChartView *createBarChart(const QString &title,
                             const QStringList &categories,
                             const QList<int> &values);

  QTimer *sessionTimer;
  int sessionSeconds;
  QLabel *stopwatchLabel;

  QWidget *createMainTab();

  QLabel *totalTimeLabel;
  int totalSessionSeconds;
};

#endif