#ifndef STRUCTVARIABLE_H
#define STRUCTVARIABLE_H

#include <QDir>
#include <QPixmapCache>
#include <QStandardPaths>
#include <QString>

// Structură pentru toate variabilele globale
struct GlobalVariable {

    // Tipuri complexe mari
    QPixmapCache cache_img;

    // QByteArray-uri
    QByteArray organizationLogoData  = nullptr;  // constanta logotipilui
    QByteArray organizationStampData = nullptr;  // stampila organizatiei
    QByteArray organizationDoctorStampData = nullptr;     // stampila doctorului implicit
    QByteArray organizationDoctorSignatureData = nullptr; // semnatura doctorului implicit

    // QString-uri lungi (texte mari, HTML)
    QString str_content_message_video;
    QString str_content_message_report;

    // path
    QString imageDatabasePath = nullptr;
    QString docsTemplatesPath = nullptr;
    QString reportsPath       = nullptr;
    QString settingsPath      = nullptr;
    QString logPath           = nullptr;

    // sqlite
    QString sqliteDatabaseName = nullptr;
    QString sqliteDatabasePath = nullptr;

    // setarile dimensiunilor ferestrelor, sectiilor tabelelor si setarile rapoartelor
    QString config_dir = QDir::toNativeSeparators(
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/USG"
        );
    QString json_dir           = QDir::toNativeSeparators(config_dir + "/settings");
    QString pathSettingsCommon = QDir::toNativeSeparators(json_dir + "/table_settings.json");
    QString pathSettingsReport = QDir::toNativeSeparators(json_dir + "/report_settings.json");

    QString mySQLhost          = nullptr;
    QString mySQLnameBase      = nullptr;
    QString mySQLport          = nullptr;
    QString mySQLoptionConnect = nullptr;
    QString mySQLuser          = nullptr;
    QString mySQLpasswdUser    = nullptr;

    QString cloud_host          = nullptr;
    QString cloud_nameBase      = nullptr;
    QString cloud_port          = nullptr;
    QString cloud_optionConnect = nullptr;
    QString cloud_user          = nullptr;
    QString cloud_passwd        = nullptr;

    QString langApp     = nullptr;
    QString unitMeasure = nullptr;
    QString nameUserApp = nullptr;

    QString organizationName    = nullptr;
    QString organizationEmail   = nullptr;
    QString organizationSite    = nullptr;
    QString organizationPhone   = nullptr;
    QString organizationAddress = nullptr;
    QString organizationBrandUSG = nullptr;
    QString organizationDoctorName = nullptr;
    QString organizationDoctorAbbreviatedName = nullptr;

    // directory
    QString exportDirectory = QDir::toNativeSeparators(QDir::tempPath() + "/USG");
    QString videoDirectory  = nullptr;

    QString connectionMade = nullptr;

    // int-uri
    int moveApp              = -1;
    int indexTypeSQL         = -1;
    int idUserApp            = -1;
    int organizationID       = -1;
    int organizationDoctorID = -1;
    int organizationNurseID  = -1;

    int updateIntervalListDoc = 0;
    int numSavedFilesLog      = -1;

    // bool-uri (la final)
    bool unknowModeLaunch  = false;
    bool isSystemThemeDark = false;
    bool firstLaunch       = false;
    bool thisMySQL         = false;
    bool thisSqlite        = false;
    bool thisSqlCipher     = false;

    bool cloud_srv_exist       = false;
    bool cloud_configured      = false;
    bool showUserManual        = false;
    bool showHistoryVersion    = false;
    bool memoryUser            = false;
    bool showQuestionCloseApp  = false;
    bool order_splitFullName   = false;
    bool showDesignerMenuPrint = false;
    bool checkNewVersionApp    = false;
    bool databasesArchiving    = false;
    bool showAsistantHelper    = false;
    bool showDocumentsInSeparatWindow = false;
    bool minimizeAppToTray       = false;
    bool show_content_info_video = true;
    bool show_info_reports       = true;
};

#endif // STRUCTVARIABLE_H
