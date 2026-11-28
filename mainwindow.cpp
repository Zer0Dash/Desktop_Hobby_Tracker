#include "mainwindow.h"

#include <QDialog>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>

MainWindow::MainWindow(ApiManager *api, QWidget *parent)
    : QWidget(parent), apiManager(api)
{
    setupUI();
    setupApiConnections();
    applyStyles();
}

MainWindow::~MainWindow() {}

void MainWindow::loadInitialData()
{
    requestStatsRefresh();
}

void MainWindow::requestStatsRefresh()
{
    if (dateFromEdit && dateToEdit) {
        apiManager->fetchHobbiesStats(dateFromEdit->date(), dateToEdit->date().addDays(1));
    } else {
        apiManager->fetchHobbiesStats(QDate::currentDate().addDays(-30), QDate::currentDate());
    }
}

void MainWindow::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QWidget *sideBar = new QWidget(this);
    sideBar->setObjectName("sideBar");
    sideBar->setFixedWidth(260);

    QVBoxLayout *menuLayout = new QVBoxLayout(sideBar);
    menuLayout->setContentsMargins(16, 24, 16, 20);
    menuLayout->setSpacing(8);

    QLabel *logoLabel = new QLabel("Hobby Tracker", sideBar);
    logoLabel->setObjectName("logoLabel");
    logoLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    menuLayout->addWidget(logoLabel);
    menuLayout->addSpacing(16);

    navButtonGroup = new QButtonGroup(this);
    navButtonGroup->setExclusive(true);

    btnMain = new QPushButton("Таймер сессии", sideBar);
    btnMain->setCheckable(true);
    btnMain->setChecked(true);
    btnMain->setCursor(Qt::PointingHandCursor);

    btnStats = new QPushButton("Общая статистика", sideBar);
    btnStats->setCheckable(true);
    btnStats->setCursor(Qt::PointingHandCursor);

    navButtonGroup->addButton(btnMain);
    navButtonGroup->addButton(btnStats);

    menuLayout->addWidget(btnMain);
    menuLayout->addWidget(btnStats);
    menuLayout->addSpacing(12);

    QFrame *divider = new QFrame(sideBar);
    divider->setObjectName("sideDivider");
    divider->setFrameShape(QFrame::HLine);
    divider->setFixedHeight(1);
    menuLayout->addWidget(divider);
    menuLayout->addSpacing(12);

    QHBoxLayout *hobbiesHeaderLayout = new QHBoxLayout();
    hobbiesHeaderLayout->setContentsMargins(6, 0, 2, 0);

    QLabel *hobbiesSectionTitle = new QLabel("МОИ ХОББИ", sideBar);
    hobbiesSectionTitle->setObjectName("sectionTitle");

    btnAddHobby = new QPushButton("+ Добавить", sideBar);
    btnAddHobby->setObjectName("btnAddHobby");
    btnAddHobby->setCursor(Qt::PointingHandCursor);

    hobbiesHeaderLayout->addWidget(hobbiesSectionTitle);
    hobbiesHeaderLayout->addStretch();
    hobbiesHeaderLayout->addWidget(btnAddHobby);
    menuLayout->addLayout(hobbiesHeaderLayout);

    QScrollArea *scrollArea = new QScrollArea(sideBar);
    scrollArea->setObjectName("hobbyScrollArea");
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *scrollContainer = new QWidget();
    scrollContainer->setObjectName("scrollContainer");
    hobbiesListLayout = new QVBoxLayout(scrollContainer);
    hobbiesListLayout->setContentsMargins(0, 4, 0, 4);
    hobbiesListLayout->setSpacing(6);
    hobbiesListLayout->addStretch();

    scrollArea->setWidget(scrollContainer);
    menuLayout->addWidget(scrollArea, 1);

    QWidget *sessionCard = new QWidget(sideBar);
    sessionCard->setObjectName("sessionCard");
    QVBoxLayout *cardLayout = new QVBoxLayout(sessionCard);
    cardLayout->setContentsMargins(12, 12, 12, 12);
    cardLayout->setSpacing(4);

    QLabel *sessionCaption = new QLabel("ВСЕГО ЗА СЕССИЮ", sessionCard);
    sessionCaption->setObjectName("sessionCaption");
    sessionCaption->setAlignment(Qt::AlignCenter);

    totalSessionSeconds = 0;
    totalTimeLabel = new QLabel("00:00:00", sessionCard);
    totalTimeLabel->setObjectName("totalTimeLabel");
    totalTimeLabel->setAlignment(Qt::AlignCenter);

    cardLayout->addWidget(sessionCaption);
    cardLayout->addWidget(totalTimeLabel);
    menuLayout->addWidget(sessionCard);

    btnLeave = new QPushButton("Выйти из аккаунта", sideBar);
    btnLeave->setObjectName("btnLeave");
    btnLeave->setCursor(Qt::PointingHandCursor);
    menuLayout->addWidget(btnLeave);

    stackedWidget = new QStackedWidget(this);
    stackedWidget->setObjectName("contentArea");

    QWidget *mainPage = createMainTab();
    QWidget *overviewPage = createOverviewTab();

    stackedWidget->addWidget(mainPage);
    stackedWidget->addWidget(overviewPage);

    mainLayout->addWidget(sideBar);
    mainLayout->addWidget(stackedWidget);

    connect(btnMain, &QPushButton::clicked, this, [this, mainPage]() {
        stackedWidget->setCurrentWidget(mainPage);
    });

    connect(btnStats, &QPushButton::clicked, this, [this, overviewPage]() {
        requestStatsRefresh();
        stackedWidget->setCurrentWidget(overviewPage);
    });

    connect(btnAddHobby, &QPushButton::clicked, this, &MainWindow::showAddHobbyDialog);

    connect(btnLeave, &QPushButton::clicked, this, [this]() {
        clearAllHobbies();
        emit logoutRequested();
    });

    setWindowTitle("Hobby Tracker");
    resize(1080, 740);
}

