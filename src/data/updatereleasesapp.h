#ifndef UPDATERELEASESAPP_H
#define UPDATERELEASESAPP_H

#include <QObject>
#include <QVersionNumber>
#include <QGuiApplication>

#include <common/globals.h>
#include <data/database.h>
#include <data/database_common.h>
#include <data/loggingcategories.h>
#include <common/version.h>

class DataBase;
class DataBaseCommon;

class UpdateReleasesApp : public QObject
{
    Q_OBJECT
public:
    explicit UpdateReleasesApp(QObject* parent = nullptr);
    ~UpdateReleasesApp();

    bool execUpdateCurrentRelease(const QString currentRelease);
    bool ensureRequiredViews();

signals:
    void migrationProgress(int value, int maximum, const QString &message);

private:

    bool validateMigrationContext(const QVersionNumber &current,
                                  const QVersionNumber &target) const;
    bool validateMigrationPrerequisites(const QVersionNumber &migrationVersion) const;
    bool validatePostMigration(const QVersionNumber &migrationVersion) const;
    bool transferSqliteUuidsToCloud();
    bool ensurePatientAppointmentsSchema();

    bool update_2_0_4();
    bool update_2_0_5();
    bool update_2_0_6();
    bool update_2_0_7();
    bool update_2_0_9();

    bool update_3_0_1();
    bool updateTablePacients_release_3_0_1();
    bool updateTableKidney_release_3_0_1();
    bool updateTableIntestinalLoops_release_3_0_1();
    bool updateTableGynecology_release_3_0_1();
    bool updateTableformationsSystemTemplates_release_3_0_1();
    void createIndex_release_3_0_1();
    void createIndexForBaseImage_3_0_1();

    bool update_3_0_3();

    bool existErrFormationsSystemTemplates();
    bool update_3_0_6();
    bool update_3_0_7();
    bool update_4_0_1();
    bool update_4_1_0();
    bool update_4_1_2();

private:
    DataBase* db;
    DataBaseCommon db_common;
};

#endif // UPDATERELEASESAPP_H
