#ifndef DATABASESELECTION_H
#define DATABASESELECTION_H

#include <QDialog>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTimer>
#include <QStyleFactory>
#include "data/database.h"

#if defined(Q_OS_LINUX)
    #define NAME_DIR_CONFIG_PATH  "/.config/USG"
    #define NAME_FILE_CONFIG_PATH "/.config/USG/settings.conf"
#elif defined(Q_OS_MACOS)
    #define NAME_DIR_CONFIG_PATH  "/.config/USG"
    #define NAME_FILE_CONFIG_PATH "/.config/USG/settings.conf"
    #define NAME_DIR_LOG_PATH     "/.local/USG/logs"
#elif defined(Q_OS_WIN)
#endif

namespace Ui {
class DatabaseSelection;
}

class DatabaseSelection : public QDialog
{
    Q_OBJECT

public:
    explicit DatabaseSelection(QWidget *parent = nullptr);
    ~DatabaseSelection();

    QString str_name_base = nullptr;

private slots:
    void readFileSettings(const QString pathToFile);
    void onCurrentRowChanged(const int row);

    void onConnectToBase();
    void onAddDatabase();
    void onRemoveRowListWidget();

    void updateTimer();

private:
    Ui::DatabaseSelection *ui;
    DataBase* db;
    QTimer* timer;

#if defined(Q_OS_LINUX)
    QString dirConfigPath  = QDir::homePath() + NAME_DIR_CONFIG_PATH;
    QString fileConfigPath = QDir::homePath() + NAME_FILE_CONFIG_PATH;
#elif defined(Q_OS_MACOS)
    QString dirConfigPath  = QDir::homePath() + NAME_DIR_CONFIG_PATH;
    QString fileConfigPath = QDir::homePath() + NAME_FILE_CONFIG_PATH;
    QString dirLogPath     = QDir::homePath() + NAME_DIR_LOG_PATH;
#elif defined(Q_OS_WIN)
    QString dirConfigPath  = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config";
    QString fileConfigPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config/settings.conf";
#endif

    QStyle* style_fusion = QStyleFactory::create("Fusion");
};

#endif // DATABASESELECTION_H