void MainWindow::setupApiConnections()
{
    connect(apiManager, &ApiManager::hobbyCreated, this, [this](const QString &hobbyId, const QString &name) {
        upsertHobby(hobbyId, name, 0.0, 0, 0.0);
        updateOverviewChart();
    });

    connect(apiManager, &ApiManager::hobbyRenamed, this, [this](const QString &hobbyId, const QString &newName) {
        for (int i = 0; i < hobbies.size(); ++i) {
            if (hobbies[i].hobbyId == hobbyId) {
                hobbies[i].name = newName;
                if (hobbies[i].navButton) hobbies[i].navButton->setText(newName);
                if (hobbies[i].titleLabel) hobbies[i].titleLabel->setText(newName);
                int comboIdx = hobbySelector->findData(hobbyId);
                if (comboIdx >= 0) {
                    hobbySelector->setItemText(comboIdx, newName);
                }
                break;
            }
        }
        updateOverviewChart();
    });

    connect(apiManager, &ApiManager::hobbyDeleted, this, [this](const QString &hobbyId) {
        removeHobbyLocally(hobbyId);
        updateOverviewChart();
        btnMain->setChecked(true);
        stackedWidget->setCurrentIndex(0);
    });

    connect(apiManager, &ApiManager::hobbiesStatsLoaded, this, [this](const QJsonObject &response) {
        QJsonObject commonStats = response["common_stats"].toObject();
        commonTotalHoursLabel->setText(QString::number(commonStats["total_hours"].toDouble(), 'f', 1) + " ч");
        commonActivitiesLabel->setText(QString::number(commonStats["activities"].toInt()));
        commonHoursPerDayLabel->setText(QString::number(commonStats["hours_per_day"].toDouble(), 'f', 1) + " ч/д");

        QJsonArray statsArray = response["hobbies_stats"].toArray();
        QStringList activeIds;

        for (const QJsonValue &val : statsArray) {
            QJsonObject itemObj = val.toObject();
            QString id = itemObj["hobby_id"].toString();
            QString name = itemObj["hobby_name"].toString();
            QJsonObject st = itemObj["stats"].toObject();

            activeIds.append(id);
            upsertHobby(
                id,
                name,
                st["total_hours"].toDouble(),
                st["activities"].toInt(),
                st["hours_per_day"].toDouble()
                );
        }

        for (int i = hobbies.size() - 1; i >= 0; --i) {
            if (!activeIds.contains(hobbies[i].hobbyId)) {
                removeHobbyLocally(hobbies[i].hobbyId);
            }
        }

        updateOverviewChart();
    });

    connect(apiManager, &ApiManager::hobbyStatsLoaded, this, [this](const QJsonObject &response) {
        QString id = response["hobby_id"].toString();
        QString name = response["hobby_name"].toString();
        QJsonObject st = response["stats"].toObject();
        upsertHobby(
            id,
            name,
            st["total_hours"].toDouble(),
            st["activities"].toInt(),
            st["hours_per_day"].toDouble()
            );
    });

    connect(apiManager, &ApiManager::activityAdded, this, [this]() {
        requestStatsRefresh();
    });

    connect(apiManager, &ApiManager::apiError, this, [this](const QString &errorMessage) {
        QMessageBox::warning(this, "Ошибка API", errorMessage);
    });
}

