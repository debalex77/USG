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

#include <common/cryptomanager.h>

class DataBase : public QObject
{
    Q_OBJECT

public:
    explicit DataBase(QObject* parent = nullptr);
    ~DataBase();

    bool connectToDataBase();   
    bool createConnectBaseSqlite(QString &txtMessage);

    QSqlDatabase getDatabase();
    QSqlDatabase getDatabaseThread(const QString threadConnectionName, const bool thisMySQL);
    QSqlDatabase getDatabaseCloudThread(const QString threadConnectionName);
    QSqlDatabase getDatabaseImageThread(const QString threadConnectionName);
    void removeDatabaseThread(const QString threadConnectionName);
    void removeDatabaseImageThread(const QString threadConnectionName);
    QSqlDatabase getDatabaseImage();

    // -------------------------------------------------------------
    //----------- functii universale

    /*!
     * \brief Formatează o dată din baza de date în formatul afișabil „dd.MM.yyyy HH:mm:ss”.
     *
     * În funcție de tipul bazei de date (MySQL sau SQLite), aplică formatul corespunzător
     * pentru parsarea valorii brute a datei din baza de date și o convertește într-un
     * format uman-lizibil.
     *
     * \param rawDate Data brută sub formă de string, obținută din câmpul SQL (ex: "2024-05-28T14:33:00").
     * \return Un string cu data convertită în format „dd.MM.yyyy HH:mm:ss” dacă parsarea reușește,
     * altfel returnează valoarea originală \c rawDate.
     *
     * \note Formatul de intrare este determinat din contextul aplicației prin \c globals().thisMySQL:
     *  - dacă este \c true, se așteaptă formatul „yyyy-MM-ddTHH:mm:ss” (MySQL)
     *  - dacă este \c false, se așteaptă formatul „yyyy-MM-dd HH:mm:ss” (SQLite)
     *
     * \see QDateTime::fromString, QDateTime::toString
     */
    QString formatDatabaseDate(const QString &rawDate);

    /*!
     * \brief Functie universala - inserează o înregistrare într-un tabel.
     * Creează automat interogarea `INSERT INTO`, folosind perechi cheie-valoare dintr-un QVariantMap.
     *
     * \param class_name - numele clasei apelante, folosit pentru loguri/erori.
     * \param name_table - numele tabelului în care se face inserarea.
     * \param values - harta cheie-valoare (coloană => valoare) pentru inserare.
     * \param err - listă în care sunt stocate mesajele de eroare, dacă apar.
     * \param insertedId - pointer opțional către un QVariant în care se returnează ID-ul inserat (dacă există).
     *
     * \return true dacă inserarea a avut succes, altfel false.
     *
     * \note Exemplu (in caz cand este necesar ID inserat):
     * db.insertIntoTable("UserPreferences", "users", valori, err, &id) - cu returnarea ID inserat
     * db.insertIntoTable("UserPreferences", "users", valori, err) - fara ID
     */
    bool insertIntoTable(const QString class_name,
                         const QString name_table,
                         const QVariantMap &values,
                         QStringList &err,
                         QVariant *insertedId = nullptr);

    /*!
     * \brief Actualizează un rând într-un tabel, folosind perechi cheie-valoare și o condiție WHERE.
     *
     * \param class_name - numele clasei apelante, folosit pentru erori/loguri.
     * \param name_table - numele tabelului în care se face actualizarea.
     * \param values - chei și valori pentru coloanele ce trebuie actualizate.
     * \param where_conditions - WHERE cu mai multe conditii.
     * \param err - referință la o listă în care se vor adăuga mesajele de eroare (dacă apar).
     * \return true dacă actualizarea a avut succes, altfel false.
     */
    bool updateTable(const QString class_name,
                     const QString name_table,
                     const QVariantMap &values,
                     const QMap<QString, QVariant> &where_conditions,
                     QStringList &err);

