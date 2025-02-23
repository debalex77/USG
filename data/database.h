#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QMap>
#include <QCryptographicHash>
#include <QProgressDialog>

#if (QT_VERSION > QT_VERSION_CHECK(5, 15, 2))
#include <QRegularExpression>
#endif

#include <data/globals.h>
#include <data/loggingcategories.h>

class DataBase : public QObject
{
    Q_OBJECT

public:
    explicit DataBase(QObject* parent = nullptr);
    ~DataBase();

    bool connectToDataBase();   
    bool createConnectBaseSqlite(QString &txtMessage);

    QSqlDatabase getDatabase();
    QSqlDatabase getDatabaseThread(const QString threadConnectionName);
    QSqlDatabase getDatabaseImageThread(const QString threadConnectionName);
    void removeDatabaseThread(const QString threadConnectionName);
    void removeDatabaseImageThread(const QString threadConnectionName);
    QSqlDatabase getDatabaseImage();

    bool deleteDataFromTable(const QString name_table, const QString deletionCondition = nullptr, const QString valueCondition = nullptr);

    void creatingTables();
    bool checkTable(const QString name_table);
    void creatingTables_DbImage();

    void loadInvestigationFromXml();
    void updateInvestigationFromXML_2024();
    void loadNormogramsFromXml();
    void insertDataForTabletypesPrices();
    void insertSetTableSettingsUsers();

    int findIdFromTableSettingsForm(const QString typeForm, const int numberSection) const;
    bool insertUpdateDataTableSettingsForm(const bool insertData, const QString typeForm, const int numberSection, const int sectionSize, int id_data = -1, int directionSorting = -1) const;
    int getSizeSectionFromTableSettingsForm(const int numberSection, const QString typeForm, const int id_data) const;
    int getDirectionSortingFromTableSettingsForm(const int numberSection, const QString typeForm) const;
    bool updatePeriodInTableSettingsForm(const int id_data, const QString date_start, const QString date_end);
    void getPeiodFromTableSettingsForm(const int id_data, const QString typeForm, QString &date_start, QString &date_end);

    bool existNameObject(const QString nameTable, const QString nameObject);
    bool existNameObjectReturnId(const QString nameTable, const QString nameObject, int &_id);
    bool createNewObject(const QString nameTable, QMap<QString, QString> &items);
    bool updateDataObject(const QString nameTable, const int _id, QMap<QString, QString> &items);

    bool execQuery(const QString strQuery);
    bool removeObjectById(const QString nameTable, const int _id);
    int getLastIdForTable(const QString nameTable) const;
    int getLastIdForTableByDatabase(const QString nameTable, QSqlDatabase name_database) const;
    int getLastNumberDoc(const QString nameTable) const;

    bool getObjectDataByMainId(const QString nameTable, const QString nameId, const int _id, QMap<QString, QString> &items);
    bool getDataFromQuery(const QString strQuery, QMap<QString, QString> &items);
    bool getDataFromQueryByRecord(const QString strQuery, QMap<QString, QString> &items) const;

    int statusDeletionMarkObject(const QString nameTable, const int _id) const;
    bool deletionMarkObject(const QString nameTable, const int _id);
    bool getObjectDataById(const QString nameTable, const int _id, QMap<QString, QString> &items);

    bool postDocument(const QString nameTable, QMap<QString, QString> &items);

    bool createTableUsers();
    bool createTableDoctors();
    bool createTableFullNameDoctors();
    bool createTableNurses();
    bool createTableFullNameNurses();
    bool createTablePacients();
    bool createTableFullNamePacients();
    bool createTableTypesPrices();
    bool createTableOrganizations();
    bool createTableContracts();
    bool createTableInvestigations();
    bool createTableInvestigationsGroup();
    bool createTablePricings();
    bool createTable_PricingsTable();
    bool createTablePricingsPresentation();
    bool createTableOrderEcho();
    bool createTable_OrderEchoTable();
    bool createTableOrderEchoPresentation();
    bool createTableConstants();
    bool createTableReportEcho();
    bool createTableReportEchoPresentation();
    bool createTableLiver();
    bool createTableCholecist();
    bool createTablePancreas();
    bool createTableSpleen();
    bool createTableIntestinalLoop();
    bool createTableKidney();
    bool createTableBladder();
    bool createTableProstate();
    bool createTableGynecology();
    bool createTableBreast();
    bool createTableThyroid();
    bool createTableGestation0();
    bool createTableGestation1();

