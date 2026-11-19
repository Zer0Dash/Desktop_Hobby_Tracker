#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
  setupUI();
  applyStyles();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
  QHBoxLayout *mainLayout = new QHBoxLayout(this);

  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  QWidget *sideBar = new QWidget();
  sideBar->setObjectName("sideBar");
  sideBar->setFixedWidth(250);

  QVBoxLayout *menuLayout = new QVBoxLayout(sideBar);
  menuLayout->setContentsMargins(10, 20, 10, 20);
  menuLayout->setSpacing(10);

  QLabel *logoLabel = new QLabel("Hobby Tracker");
  logoLabel->setObjectName("logoLabel");
  logoLabel->setAlignment(Qt::AlignCenter);

  btnMain = new QPushButton("Главная");
  btnHobby = new QPushButton("Хобби (Общее)");
  btnProgramming = new QPushButton("Программирование");

  btnMain->setCheckable(true);
  btnHobby->setCheckable(true);
  btnProgramming->setCheckable(true);

  btnMain->setChecked(true);

  menuLayout->addWidget(logoLabel);
  menuLayout->addSpacing(20);
  menuLayout->addWidget(btnMain);
  menuLayout->addWidget(btnHobby);
  menuLayout->addWidget(btnProgramming);
  menuLayout->addStretch();

  totalSessionSeconds = 0;

  totalTimeLabel = new QLabel("Всего за сессию:\n00:00:00");
  totalTimeLabel->setAlignment(Qt::AlignCenter);
  totalTimeLabel->setObjectName("totalTimeLabel");

  menuLayout->addWidget(totalTimeLabel);

  stackedWidget = new QStackedWidget();

  QWidget *mainPage = createMainTab();
  QChartView *hobbyPage = createPieChart("Распределение времени по всем хобби");
  QWidget *progPage = createProgrammingTab();

  stackedWidget->addWidget(mainPage);
  stackedWidget->addWidget(hobbyPage);
  stackedWidget->addWidget(progPage);

  mainLayout->addWidget(sideBar);
  mainLayout->addWidget(stackedWidget);

  connect(btnMain, &QPushButton::clicked, [this]() { switchTab(0); });

  connect(btnHobby, &QPushButton::clicked, [this]() { switchTab(1); });

  connect(btnProgramming, &QPushButton::clicked, [this]() { switchTab(2); });

  setWindowTitle("Hobby Tracker");
  resize(1024, 768);
}

QChartView *MainWindow::createChart(const QString &title,
                                    const QList<QPointF> &points) {
  QLineSeries *series = new QLineSeries();

  for (const QPointF &p : points) {
    series->append(p);
  }

  QChart *chart = new QChart();
  chart->addSeries(series);
  chart->createDefaultAxes();
  chart->setTitle(title);
  chart->setTheme(QChart::ChartThemeLight);
  chart->legend()->hide();
  chart->setAnimationOptions(QChart::SeriesAnimations);

  QChartView *chartView = new QChartView(chart);
  chartView->setRenderHint(QPainter::Antialiasing);

  return chartView;
}

void MainWindow::switchTab(int index) {
  stackedWidget->setCurrentIndex(index);

  btnMain->setChecked(index == 0);
  btnHobby->setChecked(index == 1);
  btnProgramming->setChecked(index == 2);
}

QWidget *MainWindow::createProgrammingTab() {
  QWidget *tabWidget = new QWidget();

  QVBoxLayout *tabLayout = new QVBoxLayout(tabWidget);
  tabLayout->setContentsMargins(20, 20, 20, 20);
  tabLayout->setSpacing(15);

  int totalHours = 142;

  QLabel *totalLabel =
      new QLabel(QString("Общее количество часов: %1").arg(totalHours));

  QFont titleFont = totalLabel->font();
  titleFont.setPointSize(16);
  titleFont.setBold(true);
  totalLabel->setFont(titleFont);

  tabLayout->addWidget(totalLabel);

  QTableWidget *table = new QTableWidget(1, 7);

  QStringList days = {"Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс"};
  table->setHorizontalHeaderLabels(days);

  table->verticalHeader()->setVisible(false);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);

  QList<int> weeklyHours = {4, 2, 5, 0, 6, 8, 3};

  for (int col = 0; col < 7; ++col) {
    QTableWidgetItem *item =
        new QTableWidgetItem(QString::number(weeklyHours[col]) + " ч");
    item->setTextAlignment(Qt::AlignCenter);
    table->setItem(0, col, item);
  }

  table->setFixedHeight(65);
  tabLayout->addWidget(table);

  QChartView *chartView =
      createBarChart("Динамика за неделю", days, weeklyHours);
  tabLayout->addWidget(chartView, 1);

  return tabWidget;
}

QChartView *MainWindow::createPieChart(const QString &title) {
  QPieSeries *series = new QPieSeries();

  series->append("Программирование", 142);
  series->append("Чтение", 24);
  series->append("Спорт", 58);
  series->append("Игры", 10);

  QPieSlice *progSlice = series->slices().at(0);
  progSlice->setLabelVisible(true);
  progSlice->setBrush(QColor("#3498db"));

  QPieSlice *sportSlice = series->slices().at(1);
  sportSlice->setLabelVisible(true);
  sportSlice->setBrush(QColor("#e74c3c"));

  QPieSlice *readSlice = series->slices().at(2);
  readSlice->setLabelVisible(true);
  readSlice->setBrush(QColor("#2ecc71"));

  QChart *chart = new QChart();
  chart->addSeries(series);
  chart->setTitle(title);

  chart->legend()->setVisible(true);
  chart->legend()->setAlignment(Qt::AlignRight);
  chart->setAnimationOptions(QChart::SeriesAnimations);

  QChartView *chartView = new QChartView(chart);
  chartView->setRenderHint(QPainter::Antialiasing);

  return chartView;
}