QWidget* MainWindow::createStatCard(const QString &caption, QLabel **valueLabelOut)
{
    QWidget *card = new QWidget(this);
    card->setObjectName("metricCard");

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setSpacing(6);

    QLabel *capLabel = new QLabel(caption, card);
    capLabel->setObjectName("metricCaption");

    QLabel *valLabel = new QLabel("0", card);
    valLabel->setObjectName("metricValue");

    layout->addWidget(capLabel);
    layout->addWidget(valLabel);

    *valueLabelOut = valLabel;
    return card;
}

QWidget* MainWindow::createMainTab()
{
    QWidget *tabWidget = new QWidget();
    QVBoxLayout *outerLayout = new QVBoxLayout(tabWidget);
    outerLayout->setAlignment(Qt::AlignCenter);

    QWidget *timerCard = new QWidget(tabWidget);
    timerCard->setObjectName("timerCard");
    timerCard->setFixedSize(480, 420);

    QVBoxLayout *cardLayout = new QVBoxLayout(timerCard);
    cardLayout->setAlignment(Qt::AlignCenter);
    cardLayout->setContentsMargins(36, 32, 36, 32);
    cardLayout->setSpacing(16);

    QLabel *selectLabel = new QLabel("Выберите хобби для трекинга", timerCard);
    selectLabel->setObjectName("mutedLabel");
    selectLabel->setAlignment(Qt::AlignCenter);

    hobbySelector = new QComboBox(timerCard);
    hobbySelector->setObjectName("hobbySelector");
    hobbySelector->setCursor(Qt::PointingHandCursor);

    noteInput = new QLineEdit(timerCard);
    noteInput->setObjectName("noteInput");
    noteInput->setPlaceholderText("Заметка к сессии (необязательно)");

    stopwatchLabel = new QLabel("00:00:00", timerCard);
    stopwatchLabel->setObjectName("stopwatchLabel");
    stopwatchLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(12);

    QPushButton *btnStart = new QPushButton("Старт", timerCard);
    btnStart->setObjectName("btnPrimary");
    btnStart->setCursor(Qt::PointingHandCursor);

    QPushButton *btnPause = new QPushButton("Пауза", timerCard);
    btnPause->setObjectName("btnWarning");
    btnPause->setCursor(Qt::PointingHandCursor);

    QPushButton *btnStop = new QPushButton("Завершить", timerCard);
    btnStop->setObjectName("btnDanger");
    btnStop->setCursor(Qt::PointingHandCursor);

    btnPause->hide();
    btnStop->hide();

    buttonsLayout->addWidget(btnStart);
    buttonsLayout->addWidget(btnPause);
    buttonsLayout->addWidget(btnStop);

    cardLayout->addWidget(selectLabel);
    cardLayout->addWidget(hobbySelector);
    cardLayout->addWidget(noteInput);
    cardLayout->addSpacing(6);
    cardLayout->addWidget(stopwatchLabel);
    cardLayout->addSpacing(6);
    cardLayout->addLayout(buttonsLayout);

    outerLayout->addWidget(timerCard);

    sessionSeconds = 0;
    sessionTimer = new QTimer(this);

    connect(sessionTimer, &QTimer::timeout, this, [this]() {
        sessionSeconds++;
        QTime time(0, 0, 0);
        time = time.addSecs(sessionSeconds);
        stopwatchLabel->setText(time.toString("hh:mm:ss"));
    });

    connect(btnStart, &QPushButton::clicked, this, [this, btnStart, btnPause, btnStop]() {
        if (hobbySelector->count() == 0) {
            QMessageBox::information(this, "Нет хобби", "Сначала добавьте хотя бы одно хобби в боковой панели.");
            return;
        }
        if (!sessionTimer->isActive()) {
            if (sessionSeconds == 0) {
                currentSessionStartedAt = QDateTime::currentDateTimeUtc();
            }
            sessionTimer->start(1000);
            hobbySelector->setEnabled(false);
            btnStart->hide();
            btnPause->show();
            btnStop->show();
        }
    });

    connect(btnPause, &QPushButton::clicked, this, [this, btnStart, btnPause]() {
        if (sessionTimer->isActive()) {
            sessionTimer->stop();
            btnPause->hide();
            btnStart->setText("Продолжить");
            btnStart->show();
        }
    });

    connect(btnStop, &QPushButton::clicked, this, [this, btnStart, btnPause, btnStop]() {
        sessionTimer->stop();
        hobbySelector->setEnabled(true);

        if (sessionSeconds > 0) {
            totalSessionSeconds += sessionSeconds;
            QTime totalTime(0, 0, 0);
            totalTime = totalTime.addSecs(totalSessionSeconds);
            totalTimeLabel->setText(totalTime.toString("hh:mm:ss"));

            QString selectedHobbyId = hobbySelector->currentData().toString();
            int durationMinutes = qMax(1, sessionSeconds / 60);
            QDateTime startedAt = currentSessionStartedAt.isValid()
                                      ? currentSessionStartedAt
                                      : QDateTime::currentDateTimeUtc().addSecs(-sessionSeconds);

            if (!selectedHobbyId.isEmpty()) {
                apiManager->addActivity(selectedHobbyId, durationMinutes, startedAt, noteInput->text());
            }
        }

        sessionSeconds = 0;
        noteInput->clear();
        stopwatchLabel->setText("00:00:00");
        btnStart->setText("Старт");
        btnStart->show();
        btnPause->hide();
        btnStop->hide();
    });

    return tabWidget;
}

