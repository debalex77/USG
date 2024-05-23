#ifndef UPDATERELEASESAPP_H
#define UPDATERELEASESAPP_H

#include <QObject>
#include <data/globals.h>
#include <data/database.h>
#include <data/loggingcategories.h>

#define release_1_1_3 "1.1.3"
#define release_1_1_4 "1.1.4"
#define release_1_1_5 "1.1.5"
#define release_2_0_4 "2.0.4"
#define release_2_0_5 "2.0.5"
#define release_2_0_6 "2.0.6"
#define release_2_0_7 "2.0.7"
#define release_2_0_8 "2.0.8"
#define release_2_0_9 "2.0.9"

class UpdateReleasesApp : public QObject
{
    Q_OBJECT
public:
    explicit UpdateReleasesApp(QObject* parent = nullptr);
    ~UpdateReleasesApp();

    bool execUpdateCurrentRelease(const QString current_release);

private:
    int converVersionStringToNumber(const QString current_release) const;
    bool updateRelease_2_0_9();

private:
    DataBase *db;
};

#endif // UPDATERELEASESAPP_H