QChartView *MainWindow::createBarChart(const QString &title,
                                       const QStringList &categories,
                                       const QList<int> &values) {
  QBarSet *set = new QBarSet("Часы");

  for (int val : values) {
    *set << val;
  }

  set->setBrush(QColor("#3498db"));

  QBarSeries *series = new QBarSeries();
  series->append(set);

  QChart *chart = new QChart();
  chart->addSeries(series);
  chart->setTitle(title);
  chart->setAnimationOptions(QChart::SeriesAnimations);

  QBarCategoryAxis *axisX = new QBarCategoryAxis();
  axisX->append(categories);
  chart->addAxis(axisX, Qt::AlignBottom);
  series->attachAxis(axisX);

  QValueAxis *axisY = new QValueAxis();
  axisY->setLabelFormat("%d");
  axisY->setTitleText("Часы");

  int maxVal = 0;
  for (int val : values) {
    if (val > maxVal)
      maxVal = val;
  }
  axisY->setRange(0, maxVal + 2);

  chart->addAxis(axisY, Qt::AlignLeft);
  series->attachAxis(axisY);

  chart->legend()->setVisible(false);

  QChartView *chartView = new QChartView(chart);
  chartView->setRenderHint(QPainter::Antialiasing);

  return chartView;
}

QWidget *MainWindow::createMainTab() {
  QWidget *tabWidget = new QWidget();
  QVBoxLayout *tabLayout = new QVBoxLayout(tabWidget);
  tabLayout->setAlignment(Qt::AlignCenter);
  tabLayout->setSpacing(20);

  stopwatchLabel = new QLabel("00:00:00");
  stopwatchLabel->setAlignment(Qt::AlignCenter);

  QFont timeFont = stopwatchLabel->font();
  timeFont.setPointSize(48);
  timeFont.setBold(true);
  stopwatchLabel->setFont(timeFont);
  stopwatchLabel->setStyleSheet("color: #2c3e50;");

  QHBoxLayout *buttonsLayout = new QHBoxLayout();

  QPushButton *btnStart = new QPushButton("Старт");
  QPushButton *btnStop = new QPushButton("Стоп ");
  QPushButton *btnPause = new QPushButton("Пауза");

  QString btnStyle =
      "QPushButton { background-color: #3498db; color: white; border-radius: "
      "5px; padding: 10px; font-size: 16px; min-width: 120px; } "
      "QPushButton:hover { background-color: #2980b9; }";

  btnStart->setStyleSheet(btnStyle);
  btnStop->setStyleSheet(btnStyle);
  btnPause->setStyleSheet(btnStyle);

  btnPause->hide();
  btnStop->hide();
  buttonsLayout->addWidget(btnStart);
  buttonsLayout->addWidget(btnPause);
  buttonsLayout->addWidget(btnStop);

  sessionSeconds = 0;
  sessionTimer = new QTimer(this);

  connect(sessionTimer, &QTimer::timeout, this, [this]() {
    sessionSeconds++;
    QTime time(0, 0, 0);
    time = time.addSecs(sessionSeconds);
    stopwatchLabel->setText(time.toString("hh:mm:ss"));
  });

  connect(btnStart, &QPushButton::clicked, this,
          [this, btnStart, btnPause, btnStop]() {
            if (!sessionTimer->isActive()) {
              sessionTimer->start(1000);
              btnStart->hide();
              btnStop->show();
              btnPause->show();
            }
          });

  connect(btnStop, &QPushButton::clicked, this,
          [this, btnStart, btnPause, btnStop]() {
            sessionTimer->stop();
            totalSessionSeconds += sessionSeconds;

            QTime totalTime(0, 0, 0);
            totalTime = totalTime.addSecs(totalSessionSeconds);
            totalTimeLabel->setText("Всего за сессию:\n" +
                                    totalTime.toString("hh:mm:ss"));

            sessionSeconds = 0;
            stopwatchLabel->setText("00:00:00");
            btnStart->show();
            btnPause->hide();
            btnStop->hide();
            btnStart->setText("Старт");
          });

  connect(btnPause, &QPushButton::clicked, this,
          [this, btnStart, btnPause, btnStop]() {
            if (sessionTimer->isActive()) {
              sessionTimer->stop();
              btnPause->hide();
              btnStart->show();
              btnStart->setText("Продолжить");
            }
          });

  tabLayout->addWidget(stopwatchLabel);
  tabLayout->addLayout(buttonsLayout);

  return tabWidget;
}

void MainWindow::applyStyles() {
  QString styleSheet = R"(
        #sideBar {
            background-color: #2c3e50;
        }

        #logoLabel {
            color: #ecf0f1;
            font-size: 20px;
            font-weight: bold;
        }

        QPushButton {
            background-color: transparent;
            color: #bdc3c7;
            border: none;
            padding: 12px;
            text-align: left;
            font-size: 14px;
            border-radius: 5px;
        }

        QPushButton:hover {
            background-color: #34495e;
            color: #ffffff;
        }

        QPushButton:checked {
            background-color: #3498db;
            color: #ffffff;
            font-weight: bold;
        }

        #mainContentText {
            font-size: 24px;
            color: #7f8c8d;
        }

        #totalTimeLabel{
            color: #ffffff;
            font-size: 18px;
            font-weight: bold;
            margin-top: 10px;
        }
    )";

  this->setStyleSheet(styleSheet);
}