QWidget* MainWindow::createOverviewTab()
{
    QWidget *tabWidget = new QWidget();
    QVBoxLayout *tabLayout = new QVBoxLayout(tabWidget);
    tabLayout->setContentsMargins(28, 28, 28, 28);
    tabLayout->setSpacing(18);

    QHBoxLayout *headerRow = new QHBoxLayout();
    QLabel *headerLabel = new QLabel("Общая статистика", tabWidget);
    headerLabel->setObjectName("pageHeader");

    QLabel *fromLabel = new QLabel("С:", tabWidget);
    fromLabel->setObjectName("mutedLabel");
    dateFromEdit = new QDateEdit(QDate::currentDate().addDays(-30), tabWidget);
    dateFromEdit->setCalendarPopup(true);
    dateFromEdit->setDisplayFormat("yyyy-MM-dd");

    QLabel *toLabel = new QLabel("По:", tabWidget);
    toLabel->setObjectName("mutedLabel");
    dateToEdit = new QDateEdit(QDate::currentDate(), tabWidget);
    dateToEdit->setCalendarPopup(true);
    dateToEdit->setDisplayFormat("yyyy-MM-dd");

    QPushButton *btnApplyDates = new QPushButton("Обновить", tabWidget);
    btnApplyDates->setObjectName("btnSecondary");
    btnApplyDates->setCursor(Qt::PointingHandCursor);

    headerRow->addWidget(headerLabel);
    headerRow->addStretch();
    headerRow->addWidget(fromLabel);
    headerRow->addWidget(dateFromEdit);
    headerRow->addWidget(toLabel);
    headerRow->addWidget(dateToEdit);
    headerRow->addWidget(btnApplyDates);
    tabLayout->addLayout(headerRow);

    QHBoxLayout *cardsRow = new QHBoxLayout();
    cardsRow->setSpacing(16);
    cardsRow->addWidget(createStatCard("ВСЕГО ЧАСОВ", &commonTotalHoursLabel));
    cardsRow->addWidget(createStatCard("ВСЕГО АКТИВНОСТЕЙ", &commonActivitiesLabel));
    cardsRow->addWidget(createStatCard("В СРЕДНЕМ В ДЕНЬ", &commonHoursPerDayLabel));
    tabLayout->addLayout(cardsRow);

    overviewPieSeries = new QPieSeries();

    QChart *chart = new QChart();
    chart->addSeries(overviewPieSeries);
    chart->setTitle("Распределение времени по хобби");
    chart->setBackgroundRoundness(12);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setObjectName("chartCard");

    tabLayout->addWidget(chartView, 1);

    connect(btnApplyDates, &QPushButton::clicked, this, &MainWindow::requestStatsRefresh);

    return tabWidget;
}

