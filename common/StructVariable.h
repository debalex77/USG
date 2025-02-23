#ifndef STRUCTVARIABLE_H
#define STRUCTVARIABLE_H

#include <QDir>
#include <QPixmapCache>
#include <QString>

// Structură pentru toate variabilele globale
struct GlobalVariable {

    // Tipuri complexe mari
    QPixmapCache cache_img;

    // QByteArray-uri
    QByteArray c_logo_byteArray        = nullptr;  // constanta logotipilui
    QByteArray main_stamp_organization = nullptr;  // stampila organizatiei
    QByteArray stamp_main_doctor       = nullptr;  // stampila doctorului implicit
    QByteArray signature_main_doctor   = nullptr;  // semnatura doctorului implicit

    // QString-uri lungi (texte mari, HTML)
    QString str_content_message_video;
    QString str_content_message_report;

    // QString-uri pentru setări și căi
    QString pathImageBaseAppSettings = nullptr;
    QString pathTemplatesDocs = nullptr;
    QString pathReports = nullptr;
    QString pathAppSettings = nullptr;
    QString pathLogAppSettings = nullptr;
    QString sqliteNameBase = nullptr;
    QString sqlitePathBase = nullptr;

    QString mySQLhost = nullptr;
    QString mySQLnameBase = nullptr;
    QString mySQLport = nullptr;
    QString mySQLoptionConnect = nullptr;
    QString mySQLuser = nullptr;
    QString mySQLpasswdUser = nullptr;

    QString langApp = nullptr;
    QString unitMeasure = nullptr;
    QString nameUserApp = nullptr;
    QString c_brandUSG = nullptr;

    QString main_name_organization = nullptr;
    QString main_email_organization = nullptr;
    QString main_phone_organization = nullptr;
    QString main_addres_organization = nullptr;
    QString main_path_save_documents = QDir::toNativeSeparators(QDir::tempPath() + "/USG");
    QString main_name_doctor = nullptr;
    QString main_name_abbreviat_doctor = nullptr;

    QString connectionMade = nullptr;
    QString pathDirectoryVideo = nullptr;

    // int-uri
    int moveApp = -1;
    int indexTypeSQL = -1;
    int idUserApp = -1;
    int c_id_organizations = -1;
    int c_id_doctor = -1;
    int c_id_nurse = -1;

    int updateIntervalListDoc = 0;
    int numSavedFilesLog = -1;

    // bool-uri (la final)
    bool unknowModeLaunch = false;
    bool isSystemThemeDark = false;
    bool firstLaunch = false;
    bool thisMySQL = false;
    bool thisSqlite = false;
    bool thisSqlCipher = false;

    bool showUserManual = false;
    bool showHistoryVersion = false;
    bool memoryUser = false;
    bool showQuestionCloseApp = false;
    bool order_splitFullName = false;
    bool showDesignerMenuPrint = false;
    bool checkNewVersionApp = false;
    bool databasesArchiving = false;
    bool showAsistantHelper = false;
    bool showDocumentsInSeparatWindow = false;
    bool minimizeAppToTray = false;
    bool show_content_info_video = true;
    bool show_info_reports = true;

};

#endif // STRUCTVARIABLE_H
