#ifndef UPDATERELEASESAPP_H
#define UPDATERELEASESAPP_H

#include <QObject>
#include <data/globals.h>
#include <data/database.h>
#include <data/loggingcategories.h>

class UpdateReleasesApp : public QObject
{
    Q_OBJECT
public:
    explicit UpdateReleasesApp(QObject* parent = nullptr);
    ~UpdateReleasesApp();

    bool execUpdateCurrentRelease(const QString current_release);
    void initializeUpdateFunctions();

private:
    void updateRelease_2_0_4();
    void updateRelease_2_0_5();
    void updateRelease_2_0_6();
    void updateRelease_2_0_7();
    void updateRelease_2_0_9();

    void updateRelease_3_0_1();
    void updateTablePacients_release_3_0_1();
    void updateTableKidney_release_3_0_1();
    void updateTableIntestinalLoops_release_3_0_1();
    void updateTableGynecology_release_3_0_1();
    void updateTableformationsSystemTemplates_release_3_0_1();
    void createIndex_release_3_0_1();
    void createIndexForBaseImage_3_0_1();

    void updateRelease_3_0_3();

    bool existErrFormationsSystemTemplates();
    void updateRelease_3_0_6();
    void updateRelease_3_0_7();

private:
    DataBase* db;
    QMap<int, QMap<int, QMap<int, std::function<void()>>>> updateFunctions;
};

#endif // UPDATERELEASESAPP_H
