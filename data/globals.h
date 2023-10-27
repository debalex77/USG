#pragma once

#include <QPixmapCache>
#include <QString>

namespace globals {

extern bool unknowModeLaunch;
extern int moveApp; // unknow=-1, move=1, not_move=0

extern QString pathAppSettings;           // drumul spre localizare setarilor aplicatiei
extern QString pathImageBaseAppSettings;  // drumul spre BD cu imagini
extern QString pathLogAppSettings;        // drumul spre fisierul de logare
extern QString pathTemplatesDocs;
extern QString pathReports;

extern QString sqliteNameBase;  // denumirea BD .sqlite3
extern QString sqlitePathBase;  // drumul spre fisierul BD .sqlite3

extern QString mySQLhost;           // date de conectarea MySQL
extern QString mySQLnameBase;
extern QString mySQLport;
extern QString mySQLoptionConnect;
extern QString mySQLuser;
extern QString mySQLpasswdUser;

extern bool thisMySQL;      // semnul MySQL
extern bool thisSqlite;

extern QString langApp;                 // codul limbei aplicatiei (ro_RO)
extern bool firstLaunch;                // prima lansare a aplicatiei
extern int indexTypeSQL;                // tipul BD (.sqlite3/MySQL)

extern bool showUserManual;     // prezentarea User manual
extern bool showHistoryVersion; // prezentarea History version

extern QString unitMeasure; // unitatea de masura (milimetri, centimetri)

extern int idUserApp;       // id utilizatorului logat
extern QString nameUserApp; // nume utilizatorului logat
extern bool memoryUser;     // memorarea numelui user

extern bool showQuestionCloseApp;
extern bool order_splitFullName;
extern int updateIntervalListDoc;
extern bool showDesignerMenuPrint;

extern int numSavedFilesLog; // fisiere de logare pastrate

extern QString connectionMade;

extern QPixmapCache cache_img;
}
