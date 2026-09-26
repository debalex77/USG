#ifndef DATABASESELECTION_H
#define DATABASESELECTION_H

#include <QDialog>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTimer>
#include <QStyleFactory>
#include "common/applicationpathscontext.h"
#include "database/database.h"

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
    DataBase *db;
    QTimer   *timer;

    QString dirConfigPath  = ApplicationPathsContext::instance().configDirectory();
    QString fileConfigPath = QDir(dirConfigPath).filePath(QStringLiteral("settings.conf"));

    QStyle* style_fusion = QStyleFactory::create("Fusion");
};

#endif // DATABASESELECTION_H