QWidget* MainWindow::createHobbyTab(HobbyItem &hobby)
{
    QWidget *tabWidget = new QWidget();
    QVBoxLayout *tabLayout = new QVBoxLayout(tabWidget);
    tabLayout->setContentsMargins(28, 28, 28, 28);
    tabLayout->setSpacing(20);

    QHBoxLayout *headerRow = new QHBoxLayout();
    hobby.titleLabel = new QLabel(hobby.name, tabWidget);
    hobby.titleLabel->setObjectName("pageHeader");

    QPushButton *btnRename = new QPushButton("Переименовать", tabWidget);
    btnRename->setObjectName("btnSecondary");
    btnRename->setCursor(Qt::PointingHandCursor);

    QPushButton *btnDelete = new QPushButton("Удалить хобби", tabWidget);
    btnDelete->setObjectName("btnOutlineDanger");
    btnDelete->setCursor(Qt::PointingHandCursor);

    headerRow->addWidget(hobby.titleLabel);
    headerRow->addStretch();
    headerRow->addWidget(btnRename);
    headerRow->addWidget(btnDelete);
    tabLayout->addLayout(headerRow);

    QHBoxLayout *cardsRow = new QHBoxLayout();
    cardsRow->setSpacing(16);
    cardsRow->addWidget(createStatCard("ВСЕГО ЧАСОВ", &hobby.totalHoursValueLabel));
    cardsRow->addWidget(createStatCard("КОЛИЧЕСТВО СЕССИЙ", &hobby.activitiesValueLabel));
    cardsRow->addWidget(createStatCard("ЧАСОВ В ДЕНЬ", &hobby.hoursPerDayValueLabel));
    tabLayout->addLayout(cardsRow);

    tabLayout->addStretch();

    QString hobbyId = hobby.hobbyId;

    connect(btnRename, &QPushButton::clicked, this, [this, hobbyId]() {
        for (const HobbyItem &item : hobbies) {
            if (item.hobbyId == hobbyId) {
                showRenameHobbyDialog(hobbyId, item.name);
                break;
            }
        }
    });

    connect(btnDelete, &QPushButton::clicked, this, [this, hobbyId]() {
        auto reply = QMessageBox::question(
            this,
            "Удаление хобби",
            "Вы уверены, что хотите удалить это хобби и всю его статистику?",
            QMessageBox::Yes | QMessageBox::No
            );
        if (reply == QMessageBox::Yes) {
            apiManager->deleteHobby(hobbyId);
        }
    });

    return tabWidget;
}

void MainWindow::upsertHobby(const QString &hobbyId, const QString &name, double totalHours, int activities, double hoursPerDay)
{
    for (int i = 0; i < hobbies.size(); ++i) {
        if (hobbies[i].hobbyId == hobbyId) {
            hobbies[i].name = name;
            hobbies[i].totalHours = totalHours;
            hobbies[i].activitiesCount = activities;
            hobbies[i].hoursPerDay = hoursPerDay;

            if (hobbies[i].navButton) hobbies[i].navButton->setText(name);
            if (hobbies[i].titleLabel) hobbies[i].titleLabel->setText(name);
            int comboIdx = hobbySelector->findData(hobbyId);
            if (comboIdx >= 0) {
                hobbySelector->setItemText(comboIdx, name);
            }
            updateHobbyCardValues(hobbies[i]);
            return;
        }
    }

    HobbyItem hobby;
    hobby.hobbyId = hobbyId;
    hobby.name = name;
    hobby.totalHours = totalHours;
    hobby.activitiesCount = activities;
    hobby.hoursPerDay = hoursPerDay;

    QPushButton *btn = new QPushButton(name, this);
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    navButtonGroup->addButton(btn);
    hobbiesListLayout->insertWidget(hobbiesListLayout->count() - 1, btn);
    hobby.navButton = btn;

    QWidget *page = createHobbyTab(hobby);
    hobby.pageWidget = page;
    stackedWidget->addWidget(page);
    updateHobbyCardValues(hobby);

    hobbySelector->addItem(name, hobbyId);

    hobbies.append(hobby);

    connect(btn, &QPushButton::clicked, this, [this, page, hobbyId]() {
        stackedWidget->setCurrentWidget(page);
        apiManager->fetchHobbyStats(hobbyId, dateFromEdit->date(), dateToEdit->date());
    });
}

