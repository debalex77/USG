#include "globals.h"
#include <QCache>

namespace globals {

bool unknowModeLaunch = false;
int moveApp = -1; // unknow=-1, move=1, not_move=0

QString pathAppSettings          = nullptr;
QString pathImageBaseAppSettings = nullptr;
QString pathLogAppSettings       = nullptr;
QString pathTemplatesDocs        = nullptr;
QString pathReports              = nullptr;

QString sqliteNameBase = nullptr;
QString sqlitePathBase = nullptr;

QString mySQLhost          = nullptr;
QString mySQLnameBase      = nullptr;
QString mySQLport          = nullptr;
QString mySQLoptionConnect = nullptr;
QString mySQLuser          = nullptr;
QString mySQLpasswdUser    = nullptr;

QString fb_host          = nullptr;  // date de conectarea Firebird
QString fb_nameBase     = nullptr;
QString fb_port          = nullptr;
QString fb_optionConnect = nullptr;
QString fb_user          = nullptr;
QString fb_passwdUser    = nullptr;

bool thisMySQL  = false;
bool thisSqlite = false;

QString langApp  = nullptr;
bool firstLaunch = false;
int indexTypeSQL = -1;

bool showUserManual     = false; // prezentarea User manual
bool showHistoryVersion = false; // prezentarea History version

QString unitMeasure = nullptr;

int idUserApp = -1;
QString nameUserApp = nullptr;
bool memoryUser;

bool showQuestionCloseApp = false;
bool order_splitFullName = false;
int updateIntervalListDoc = 0;
bool showDesignerMenuPrint = false;
bool checkNewVersionApp = false;
bool databasesArchiving = false;
bool showAsistantHelper = false;

int numSavedFilesLog = -1;

QString connectionMade = nullptr;
QPixmapCache cache_img;

QString pathDirectoryVideo = nullptr;

}
