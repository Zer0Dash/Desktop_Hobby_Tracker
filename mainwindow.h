#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "apimanager.h"

#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <QScrollArea>
#include <QComboBox>
#include <QDateEdit>
#include <QLineEdit>
#include <QTimer>
#include <QTime>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QList>
#include <QString>

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

struct HobbyItem {
    QString hobbyId;
    QString name;
    double totalHours = 0.0;
    int activitiesCount = 0;
    double hoursPerDay = 0.0;

    QPushButton *navButton = nullptr;
    QWidget *pageWidget = nullptr;
    QLabel *titleLabel = nullptr;
    QLabel *totalHoursValueLabel = nullptr;
    QLabel *activitiesValueLabel = nullptr;
    QLabel *hoursPerDayValueLabel = nullptr;
};

class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(ApiManager *api, QWidget *parent = nullptr);
    virtual ~MainWindow();
    void loadInitialData();

signals:
    void logoutRequested();

private:
    ApiManager *apiManager;

    QStackedWidget *stackedWidget;
    QButtonGroup *navButtonGroup;
    QVBoxLayout *hobbiesListLayout;

    QPushButton *btnMain;
    QPushButton *btnStats;
    QPushButton *btnAddHobby;
    QPushButton *btnLeave;

    QComboBox *hobbySelector;
    QLineEdit *noteInput;
    QTimer *sessionTimer;
    int sessionSeconds;
    int pendingSessionSeconds;
    int totalSessionSeconds;
    QDateTime currentSessionStartedAt;
    QLabel *stopwatchLabel;
    QLabel *totalTimeLabel;

    QDateEdit *dateFromEdit;
    QDateEdit *dateToEdit;
    QLabel *commonTotalHoursLabel;
    QLabel *commonActivitiesLabel;
    QLabel *commonHoursPerDayLabel;
    QPieSeries *overviewPieSeries;

    QList<HobbyItem> hobbies;

    void setupUI();
    void setupApiConnections();
    void applyStyles();
    QWidget* createMainTab();
    QWidget* createOverviewTab();
    QWidget* createHobbyTab(HobbyItem &hobby);
    QWidget* createStatCard(const QString &caption, QLabel **valueLabelOut);

    void upsertHobby(const QString &hobbyId, const QString &name, double totalHours = 0.0, int activities = 0, double hoursPerDay = 0.0);
    void removeHobbyLocally(const QString &hobbyId);
    void clearAllHobbies();
    void showAddHobbyDialog();
    void showRenameHobbyDialog(const QString &hobbyId, const QString &currentName);
    void updateHobbyCardValues(HobbyItem &hobby);
    void updateOverviewChart();
    void requestStatsRefresh();
};

#endif