void MainWindow::removeHobbyLocally(const QString &hobbyId)
{
    for (int i = 0; i < hobbies.size(); ++i) {
        if (hobbies[i].hobbyId == hobbyId) {
            int comboIdx = hobbySelector->findData(hobbyId);
            if (comboIdx >= 0) {
                hobbySelector->removeItem(comboIdx);
            }
            if (hobbies[i].navButton) {
                navButtonGroup->removeButton(hobbies[i].navButton);
                hobbiesListLayout->removeWidget(hobbies[i].navButton);
                hobbies[i].navButton->deleteLater();
            }
            if (hobbies[i].pageWidget) {
                stackedWidget->removeWidget(hobbies[i].pageWidget);
                hobbies[i].pageWidget->deleteLater();
            }
            hobbies.removeAt(i);
            break;
        }
    }
}

void MainWindow::clearAllHobbies()
{
    while (!hobbies.isEmpty()) {
        removeHobbyLocally(hobbies.last().hobbyId);
    }
    sessionTimer->stop();
    sessionSeconds = 0;
    totalSessionSeconds = 0;
    stopwatchLabel->setText("00:00:00");
    totalTimeLabel->setText("00:00:00");
}

void MainWindow::updateHobbyCardValues(HobbyItem &hobby)
{
    if (hobby.totalHoursValueLabel) {
        hobby.totalHoursValueLabel->setText(QString::number(hobby.totalHours, 'f', 1) + " ч");
    }
    if (hobby.activitiesValueLabel) {
        hobby.activitiesValueLabel->setText(QString::number(hobby.activitiesCount));
    }
    if (hobby.hoursPerDayValueLabel) {
        hobby.hoursPerDayValueLabel->setText(QString::number(hobby.hoursPerDay, 'f', 1) + " ч/д");
    }
}

void MainWindow::updateOverviewChart()
{
    if (!overviewPieSeries) return;
    overviewPieSeries->clear();

    const QList<QColor> palette = {
        QColor("#3b82f6"), QColor("#10b981"), QColor("#f59e0b"),
        QColor("#8b5cf6"), QColor("#ec4899"), QColor("#06b6d4")
    };

    for (int i = 0; i < hobbies.size(); ++i) {
        double hours = hobbies[i].totalHours;
        double sliceValue = hours > 0.01 ? hours : 0.05;
        QPieSlice *slice = overviewPieSeries->append(
            QString("%1 (%2 ч)").arg(hobbies[i].name, QString::number(hours, 'f', 1)),
            sliceValue
            );
        slice->setBrush(palette[i % palette.size()]);
        slice->setLabelVisible(hours > 0.01);
    }
}

void MainWindow::showAddHobbyDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Новое хобби");
    dialog.setFixedSize(360, 200);
    dialog.setStyleSheet(R"(
        QDialog { background-color: #ffffff; }
        QLabel#dialogTitle { font-size: 17px; font-weight: bold; color: #1e293b; }
        QLabel#dialogError { font-size: 13px; color: #ef4444; }
        QLineEdit {
            padding: 10px 12px;
            border: 1px solid #cbd5e1;
            border-radius: 8px;
            font-size: 14px;
            color: #1e293b;
            background-color: #f8fafc;
        }
        QLineEdit:focus { border: 2px solid #3b82f6; background-color: #ffffff; }
        QPushButton#btnSave {
            background-color: #3b82f6; color: white; border: none;
            border-radius: 8px; padding: 9px 18px; font-weight: bold; font-size: 14px;
        }
        QPushButton#btnSave:hover { background-color: #2563eb; }
        QPushButton#btnCancel {
            background-color: #e2e8f0; color: #475569; border: none;
            border-radius: 8px; padding: 9px 16px; font-size: 14px;
        }
        QPushButton#btnCancel:hover { background-color: #cbd5e1; }
    )");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(12);

    QLabel *title = new QLabel("Добавить новое хобби", &dialog);
    title->setObjectName("dialogTitle");

    QLineEdit *nameInput = new QLineEdit(&dialog);
    nameInput->setPlaceholderText("Например: Гитара, Чтение, Бег...");

    QLabel *errorLabel = new QLabel("", &dialog);
    errorLabel->setObjectName("dialogError");

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch();

    QPushButton *btnCancel = new QPushButton("Отмена", &dialog);
    btnCancel->setObjectName("btnCancel");
    btnCancel->setCursor(Qt::PointingHandCursor);

    QPushButton *btnSave = new QPushButton("Сохранить", &dialog);
    btnSave->setObjectName("btnSave");
    btnSave->setCursor(Qt::PointingHandCursor);
    btnSave->setDefault(true);

    btnRow->addWidget(btnCancel);
    btnRow->addWidget(btnSave);

    layout->addWidget(title);
    layout->addWidget(nameInput);
    layout->addWidget(errorLabel);
    layout->addStretch();
    layout->addLayout(btnRow);

    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);

    connect(btnSave, &QPushButton::clicked, &dialog, [&]() {
        QString trimmedName = nameInput->text().trimmed();
        if (trimmedName.isEmpty()) {
            errorLabel->setText("Введите название хобби");
            return;
        }
        for (const HobbyItem &existing : hobbies) {
            if (existing.name.compare(trimmedName, Qt::CaseInsensitive) == 0) {
                errorLabel->setText("Такое хобби уже существует");
                return;
            }
        }
        apiManager->createHobby(trimmedName);
        dialog.accept();
    });

    dialog.exec();
}