    /*!
     * \brief Selectează un singur rând dintr-un tabel, pe baza unor condiții WHERE.
     *
     * Creează și execută o interogare de tip SELECT cu coloanele specificate și condiții WHERE,
     * returnând un singur rând sub forma unui \c QVariantMap. Dacă nu există niciun rezultat,
     * se returnează un map gol.
     *
     * \param class_name Numele clasei apelante, utilizat pentru loguri și mesaje de eroare.
     * \param name_table Numele tabelului din care se face selecția.
     * \param values Chei ce reprezintă coloanele dorite la SELECT. Valorile pot fi ignorate (doar cheia contează).
     * \param where_conditions Condițiile pentru filtrarea rezultatelor (coloană => valoare).
     * \param err Referință către o listă în care vor fi adăugate mesajele de eroare, dacă apar.
     * \return Un \c QVariantMap ce conține valorile coloanelor din primul rând găsit.
     * Dacă nu există rezultate, se returnează un map gol.
     *
     * \note Interogarea adaugă automat \c LIMIT 1 pentru a preveni returnarea multiplă.
     */
    QVariantMap selectSingleRow(const QString class_name,
                                const QString name_table,
                                const QVariantMap &values,
                                const QMap<QString, QVariant> &where_conditions,
                                QStringList &err);

    /*!
     * \brief Selectează datele prin unirea tabelelor 'constants' și 'userPreferences'.
     * Realizează un JOIN între tabelele `constants` și `userPreferences` pe baza `id_users`,
     * filtrând după ID-ul utilizatorului.
     *
     * \param id_user - ID-ul utilizatorului pentru care se selectează datele.
     * \return result - un QVariantMap ce conține datele selectate (sau gol dacă nu există rezultate).
     *
     * \note Coloanele cu același nume din ambele tabele (ex: `id_users`) vor fi suprascrise în rezultat.
     */
    QVariantMap selectJoinConstantsUserPreferencesByUserId(const int id_user);

    /*!
     * \brief Șterge rânduri dintr-un tabel pe baza unor condiții WHERE.
     *
     * Creează și execută o interogare de tip DELETE, utilizând condițiile specificate
     * în parametrul \c where_conditions. Pentru siguranță, dacă \c where_conditions este gol,
     * operația este blocată.
     *
     * \param class_name Numele clasei apelante, utilizat pentru loguri și erori.
     * \param name_table Numele tabelului din care se vor șterge rânduri.
     * \param where_conditions Condițiile WHERE (chei = coloane, valori = valori căutate).
     *                        Poate conține una sau mai multe condiții.
     * \param err Referință la o listă de mesaje de eroare, care va fi completată în caz de eșec.
     * \return true dacă operația DELETE a fost executată cu succes, false în caz contrar.
     *
     * \note Pentru siguranță, funcția nu execută DELETE fără condiții WHERE.
     */
    bool deleteFromTable(const QString class_name,
                         const QString name_table,
                         const QMap<QString, QVariant> &where_conditions,
                         QStringList &err);

    /*!
     * \brief Elimina rand dintr-un tabel, pe baza unor conditii
     * \param name_table - numele tabelei
     * \param deletionCondition - sectia conditie (coloana => valoarea)
     * \param valueCondition - valoarea conditiei
     * \return true daca operatia e reusita
     */
    bool deleteDataFromTable(const QString name_table, const QString deletionCondition = nullptr, const QString valueCondition = nullptr);

    // -------------------------------------------------------------
    //----------- alte functii

    void creatingTables();
    bool checkTable(const QString name_table);
    void creatingTables_DbImage();

    void loadInvestigationFromXml();
    void updateInvestigationFromXML_2024();
    void loadNormogramsFromXml();
    void insertDataForTabletypesPrices();
    void insertSetTableSettingsUsers();

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
    bool getObjectDataById(const QString &nameTable, const int _id, QMap<QString, QString> &items);

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
    bool createTableSettingsUsers();
    bool createTableUserPreferences();
    bool createTableConclusionTemplates();
    bool createTableFormationsSystemTemplates();
    bool createTableImagesReports();

    bool createIndexTables();
    bool createTableContOnline();
    bool createTableCloudServer();

    enum REQUIRED_NUMBER
    {
        _NEGATIV        = -1,
        DELETION_MARK   = 1,
        DELETION_UNMARK = 0
    };

    void updateVariableFromTableSettingsUser();

    void getNameColumnTable(const QString nameTable/*,QMap<QString, QString> &items*/);
    int getCountSectionTableSQLite(const QString nameTable) const;
    bool existColumnInTable(const QString nameTable, const QString nameColumn) const;
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

    QByteArray getHashUserApp();

signals:
    void updateProgress(const int num_records, const int value);
    void finishedProgress(const QString txt);

private:
    CryptoManager *crypto_manager;
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
