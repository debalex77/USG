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
#include <QSqlQueryModel>

#if (QT_VERSION > QT_VERSION_CHECK(5, 15, 2))
#include <QRegularExpression>
#endif

#include <common/globals.h>
#include <data/loggingcategories.h>
#include <data/database_common.h>
#include <common/cryptomanager.h>

class DataBase : public QObject
{
    Q_OBJECT

public:
    explicit DataBase(QObject* parent = nullptr);
    ~DataBase();

    bool connectToDataBase();
    bool createConnectBaseSqlite(QString &txtMessage);
    bool createConnectBaseSqlite(const QString &databaseName,
                                 const QString &databasePath,
                                 bool initializeSchema,
                                 QString &txtMessage);

    QSqlDatabase getDatabase();
    QSqlDatabase getDatabaseThread(const QString threadConnectionName, const bool thisMySQL);
    QSqlDatabase getDatabaseCloudThread(const QString threadConnectionName);
    QSqlDatabase getDatabaseImageThread(const QString threadConnectionName);
    void removeDatabaseThread(const QString threadConnectionName);
    void removeDatabaseImageThread(const QString threadConnectionName);
    QSqlDatabase getDatabaseImage();

    // -------------------------------------------------------------
    //----------- functii universale

    int getNextNumberDoc(const QString &docName, int year, QString *err);

    /** Formatează o dată din baza de date în formatul afișabil „dd.MM.yyyy HH:mm:ss”.*/
    QString formatDatabaseDate(const QString &rawDate);

    /** Functie universala - inserează o înregistrare într-un tabel.
     * Creează automat interogarea `INSERT INTO`, folosind perechi cheie-valoare dintr-un QVariantMap. */
    bool insertIntoTable(const QString class_name,
                         const QString name_table,
                         const QVariantMap &values,
                         QStringList &err,
                         QVariant *insertedId = nullptr);

    /** Actualizează un rând într-un tabel, folosind perechi cheie-valoare și o condiție WHERE. */
    bool updateTable(const QString class_name,
                     const QString name_table,
                     const QVariantMap &values,
                     const QMap<QString, QVariant> &where_conditions,
                     QStringList &err);

    /** Selectează un singur rând dintr-un tabel, pe baza unor condiții WHERE.*/
    QVariantMap selectSingleRow(const QString class_name,
                                const QString name_table,
                                const QVariantMap &values,
                                const QMap<QString, QVariant> &where_conditions,
                                QStringList &err);

    /** Selectează datele prin unirea tabelelor 'constants' și 'userPreferences'.*/
    QVariantMap selectJoinConstantsUserPreferencesByUserId(const int id_user);

    /** Șterge rânduri dintr-un tabel pe baza unor condiții WHERE.*/
    bool deleteFromTable(const QString class_name,
                         const QString name_table,
                         const QMap<QString, QVariant> &where_conditions,
                         QStringList &err);

    /** Elimina rand dintr-un tabel, pe baza unor conditii */
    bool deleteDataFromTable(const QString name_table,
                             const QString deletionCondition = nullptr,
                             const QString valueCondition = nullptr);

    QString getTextSQL(const QString &resourcePath);

    bool execPreparedFromFile(QSqlDatabase database,
                              const QString &sqlPath,
                              const QVector<QVariant> &binds,
                              QString *err = nullptr);

    bool execPreparedFromFileReturnID(QSqlDatabase database,
                                      const QString &sqlPath,
                                      const QVector<QVariant> &binds,
                                      QVariant *lastInsertId,
                                      QString *err = nullptr);

    static void setModelQuery(QSqlQueryModel &model,
                              QSqlDatabase db,
                              QString sql,
                              const QVariantList& binds,
                              QVariantMap other = QVariantMap());

    bool deleteDocByID(const QString nameTable, const int id);

    // -------------------------------------------------------------
    //----------- alte functii

    bool creatingTables();
    bool creatingTables_DbImage();
    bool verifyNewDatabaseSchema() const;

    void loadInvestigationFromXml();
    bool updateInvestigationFromXML_2024();
    bool loadNormogramsFromXml();
    void insertDataForTabletypesPrices();
    void insertSetTableSettingsUsers();

    bool execQuery(const QString strQuery);
    bool removeObjectById(const QString nameTable, const int _id);
    int getLastIdForTable(const QString nameTable) const;
    int getLastIdForTableByDatabase(const QString nameTable, QSqlDatabase name_database) const;
    int getLastNumberDoc(const QString nameTable) const;
    int getLastIDPricings(const int id_organization, const int id_contract, const int id_typePrice) const;

    bool getDataFromQuery(const QString strQuery, QMap<QString, QString> &items);
    bool getDataFromQueryByRecord(const QString strQuery, QMap<QString, QString> &items) const;

    int statusDeletionMarkObject(const QString nameTable, const int _id) const;
    bool deletionMarkObject(const QString nameTable, const int _id);
    bool getObjectDataById(const QString &nameTable, const int _id, QMap<QString, QString> &items);

    bool postDocument(const QString nameTable, QMap<QString, QString> &items);

    bool createIndexTables();

    void updateVariableFromTableSettingsUser();

    bool existColumnInTable(const QString nameTable, const QString nameColumn) const;

    bool existIdDocument(const QString nameTable, const QString name_condition, const QString value_condition, QSqlDatabase nameDatabase);
    bool existSubalternDocument(const QString nameTable, const QString name_condition, const QString value_condition, int &id_doc);
    QString getQryFromTableConstantById(const int id_user) const;
    QString getQryForTableOrgansInternalById(const int id_doc) const;
    QString getQryForTableUrinarySystemById(const int id_doc) const;
    QString getQryForTableProstateById(const int id_doc) const;
    QString getQryForTableGynecologyById(const int id_doc) const;
    QString getQryForTableBreastById(const int id_doc) const;
    QString getQryForTableThyroidById(const int id_doc) const;
    QString getQryForTableGestation0dById(const int id_doc) const;
    QString getQryForTableGestation1dById(const int id_doc) const;
    QString getQryForTableGestation2(const int id_doc) const;
    QString getQryForTableLymphNodes(const int id_doc) const;

    QString getQryForTableOrderById(const int id_doc, const QString str_price) const;

    QByteArray fileChecksum(const QString &fileName, QCryptographicHash::Algorithm hashAlgorithm);

    static QString encode_string(const QString &str);
    static QString decode_string(const QString &str);
    QString getVersionSQLite();
    QString getVersionMySQL();

    QString getHTMLImageInfo();
    QString getHTMLImageWarning();

    QString getStyleForButtonMessageBox();
    QString toolButtonStyleForText();
    QString toolButtonStyleForIcon();

    QByteArray getHashUserApp();

    void ensureUUIDs();
    void ensureIndexUUIDs();

signals:
    void uuidProgress(int value, int maximum, const QString &message);
    void updateProgress(const int num_records, const int value);
    void finishedProgress(const QString txt);

private:
    CryptoManager *crypto_manager;
    QSqlDatabase db;
    QSqlDatabase db_image;
    DataBaseCommon db_common;
    QString m_connectionName = nullptr;
    QProgressDialog *progress_dialog;

private:
    /* -- functiile interne cu baza de date -- */

    bool openDataBase();
    bool restoreDataDase();
    void closeDataBase();
    bool enableForeignKeys();
};

#endif // DATABASE_H