void MainWindow::showRenameHobbyDialog(const QString &hobbyId, const QString &currentName)
{
    QDialog dialog(this);
    dialog.setWindowTitle("Переименовать хобби");
    dialog.setFixedSize(360, 200);
    dialog.setStyleSheet(R"(
        QDialog { background-color: #ffffff; }
        QLabel#dialogTitle { font-size: 17px; font-weight: bold; color: #1e293b; }
        QLabel#dialogError { font-size: 13px; color: #ef4444; }
        QLineEdit {
            padding: 10px 12px;
            border: 1px solid #cbd5e1;
            border-radius: 8px;
            font-size: 14px;
            color: #1e293b;
            background-color: #f8fafc;
        }
        QLineEdit:focus { border: 2px solid #3b82f6; background-color: #ffffff; }
        QPushButton#btnSave {
            background-color: #3b82f6; color: white; border: none;
            border-radius: 8px; padding: 9px 18px; font-weight: bold; font-size: 14px;
        }
        QPushButton#btnSave:hover { background-color: #2563eb; }
        QPushButton#btnCancel {
            background-color: #e2e8f0; color: #475569; border: none;
            border-radius: 8px; padding: 9px 16px; font-size: 14px;
        }
        QPushButton#btnCancel:hover { background-color: #cbd5e1; }
    )");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(12);

    QLabel *title = new QLabel("Новое название хобби", &dialog);
    title->setObjectName("dialogTitle");

    QLineEdit *nameInput = new QLineEdit(currentName, &dialog);

    QLabel *errorLabel = new QLabel("", &dialog);
    errorLabel->setObjectName("dialogError");

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch();

    QPushButton *btnCancel = new QPushButton("Отмена", &dialog);
    btnCancel->setObjectName("btnCancel");
    btnCancel->setCursor(Qt::PointingHandCursor);

    QPushButton *btnSave = new QPushButton("Сохранить", &dialog);
    btnSave->setObjectName("btnSave");
    btnSave->setCursor(Qt::PointingHandCursor);
    btnSave->setDefault(true);

    btnRow->addWidget(btnCancel);
    btnRow->addWidget(btnSave);

    layout->addWidget(title);
    layout->addWidget(nameInput);
    layout->addWidget(errorLabel);
    layout->addStretch();
    layout->addLayout(btnRow);

    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);

    connect(btnSave, &QPushButton::clicked, &dialog, [&]() {
        QString trimmedName = nameInput->text().trimmed();
        if (trimmedName.isEmpty()) {
            errorLabel->setText("Введите название хобби");
            return;
        }
        if (trimmedName != currentName) {
            apiManager->renameHobby(hobbyId, trimmedName);
        }
        dialog.accept();
    });

    dialog.exec();
}