    bool createTableGestation2();
    bool createTableGestation2_biometry();
    bool createTableGestation2_cranium();
    bool createTableGestation2_snc();
    bool createTableGestation2_heart();
    bool createTableGestation2_thorax();
    bool createTableGestation2_abdomen();
    bool createTableGestation2_urinarySystem();
    bool createTableGestation2_other();
    bool createTableGestation2_doppler();

    bool createTableNormograms();

    bool createTableRegistrationPatients();
    bool createTableSettingsForm();
    bool createTableSettingsReports();
    bool createTableSettingsUsers();
    bool createTableUserPreferences();
    bool createTableConclusionTemplates();
    bool createTableFormationsSystemTemplates();
    bool createTableImagesReports();

    bool createIndexTables();
    bool createTableContOnline();

    enum REQUIRED_NUMBER
    {
        _NEGATIV        = -1,
        DELETION_MARK   = 1,
        DELETION_UNMARK = 0
    };

    void updateVariableFromTableSettingsUser();

    void getNameColumnTable(const QString nameTable/*,QMap<QString, QString> &items*/);
    int getCountSectionTableSQLite(const QString nameTable) const;
    QMap<int, QString> getMapDataQuery(const QString strQuery);

    bool existIdDocument(const QString nameTable, const QString name_condition, const QString value_condition, QSqlDatabase nameDatabase);
    bool existSubalternDocument(const QString nameTable, const QString name_condition, const QString value_condition, int &id_doc);
    QString getQryFromTableConstantById(const int id_user) const;
    QString getQryFromTablePatientById(const int id_patient) const;
    QString getQryForTableOrgansInternalById(const int id_doc) const;
    QString getQryForTableUrinarySystemById(const int id_doc) const;
    QString getQryForTableProstateById(const int id_doc) const;
    QString getQryForTableGynecologyById(const int id_doc) const;
    QString getQryForTableBreastById(const int id_doc) const;
    QString getQryForTableThyroidById(const int id_doc) const;
    QString getQryForTableGestation0dById(const int id_doc) const;
    QString getQryForTableGestation1dById(const int id_doc) const;
    QString getQryForTableGestation2(const int id_doc) const;

    QString getQryForTableOrderById(const int id_doc, const QString str_price) const;

    QByteArray fileChecksum(const QString &fileName, QCryptographicHash::Algorithm hashAlgorithm);
    QByteArray getOutByteArrayImage(const QString nameTable, const QString selectedColumn, const QString whereColumn, const int _id);
    QByteArray getOutByteArrayImageByDatabase(const QString nameTable, const QString selectedColumn, const QString whereColumn, const int _id, QSqlDatabase m_base);
    QString encode_string(const QString &str);
    QString decode_string(const QString &str);
    QString getVersionSQLite();
    QString getVersionMySQL();

    QString getHTMLImageInfo();
    QString getHTMLImageWarning();

    QString getStyleForButtonMessageBox();
    QString getStyleForToolButton();
    QString getStyleForButtonToolBar();

signals:
    void updateProgress(const int num_records, const int value);
    void finishedProgress(const QString txt);

private:
    QSqlDatabase db;
    QSqlDatabase db_image;
    QString m_connectionName = nullptr;
    quint32 key_encoding = 073; // encode_string & decode_string

    QProgressDialog *progress_dialog;

private:
    /* -- functiile interne cu baza de date -- */

    bool openDataBase();
    bool restoreDataDase();
    void closeDataBase();
    bool enableForeignKeys();
};

#endif // DATABASE_H