void MainWindow::applyStyles()
{
    QString styleSheet = R"(
        MainWindow, #contentArea {
            background-color: #f8fafc;
        }

        #sideBar {
            background-color: #1e293b;
        }

        #logoLabel {
            color: #f8fafc;
            font-size: 21px;
            font-weight: bold;
            padding-left: 6px;
        }

        #sideDivider {
            background-color: #334155;
            border: none;
        }

        #sectionTitle {
            color: #94a3b8;
            font-size: 11px;
            font-weight: bold;
            letter-spacing: 1px;
        }

        #hobbyScrollArea, #scrollContainer {
            background-color: transparent;
        }

        QScrollBar:vertical {
            background: transparent;
            width: 5px;
        }
        QScrollBar::handle:vertical {
            background: #475569;
            border-radius: 2px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }

        #sideBar QPushButton {
            background-color: transparent;
            color: #cbd5e1;
            border: none;
            padding: 10px 12px;
            text-align: left;
            font-size: 14px;
            border-radius: 8px;
        }

        #sideBar QPushButton:hover {
            background-color: #334155;
            color: #ffffff;
        }

        #sideBar QPushButton:checked {
            background-color: #3b82f6;
            color: #ffffff;
            font-weight: bold;
        }

        #sideBar QPushButton#btnAddHobby {
            color: #60a5fa;
            font-size: 12px;
            font-weight: bold;
            padding: 4px 8px;
            text-align: center;
            border-radius: 6px;
        }

        #sideBar QPushButton#btnAddHobby:hover {
            background-color: rgba(59, 130, 246, 0.18);
            color: #93c5fd;
        }

        #sessionCard {
            background-color: #0f172a;
            border: 1px solid #334155;
            border-radius: 10px;
        }

        #sessionCaption {
            color: #94a3b8;
            font-size: 10px;
            font-weight: bold;
            letter-spacing: 1px;
        }

        #totalTimeLabel {
            color: #f8fafc;
            font-size: 20px;
            font-weight: bold;
        }

        #sideBar QPushButton#btnLeave {
            color: #94a3b8;
            margin-top: 4px;
        }

        #sideBar QPushButton#btnLeave:hover {
            background-color: rgba(239, 68, 68, 0.15);
            color: #f87171;
        }

        #timerCard {
            background-color: #ffffff;
            border: 1px solid #e2e8f0;
            border-radius: 16px;
        }

        #mutedLabel {
            color: #64748b;
            font-size: 14px;
            font-weight: 500;
        }

        #hobbySelector {
            background-color: #f8fafc;
            color: #1e293b;
            border: 1px solid #cbd5e1;
            border-radius: 8px;
            padding: 8px 14px;
            font-size: 15px;
            font-weight: bold;
            min-width: 260px;
        }

        #hobbySelector:hover {
            border-color: #3b82f6;
        }

        #noteInput {
            background-color: #f8fafc;
            color: #1e293b;
            border: 1px solid #cbd5e1;
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 13px;
        }

        #noteInput:focus {
            border: 2px solid #3b82f6;
            background-color: #ffffff;
        }

        #stopwatchLabel {
            color: #0f172a;
            font-size: 54px;
            font-weight: bold;
        }

        #timerCard QPushButton {
            border: none;
            border-radius: 10px;
            padding: 12px 24px;
            font-size: 15px;
            font-weight: bold;
            text-align: center;
            min-width: 130px;
            color: #ffffff;
        }

        #btnPrimary { background-color: #3b82f6; }
        #btnPrimary:hover { background-color: #2563eb; }

        #btnWarning { background-color: #f59e0b; }
        #btnWarning:hover { background-color: #d97706; }

        #btnDanger { background-color: #ef4444; }
        #btnDanger:hover { background-color: #dc2626; }

        #btnSecondary {
            background-color: #e2e8f0;
            color: #1e293b;
            border: none;
            border-radius: 8px;
            padding: 8px 14px;
            font-size: 13px;
            font-weight: bold;
        }
        #btnSecondary:hover {
            background-color: #cbd5e1;
        }

        #btnOutlineDanger {
            background-color: transparent;
            color: #ef4444;
            border: 1px solid #fca5a5;
            border-radius: 8px;
            padding: 8px 14px;
            font-size: 13px;
            font-weight: bold;
        }
        #btnOutlineDanger:hover {
            background-color: #fef2f2;
        }

        #pageHeader {
            color: #0f172a;
            font-size: 24px;
            font-weight: bold;
        }

        #metricCard {
            background-color: #ffffff;
            border: 1px solid #e2e8f0;
            border-radius: 12px;
        }

        #metricCaption {
            color: #64748b;
            font-size: 11px;
            font-weight: bold;
            letter-spacing: 0.8px;
        }

        #metricValue {
            color: #0f172a;
            font-size: 26px;
            font-weight: bold;
        }

        QDateEdit {
            background-color: #ffffff;
            color: #1e293b;
            border: 1px solid #cbd5e1;
            border-radius: 8px;
            padding: 6px 10px;
            font-size: 13px;
        }
    )";

    this->setStyleSheet(styleSheet);
}