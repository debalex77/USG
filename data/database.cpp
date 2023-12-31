﻿/********************************************************************
 **
 ** \304\203  - ă
 ** \303\242  - â
 ** \310\231  - ș
 ** \310\233  = ț
 **           = î
 **
 ********************************************************************/

#include "database.h"
#include <QDomDocument>

DataBase::DataBase(QObject *parent) : QObject(parent)
{
}

DataBase::~DataBase()
{
}

// *******************************************************************
// ******************* CONCTAREA LA BAZA DE DATE *********************

bool DataBase::connectToDataBase()
{
    if (globals::connectionMade == "MySQL"){
        return openDataBase();
    } else {
        if (QFile(globals::sqlitePathBase).exists()) {
            return openDataBase();
        } else {
            return restoreDataDase();
        }
    }
    return false;
}

bool DataBase::createConnectBaseSqlite(QString &txtMessage)
{
    //verificam daca este creata BD
    QSqlDatabase _newBase = QSqlDatabase::database();
    if (_newBase.open()){
        txtMessage = tr("Baza de date <b>\"%1\"</b> deja este creata !!! <br>%2").arg(globals::sqliteNameBase, globals::sqlitePathBase);
        return true;
    }

    //crearea BD
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setHostName(globals::sqliteNameBase);
    db.setDatabaseName(globals::sqlitePathBase);
    if(db.open()){
        if (globals::firstLaunch){
            creatingTables();
        }
        txtMessage = tr("Baza de date \"<b>%1</b>\" este creata cu succes.").arg(globals::sqliteNameBase);
        // !!! de adaugat - crearea/inițierea tabelelor in baza sqlite
        return true;
    } else {
        txtMessage = tr("Baza de date \"<b>%1</b>\" nu este creata. <br>"
                        "Verificati corectitudinea localizarii fisierului cu baza de date sau adresati-va administratorului aplicatiei.")
                .arg(globals::sqliteNameBase);
        return false;
    }
    return false;
}

QSqlDatabase DataBase::getDatabase()
{
    return db.database();
}

QSqlDatabase DataBase::getDatabaseImage()
{
    return db_image.database("db_image");
}

// *******************************************************************
// *************** CREAREA TABELELOR, OBIECTELOR *********************

void DataBase::creatingTables()
{
    if (createTableUsers())
        qInfo(logInfo()) << tr("A fost creata tabela 'users'.");

    if (createTableDoctors())
        qInfo(logInfo()) << tr("A fost creata tabela 'doctors'.");

    if (createTableFullNameDoctors())
        qInfo(logInfo()) << tr("A fost creata tabela 'fullNameDoctors'.");

    if (createTableNurses())
        qInfo(logInfo()) << tr("A fost creata tabela 'nurses'.");

    if (createTableFullNameNurses())
        qInfo(logInfo()) << tr("A fost creata tabela 'fullNameNurses'.");

    if (createTablePacients())
        qInfo(logInfo()) << tr("A fost creata tabela 'pacients'.");

    if (createTableFullNamePacients())
        qInfo(logInfo()) << tr("A fost creata tabela 'fullNamePacients'.");

    if (createTableTypesPrices())
        qInfo(logInfo()) << tr("A fost creata tabela 'typesPrices'.");

    if (createTableOrganizations())
        qInfo(logInfo()) << tr("A fost creata tabela 'organizations'.");

    if (createTableInvestigations())
        qInfo(logInfo()) << tr("A fost creata tabela 'investigations'.");

    if (createTableConstants())
        qInfo(logInfo()) << tr("A fost creata tabela 'constants'.");

    if (createTableContracts())
        qInfo(logInfo()) << tr("A fost creata tabela 'contracts'.");

    if (createTablePricings())
        qInfo(logInfo()) << tr("A fost creata tabela 'pricings'.");

    if (createTable_PricingsTable())
        qInfo(logInfo()) << tr("A fost creata tabela 'pricingsTable'.");

    if (createTablePricingsPresentation())
        qInfo(logInfo()) << tr("A fost creata tabela 'pricingsPresentation'.");

    if (createTableOrderEcho())
        qInfo(logInfo()) << tr("A fost creata tabela 'orderEcho'.");

    if (createTable_OrderEchoTable())
        qInfo(logInfo()) << tr("A fost creata tabela 'orderEchoTable'.");

    if (createTableOrderEchoPresentation())
        qInfo(logInfo()) << tr("A fost creata tabela 'orderEchoPresentation'.");

    if (createTableReportEcho())
        qInfo(logInfo()) << tr("A fost creata tabela 'reportEcho'.");

    if (createTableReportEchoPresentation())
        qInfo(logInfo()) << tr("A fost creata tabela 'reportEchoPresentation'.");

    if (createTableLiver())
        qInfo(logInfo()) << tr("A fost creata tabela 'tableLiver'.");

    if (createTableCholecist())
        qInfo(logInfo()) << tr("A fost creata tabela 'tableCholecist'.");

    if (createTablePancreas())
        qInfo(logInfo()) << tr("A fost creata tabela 'tablePancreas'.");

    if (createTableSpleen())
        qInfo(logInfo()) << tr("A fost creata tabela 'tableSpleen'.");

    if (createTableKidney())
        qInfo(logInfo()) << tr("A fost creata tabela 'tableKidney'.");

    if (createTableBladder())
        qInfo(logInfo()) << tr("A fost creata tabela 'tableBladder'.");

    if (createTableProstate())
        qInfo(logInfo()) << tr("A fost creata tabela 'tableProstate'.");

    if (createTableGynecology())
        qInfo(logInfo()) << tr("A fost creata tabela 'tableGynecology'.");

    if (createTableBreast())
        qInfo(logInfo()) << tr("A fost creata tabela 'tableBreast'.");

    if (createTableThyroid())
        qInfo(logInfo()) << tr("A fost creata tabela 'tableThyroid'.");

    if (createTableGestation0())
        qInfo(logInfo()) << tr("A fost creata tabela 'tableGestation0'.");

    if (createTableGestation1())
        qInfo(logInfo()) << tr("A fost creata tabela 'tableGestation1'.");

    if (createTableRegistrationPatients())
        qInfo(logInfo()) << tr("A fost creata tabela 'registrationPatients'.");

    if (createTableSettingsForm())
        qInfo(logInfo()) << tr("A fost creata tabela 'settingsForm'.");

    if (createTableSettingsReports())
        qInfo(logInfo()) << tr("A fost creata tabela 'settingsReports'.");

    if (createTableUserPreferences())
        qInfo(logInfo()) << tr("A fost creata tabela 'userPreferences'.");

    if (createTableConclusionTemplates())
        qInfo(logInfo()) << tr("A fost creata tabela 'conclusionTemplates'.");

    if (createTableImagesReports())
        qInfo(logInfo()) << tr("A fost creata tabela 'imagesReports'.");
}

bool DataBase::checkTable(const QString name_table)
{
    QSqlQuery qry;
    qry.prepare("SELECT name FROM sqlite_master WHERE TYPE='table' AND name='" + name_table + "';");
    if (qry.exec() && qry.next())
        return (qry.value(0).toString().isEmpty()) ? false : true;
    return false;
}

void DataBase::creatingTables_DbImage()
{
    QSqlQuery qry(db_image);
    qry.prepare("CREATE TABLE imagesReports ("
                "id            INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                "id_reportEcho INT     NOT NULL,"
                "id_orderEcho  INT     NOT NULL,"
                "id_patients   INT     NOT NULL,"
                "image_1       BLOB,"
                "image_2       BLOB,"
                "image_3       BLOB,"
                "image_4       BLOB,"
                "image_5       BLOB,"
                "comment_1     TEXT,"
                "comment_2     TEXT,"
                "comment_3     TEXT,"
                "comment_4     TEXT,"
                "comment_5     TEXT,"
                "id_user       INT);");
    if (qry.exec()){
        qInfo(logInfo()) << tr("Creata tabela 'imagesReports' in baza de date 'DB_IMAGE'.");
    } else {
        qCritical(logCritical()) << tr("Eroare la crearea tabelei 'imagesReports' in baza de date 'DB_IMAGE'.") + qry.lastError().text();
    }
}

void DataBase::loadInvestigationFromXml()
{
    QDomDocument investigXML;
    QFile xmlFile(":/xmls/investig.xml");
    if (!xmlFile.open(QIODevice::ReadOnly )){
        qWarning(logWarning()) << tr("Fisierul ':/xmls/investig.xml' nu a fost citit !!!");
    }
    investigXML.setContent(&xmlFile);
    xmlFile.close();

    QDomElement root = investigXML.documentElement();
    QDomElement node = root.firstChild().toElement();

    while(node.isNull() == false) {
        if(node.tagName() == "entry"){
            while(!node.isNull()){
                QString cod  = node.attribute("cod", "cod");
                QString name = node.attribute("name", "name");

                QSqlQuery qry;
                qry.prepare("INSERT INTO investigations (id,deletionMark,cod,name,`use`) VALUES ( ?, ?, ?, ?, ?);");
                qry.addBindValue(getLastIdForTable("investigations") + 1);
                qry.addBindValue(0);
                qry.addBindValue(cod);
                qry.addBindValue(name);
                qry.addBindValue(1);
                if (qry.exec())
                    qInfo(logInfo()) << tr("Investigatia '%1' este introdusa in baza de date cu codul '%2'.").arg(name, cod);
                else
                    qWarning(logWarning()) << tr("Eroare la inserare a datelor in tabela 'investigations': %1").arg(qry.lastError().text());
                node = node.nextSibling().toElement();
            }
        }
        node = node.nextSibling().toElement();
    }
}

// *******************************************************************
// ********* INSERAREA, ACTUALIZAREA SETARILOR DIN TABELE ************

void DataBase::insertDataForTabletypesPrices()
{
    QSqlQuery qry;
    int m_id = getLastIdForTable("typesPrices") + 1;
    qry.prepare("INSERT INTO typesPrices (id, deletionMark, name, discount, noncomercial) VALUES ( ?, ?, ?, ?, ?);");
    qry.addBindValue(m_id);
    qry.addBindValue(0);
#if defined(Q_OS_LINUX)
    qry.addBindValue(tr("Prețuri comerciale"));
#elif defined(Q_OS_MACOS)
    qry.addBindValue(tr("Prețuri comerciale"));
#elif defined(Q_OS_WIN)
    qry.addBindValue(tr("Pre\310\233uri comerciale"));
#endif
    qry.addBindValue(0);
    if (globals::thisMySQL)
        qry.addBindValue(false);
    else
        qry.addBindValue(0);
    if (qry.exec())
        qInfo(logInfo()) << tr("In baza de date este introdus tipul pretului 'Preturi comerciale'.");
    else
        qWarning(logWarning()) << tr("Eroare la inserare a datelor in tabela 'typesPrices': %1").arg(qry.lastError().text());

    qry.clear();
    qry.prepare("INSERT INTO typesPrices (id, deletionMark, name, discount, noncomercial) VALUES ( ?, ?, ?, ?, ?);");
    qry.addBindValue(m_id + 1);
    qry.addBindValue(0);
#if defined(Q_OS_LINUX)
    qry.addBindValue(tr("Prețuri CNAM"));
#elif defined(Q_OS_MACOS)
    qry.addBindValue(tr("Prețuri CNAM"));
#elif defined(Q_OS_WIN)
    qry.addBindValue(tr("Pre\310\233uri CNAM"));
#endif
    qry.addBindValue(0);
    if (globals::thisMySQL)
        qry.addBindValue(true);
    else
        qry.addBindValue(1);
    if (qry.exec())
        qInfo(logInfo()) << tr("In baza de date este introdus tipul pretului 'Preturi CNAM'.");
    else
        qWarning(logWarning()) << tr("Eroare la inserare a datelor in tabela 'typesPrices': %1").arg(qry.lastError().text());
}

void DataBase::insertSetTableSettingsUsers()
{
    QSqlQuery qry;
    qry.prepare("INSERT INTO userPreferences ("
                "id,"
                "id_users,"
                "versionApp,"
                "showQuestionCloseApp,"
                "showUserManual,"
                "showHistoryVersion,"
                "order_splitFullName,"
                "updateListDoc,"
                "showDesignerMenuPrint,"
                "checkNewVersionApp,"
                "databasesArchiving,"
                "showAsistantHelper) VALUES (?,?,?,?,?,?,?,?,?,?,?,?);");
    qry.addBindValue(globals::idUserApp);
    qry.addBindValue(globals::idUserApp);
    qry.addBindValue("");
    qry.addBindValue(1);
    qry.addBindValue(0);
    qry.addBindValue(0);
    qry.addBindValue(0);
    qry.addBindValue(10);
    qry.addBindValue(0);
    qry.addBindValue(1);
    qry.addBindValue(0);
    qry.addBindValue(1);

    if (! qry.exec())
        qCritical(logCritical()) << tr("Nu au fost inserate date initiale in tabela 'userPreferences' %1.")
                                        .arg((qry.lastError().text().isEmpty()) ? "" : "- " + qry.lastError().text());
}

int DataBase::findIdFromTableSettingsForm(const QString typeForm, const int numberSection) const
{
    QString str = QString("SELECT id FROM settingsForms WHERE "
                          "typeForm      = :typeForm AND "
                          "numberSection = :numberSection AND "
                          "id_users      = :id_users;");
    QSqlQuery qry;
    qry.prepare(str);
    qry.bindValue(":typeForm",      typeForm);
    qry.bindValue(":numberSection", numberSection);
    qry.bindValue(":id_users",      globals::idUserApp);
    if (! qry.exec()){
        qWarning(logWarning()) << tr("Eroare de executare a solicitarii de determinare 'id' din tabela 'settingsForms': %1").arg(qry.lastError().text());
        return -1;
    } else {
        if (qry.next() && qry.value(0).toInt() > 0)
            return qry.value(0).toInt();
        else
            return -1;
    }
}

bool DataBase::insertUpdateDataTableSettingsForm(const bool insertData,  const QString typeForm, const int numberSection, const int sectionSize, int id_data, int directionSorting) const
{
    if (insertData){
        QString str = QString("INSERT INTO settingsForms (id, id_users, typeForm, numberSection, sizeSection, directionSorting) "
                              "VALUES (:id, :id_users, :typeForm, :numberSection, :sizeSection, :directionSorting);");
        QSqlQuery qry;
        qry.prepare(str);
        qry.bindValue(":id",               getLastIdForTable("settingsForms") + 1);
        qry.bindValue(":id_users",         globals::idUserApp);
        qry.bindValue(":typeForm",         typeForm);
        qry.bindValue(":numberSection",    numberSection);
        qry.bindValue(":sizeSection",      sectionSize);
        qry.bindValue(":directionSorting", (directionSorting == -1) ? QVariant() : directionSorting);
        if (! qry.exec()){
            qWarning(logWarning()) << tr("Eroare de executare a solicitarii de inserare a datelor in tabela 'settingsForms': %1").arg(qry.lastError().text());
            return false;
        }
    } else {
        QString str = QString("UPDATE settingsForms SET "
                              " typeForm         = :typeForm, "
                              " numberSection    = :numberSection, "
                              " sizeSection      = :sizeSection,"
                              " directionSorting = :directionSorting "
                              "WHERE "
                              " id = :id AND id_users = :id_users;");
        QSqlQuery qry;
        qry.prepare(str);
        qry.bindValue(":id",               id_data);
        qry.bindValue(":id_users",         globals::idUserApp);
        qry.bindValue(":typeForm",         typeForm);
        qry.bindValue(":numberSection",    numberSection);
        qry.bindValue(":sizeSection",      sectionSize);
        qry.bindValue(":directionSorting", (directionSorting == -1) ? QVariant() : directionSorting);
        if (! qry.exec()){
            qWarning(logWarning()) << tr("Eroare de executare a solicitarii de actualizare a datelor in tabela 'settingsForms': %1").arg(qry.lastError().text());
            return false;
        }
    }

    return true;
}

int DataBase::getSizeSectionFromTableSettingsForm(const int numberSection, const QString typeForm, const int id_data) const
{
    QString str = QString("SELECT sizeSection FROM settingsForms WHERE "
                          "typeForm      = :typeForm AND "
                          "numberSection = :numberSection AND "
                          "id            = :id AND "
                          "id_users      = :id_users;");
    QSqlQuery qry;
    qry.prepare(str);
    qry.bindValue(":id",            id_data);
    qry.bindValue(":id_users",      globals::idUserApp);
    qry.bindValue(":typeForm",      typeForm);
    qry.bindValue(":numberSection", numberSection);
    if (qry.exec() && qry.next() && qry.value(0).toInt() > 0)
        return qry.value(0).toInt();
    else
        return 0;
}

int DataBase::getDirectionSortingFromTableSettingsForm(const int numberSection, const QString typeForm) const
{
    QString str;
    if (globals::thisMySQL)
        str = QString("SELECT directionSorting FROM settingsForms WHERE "
                      "directionSorting IS NOT Null AND "
                      "typeForm      = :typeForm AND "
                      "numberSection = :numberSection AND "
                      "id_users      = :id_users;");
    else
        str = QString("SELECT directionSorting FROM settingsForms WHERE "
                      "directionSorting NOT NULL AND "
                      "typeForm      = :typeForm AND "
                      "numberSection = :numberSection AND "
                      "id_users      = :id_users;");
    QSqlQuery qry;
    qry.prepare(str);
    qry.bindValue(":id_users",      globals::idUserApp);
    qry.bindValue(":typeForm",      typeForm);
    qry.bindValue(":numberSection", numberSection);
    if (qry.exec() && qry.next() && qry.value(0).toInt() >= 0)
        return qry.value(0).toInt();
    else
        return -1;
}

bool DataBase::updatePeriodInTableSettingsForm(const int id_data, const QString date_start, const QString date_end)
{
    QString str = QString("UPDATE settingsForms SET "
                          " dateStart  = :dateStart, "
                          " dateEnd    = :dateEnd "
                          "WHERE "
                          " id = :id AND id_users = :id_users;");
    QSqlQuery qry;
    qry.prepare(str);
    qry.bindValue(":id",        id_data);
    qry.bindValue(":id_users",  globals::idUserApp);
    qry.bindValue(":dateStart", date_start);
    qry.bindValue(":dateEnd",   date_end);
    if (! qry.exec()){
        qWarning(logWarning()) << tr("Eroare de executare a solicitarii de actualizare a perioadei din tabela 'settingsForms': %1").arg(qry.lastError().text());
        return false;
    }
    return true;
}

void DataBase::getPeiodFromTableSettingsForm(const int id_data, const QString typeForm, QString &date_start, QString &date_end)
{
    QString str;
    if (globals::thisMySQL) // globals::mySQLhost
        str = QString("SELECT dateStart, dateEnd FROM settingsForms WHERE "
                      "dateStart       IS NOT Null AND "
                      "dateEnd         IS NOT Null AND "
                      "typeForm      = :typeForm AND "
                      "numberSection = 0 AND "
                      "id_users      = :id_users AND "
                      "id            = :id;");
    else
        str = QString("SELECT dateStart, dateEnd FROM settingsForms WHERE "
                      "dateStart     NOT NULL AND "
                      "dateEnd       NOT NULL AND "
                      "typeForm      = :typeForm AND "
                      "numberSection = 0 AND "
                      "id_users      = :id_users AND "
                      "id            = :id;");
    QSqlQuery qry;
    qry.prepare(str);
    qry.bindValue(":id",       QString::number(id_data));
    qry.bindValue(":id_users", QString::number(globals::idUserApp));
    qry.bindValue(":typeForm", typeForm);
    if (qry.exec() && qry.next()){
        if (globals::thisMySQL){
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            date_start = qry.value(0).toString().replace(QRegExp("T"), " ").replace(".000","");
            date_end   = qry.value(1).toString().replace(QRegExp("T"), " ").replace(".000","");
#else
            date_start = qry.value(0).toString().replace(QRegularExpression("T"), " ").replace(".000","");
            date_end   = qry.value(1).toString().replace(QRegularExpression("T"), " ").replace(".000","");
#endif
        }else {
            date_start = qry.value(0).toString();
            date_end   = qry.value(1).toString();
        }
    } else {
        qWarning(logWarning()) << tr("Eroare de executare a solicitarii determinarii perioadei din tabela 'settingsForms': %1").arg(qry.lastError().text());
    }
}

// *******************************************************************
// *************** CREAREA TABELELOR, OBIECTELOR *********************

bool DataBase::existNameObject(const QString nameTable, const QString nameObject)
{
    QSqlQuery qry;
    qry.prepare("SELECT count(name) "
                "FROM " + nameTable + " "
                                      "WHERE name = '" + nameObject + "' "
                                                                      "AND deletionMark = 0;");
    if (qry.exec()){
        qry.next();
        int n = qry.value(0).toInt();
        bool value = (n > 0) ? true:false;
        return value;
    } else {
        qWarning(logWarning()) << tr("%1 - existNameObject()").arg(metaObject()->className())
                               << tr("Solicitarea nereusita: %1").arg(qry.lastError().text());
        return false;
    }
}

bool DataBase::existNameObjectReturnId(const QString nameTable, const QString nameObject, int &_id)
{
    QSqlQuery qry;
    qry.prepare("SELECT id "
                "FROM " + nameTable + " "
                                      "WHERE name = '" + nameObject + "' "
                                                                      "AND deletionMark = 0;");
    if (qry.exec()){
        qry.next();
        _id = qry.value(0).toInt();
        return (_id > 0) ? true:false;
    } else {
        qWarning(logWarning()) << tr("%1 - existNameObjectReturnId()").arg(metaObject()->className())
                               << tr("Solicitarea nereusita: %1").arg(qry.lastError().text());
        return false;
    }
}

bool DataBase::createNewObject(const QString nameTable, QMap<QString, QString> &items)
{
    QString _key;        // nume colonitelor
    QString _value;      // valoarea
    QMapIterator<QString, QString> it(items);
    while (it.hasNext()) {   // determinam _key and _value
        it.next();
        _key = _key + "'" + it.key() + "',";         // ex: 'name', 'comment',
        _value = _value + "'" + it.value() + "',";
    }
    _key.resize(_key.size() - 1);      // eliminam ultimul simbol (,):
    _value.resize(_value.size() - 1);  // ex: 'name', 'comment'

    QSqlQuery qry;  // determinam id
    qry.prepare("SELECT max(id) FROM " + nameTable + ";");
    qry.exec();
    qry.next();
    int _id = qry.value(0).toInt() + 1;

    // cream obiect nou
    qry.prepare("INSERT INTO " + nameTable +
                "(id, deletionMark, " + _key + ")"
                                               " VALUES(" + QString::number(_id) + ","
                                                                                   "0,"
                                                                                   "" + _value + ");");
    if (qry.exec()){
        items.insert("id", QString::number(_id)); // adaugam p-u extragerea din clasa parinte
        return true;
    } else {
        qWarning(logWarning()) << tr("%1: createNewObject(nameTable = %2): "
                                     "<br>Crearea obiectului a esuat.")
                                  .arg(metaObject()->className(),
                                       nameTable);
        return false;
    }
}

bool DataBase::updateDataObject(const QString nameTable, const int _id, QMap<QString, QString> &items)
{
    QString keyValue;    // key and value
    QMapIterator<QString, QString> it(items);
    while (it.hasNext()) {
        it.next();
        if (it.value().isEmpty())
            continue;
        else
           keyValue = keyValue + it.key() + " = '" + it.value() + "',";
    }
    keyValue.resize(keyValue.size() - 1);  // eliminam ultimul simbol (,):

    QSqlQuery qry;
    qry.prepare("UPDATE " + nameTable +
                " SET " + keyValue +
                "WHERE id = " + QString::number(_id) + ";");
    if (qry.exec()){
        return true;
    } else {
        QString _nameObject = items.constFind("name").value();
        qWarning(logWarning()) << tr("%1: updateDataObject(nameTable = %2, _id = %3): "
                                     "<br>Modificarea datelor obiectului <<-%4->> a esuat.")
                                  .arg(metaObject()->className(),
                                       nameTable,
                                       QString::number(_id),
                                       (_nameObject.isEmpty())? "unknown" : _nameObject) + "<br>" + qry.lastError().text();
        return false;
    }
}

// *******************************************************************
// ****************** FUNCTIILE CU DOCUMENTE *************************

bool DataBase::execQuery(const QString strQuery)
{
    QSqlQuery qry;

    qry.prepare(strQuery);
    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1: Executarea solicitarii nereusita !!!").arg(metaObject()->className())
                               << qry.lastError().text();
        return false;
    }
}

bool DataBase::removeObjectById(const QString nameTable, const int _id)
{
    QSqlQuery qry;

    qry.prepare(QString("DELETE FROM %1 WHERE id = '%1';").arg(nameTable, QString::number(_id)));
    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1: Executarea eliminarii obiectului nereusita !!!").arg(metaObject()->className())
                               << qry.lastError().text();
        return false;
    }
}

int DataBase::getLastIdForTable(const QString nameTable) const
{
    int lastId = 0;
    QString strQryLastNrDoc;
    strQryLastNrDoc = QString("SELECT id FROM %1 ORDER BY id DESC LIMIT 1;").arg(nameTable);

    QMap<QString, QString> _items;
    if (getDataFromQueryByRecord(strQryLastNrDoc, _items)){
        if (_items.count() != 0){
            lastId = _items.constFind("id").value().toInt();
        }
    }
    return lastId;
}

int DataBase::getLastIdForTableByDatabase(const QString nameTable, QSqlDatabase name_database) const
{
    int last_id = 0;
    QSqlQuery qry(name_database);
    qry.prepare(QString("SELECT id FROM " + nameTable + " ORDER BY id DESC LIMIT 1;"));
    if (qry.exec()){
        qry.first();
        last_id = qry.value(0).toInt();
    } else {
        qWarning(logWarning()) << tr("Eroare de executare a solicitarii 'getLastIdForTableByDatabase()': %1").arg(qry.lastError().text());
    }
    return last_id;
}

int DataBase::getLastNumberDoc(const QString nameTable) const
{
    QString lastNumberDoc;
    QString strQry = QString("SELECT numberDoc FROM %1 ORDER BY id DESC LIMIT 1;").arg(nameTable);
    QMap<QString, QString> _items;
    if (getDataFromQueryByRecord(strQry, _items)){
        if (_items.count() != 0){
            lastNumberDoc = _items.constFind("numberDoc").value();
        }
    }
    if (lastNumberDoc.isEmpty())
        return 0;
    else
        return lastNumberDoc.toInt();
}

bool DataBase::getObjectDataByMainId(const QString nameTable, const QString nameId, const int _id, QMap<QString, QString> &items)
{
    QSqlQuery qry;
    qry.exec("SELECT * FROM " + nameTable + " WHERE " + nameId + " = " + QString::number(_id) + ";");
    qry.next();
    QSqlRecord rec = qry.record();
    for (int n = 0; n < rec.count(); n++) {
        items.insert(rec.fieldName(n), rec.value(n).toString());
    }
    return true;
}

bool DataBase::getDataFromQuery(const QString strQuery, QMap<QString, QString> &items)
{
    QSqlQuery qry;
    qry.exec(strQuery);
    while (qry.next()){
        items.insert(qry.value(0).toString(),qry.value(1).toString());
    }
    return true;
}

bool DataBase::getDataFromQueryByRecord(const QString strQuery, QMap<QString, QString> &items) const
{
    QSqlQuery qry;
    qry.exec(strQuery);
    QSqlRecord rec = qry.record();

    while (qry.next()){
        for (int n = 0; n < rec.count(); n++) {
            items.insert(rec.fieldName(n), qry.value(n).toString());
        }
    }
    return true;
}

int DataBase::statusDeletionMarkObject(const QString nameTable, const int _id) const
{
    QSqlQuery qry;
    qry.prepare("SELECT deletionMark  FROM " + nameTable + "  WHERE id = :id;");
    qry.bindValue(":id", _id);
    if (qry.exec() && qry.next()){
        return qry.value(0).toInt();
    } else {
        return _NEGATIV;
    }
}

bool DataBase::deletionMarkObject(const QString nameTable, const int _id)
{
    int statusDeletion;

    // determinam statut curent
    int currentStatus = statusDeletionMarkObject(nameTable, _id);
    if (currentStatus != _NEGATIV){
        if (currentStatus == DELETION_UNMARK){
            statusDeletion = DELETION_MARK;
        } else {
            statusDeletion = DELETION_UNMARK;
        }
    } else {
        qWarning(logWarning()) << tr("%1 - deletionMarkObject(nameTable = %2, id = %3)").arg(metaObject()->className(), nameTable, QString::number(_id))
                               << tr("Status 'deletionMark' = -1 : nu poate fi negativ !!!");
        return false;
    }
    // marcam elementul
    QSqlQuery qry;
    qry.prepare("UPDATE " + nameTable + " SET deletionMark = :deletionMark WHERE id = :id;");
    qry.bindValue(":deletionMark", statusDeletion);
    qry.bindValue(":id", _id);
    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - deletionMarkObject(nameTable = %2, id = %3)").arg(metaObject()->className(), nameTable, QString::number(_id))
                               << tr("Modificarea statusului 'deletionMark' a obiectului cu 'ID'=%1 nu este reusita. Erroarea:%2")
                                  .arg(QString::number(_id), qry.lastError().text());
        return false;
    }
}

bool DataBase::getObjectDataById(const QString nameTable, const int _id, QMap<QString, QString> &items)
{   
    QSqlQuery qry;
    qry.exec("SELECT * FROM " + nameTable + " WHERE id = " + QString::number(_id) + ";");
    qry.next();
    QSqlRecord rec = qry.record();
    for (int n = 0; n < rec.count(); n++) {
        items.insert(rec.fieldName(n), rec.value(n).toString());
    }
    return true;
}

bool DataBase::postDocument(const QString nameTable, QMap<QString, QString> &items)
{
    QString _key;        // nume colonitelor
    QString _value;      // valoarea
    QMapIterator<QString, QString> it(items);
    while (it.hasNext()) {   // determinam _key and _value
        it.next();
        _key = _key + "'" + it.key() + "',";         // ex: 'name', 'comment',
        _value = _value + "'" + it.value() + "',";
    }
    _key.resize(_key.size() - 1);      // eliminam ultimul simbol (,):
    _value.resize(_value.size() - 1);  // ex: 'name', 'comment'

    QSqlQuery qry;  // determinam id
    //    qry.prepare("SELECT max(id) FROM " + nameTable + ";");
    //    qry.exec();
    //    qry.next();
    //    int _id = qry.value(0).toInt() + 1;

    // cream obiect nou
    qry.prepare("INSERT INTO " + nameTable +
                "(" + _key + ")"
                             " VALUES(" + _value + ");");
    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1: postDocument(nameTable = %2): "
                                     "<br>Validarea documentului a esuat.")
                                  .arg(metaObject()->className(),
                                       nameTable);
        return false;
    }
}

// *******************************************************************
// ************** CREAREA TABELELOR LA PRIMA LANSARE *****************

bool DataBase::createTableUsers()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists users ("
                    "id             INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "deletionMark   INT NOT Null,"
                    "name           VARCHAR (50) NOT Null,"
                    "password       VARCHAR (50),"
                    "hash           CHAR (64),"
                    "lastConnection DATETIME);");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE users ("
                    "id             INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                    "deletionMark   INTEGER NOT NULL,"
                    "name           TEXT (50) NOT NULL,"
                    "password       TEXT (50),"
                    "hash           TEXT (64),"
                    "lastConnection TEXT (19));");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableUsers()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela \"users\".") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableDoctors()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists doctors ("
                    "id           INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "deletionMark INT NOT Null,"
                    "name         VARCHAR (80) NOT Null, "
                    "fName        VARCHAR (50), "
                    "mName        VARCHAR (50),"
                    "telephone    VARCHAR (100),"
                    "email        VARCHAR (100),"
                    "comment      VARCHAR (255),"
                    "signature    LONGBLOB,"
                    "stamp        LONGBLOB);");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE doctors ("
                    "id           INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                    "deletionMark INTEGER NOT NULL,"
                    "name         TEXT (80) NOT NULL, "
                    "fName        TEXT (50), "
                    "mName        TEXT (50),"
                    "telephone    TEXT (100),"
                    "email        TEXT (100),"
                    "comment      TEXT (255),"
                    "signature    BLOB,"
                    "stamp        BLOB);");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableDoctors()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela \"doctors\".") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableFullNameDoctors()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL") {
        qry.prepare("CREATE TABLE `fullNameDoctors` ("
                    "`id` int NOT Null AUTO_INCREMENT,"
                    "`id_doctors`      int NOT Null,"
                    "`name`            varchar(200) NOT Null,"
                    "`nameAbbreviated` varchar(250) DEFAULT Null,"
                    "`nameTelephone`   varchar(250) DEFAULT Null,"
                    "PRIMARY KEY (`id`),"
                    "KEY `fullNameDoctors_doctors_id_idx` (`id_doctors`),"
                    "CONSTRAINT `fullNameDoctors_doctors_id` FOREIGN KEY (`id_doctors`) REFERENCES `doctors` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");"
                    "CREATE TRIGGER `create_full_name_doctor` "
                    "AFTER INSERT ON `doctors` FOR EACH ROW "
                    "BEGIN"
                    "  INSERT INTO fullNameDoctors (id_doctors, name, nameAbbreviated, nameTelephone) "
                    "  VALUES (NEW.id, "
                    "    CONCAT(NEW.name,' ', NEW.fName),"
                    "    CONCAT(NEW.name,' ', SUBSTRING(NEW.fName,1,1),'.'),"
                    "    CONCAT(NEW.name,' ', NEW.fName,', tel.: ', NEW.telephone)"
                    "  );"
                    "END;"
                    "CREATE TRIGGER `update_full_name_doctor` "
                    "AFTER UPDATE ON `doctors` FOR EACH ROW "
                    "BEGIN"
                    "UPDATE fullNameDoctors SET "
                    "  name = CONCAT(NEW.name, ' ', NEW.fName),"
                    "  nameAbbreviated = CONCAT(NEW.name, ' ', SUBSTRING(NEW.fName, 1, 1), '.'),"
                    "  nameTelephone = CONCAT(NEW.name, ' ', NEW.fName, ', tel.: ', NEW.telephone) "
                    "WHERE id_doctors = NEW.id;"
                    "END;");
        if (qry.exec()){
            return true;
        } else {
            qWarning(logWarning()) << tr("%1 - createTableFullNameDoctors()").arg(metaObject()->className())
                                   << tr("Nu a fost creata tabela \"fullNameDoctors\".") + qry.lastError().text();
            return false;
        }
    } else if (globals::connectionMade == "Sqlite") {

        db.transaction();

        try {
            qry.prepare("CREATE TABLE fullNameDoctors ("
                        "id              INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                        "id_doctors      INTEGER NOT NULL CONSTRAINT fullNameDoctors_doctors_id REFERENCES doctors (id) ON DELETE CASCADE ON UPDATE RESTRICT,"
                        "name            TEXT (200) NOT NULL, "
                        "nameAbbreviated TEXT (250), "
                        "nameTelephone   TEXT (250)"
                        ");");
            qry.exec();

            qry.prepare("CREATE TRIGGER `create_full_name_doctor` "
                        "AFTER INSERT ON `doctors` FOR EACH ROW "
                        "BEGIN "
                        "  INSERT INTO fullNameDoctors (id_doctors, name, nameAbbreviated, nameTelephone) "
                        "  VALUES (NEW.id, "
                        "    NEW.name || ' ' || NEW.fName,"
                        "    NEW.name || ' ' || SUBSTR(NEW.fName,1,1) || '.',"
                        "    NEW.name || ' ' || NEW.fName || ', tel.: ' || NEW.telephone"
                        "  );"
                        "END;");
            qry.exec();

            qry.prepare("CREATE TRIGGER `update_full_name_doctor` "
                        "AFTER UPDATE ON `doctors` FOR EACH ROW "
                        "BEGIN"
                        " UPDATE fullNameDoctors SET "
                        "  name = NEW.name || ' ' || NEW.fName,"
                        "  nameAbbreviated = NEW.name || ' ' || SUBSTR(NEW.fName,1,1) || '.',"
                        "  nameTelephone = NEW.name || ' ' || NEW.fName || ', tel.: ' || NEW.telephone "
                        " WHERE id_doctors = NEW.id;"
                        "END;");
            qry.exec();

            db.commit();
            return true;

        } catch (...) {
            db.rollback();
            qWarning(logWarning()) << tr("%1 - createTableFullNameDoctors()").arg(metaObject()->className())
                                   << tr("Nu a fost creata tabela \"fullNameDoctors\".") + qry.lastError().text();
            return false;
            throw;
        }

    } else {
        return false;
    }


}

bool DataBase::createTableNurses()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists nurses ("
                    "id           INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "deletionMark INT NOT Null,"
                    "name         VARCHAR (80) NOT Null, "
                    "fName        VARCHAR (50), "
                    "mName        VARCHAR (50),"
                    "telephone    VARCHAR (100),"
                    "email        VARCHAR (100),"
                    "comment      VARCHAR (255));");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE nurses ("
                    "id           INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                    "deletionMark INTEGER NOT NULL,"
                    "name         TEXT (80) NOT NULL, "
                    "fName        TEXT (50), "
                    "mName        TEXT (50),"
                    "telephone    TEXT (100),"
                    "email        TEXT (100),"
                    "comment      TEXT (255));");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableNurses()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela \"nurses\".") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableFullNameNurses()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL") {

        qry.prepare("CREATE TABLE `fullNameNurses` ("
                    "`id` int NOT Null AUTO_INCREMENT,"
                    "`id_nurses`       int NOT Null,"
                    "`name`            varchar(200) NOT Null,"
                    "`nameAbbreviated` varchar(250) DEFAULT Null,"
                    "`nameTelephone`   varchar(250) DEFAULT Null,"
                    "PRIMARY KEY (`id`),"
                    "KEY `fullNameNurses_nurses_id_idx` (`id_nurses`),"
                    "CONSTRAINT `fullNameNurses_nurses_id` FOREIGN KEY (`id_nurses`) REFERENCES `nurses` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");"
                    "CREATE TRIGGER `create_full_name_nurse` "
                    "AFTER INSERT ON `nurses` FOR EACH ROW "
                    "BEGIN"
                    "  INSERT INTO fullNameNurses (id_nurses, name, nameAbbreviated, nameTelephone) "
                    "  VALUES (NEW.id, "
                    "    CONCAT(NEW.name,' ', NEW.fName),"
                    "    CONCAT(NEW.name,' ', SUBSTRING(NEW.fName,1,1),'.'),"
                    "    CONCAT(NEW.name,' ', NEW.fName,', tel.: ', NEW.telephone)"
                    "  );"
                    "END;"
                    "CREATE TRIGGER `update_full_name_nurse` "
                    "AFTER UPDATE ON `nurses` FOR EACH ROW "
                    "BEGIN"
                    "  UPDATE fullNameNurses SET "
                    "  name = CONCAT(NEW.name, ' ', NEW.fName),"
                    "  nameAbbreviated = CONCAT(NEW.name, ' ', SUBSTRING(NEW.fName, 1, 1), '.'),"
                    "  nameTelephone = CONCAT(NEW.name, ' ', NEW.fName, ', tel.: ', NEW.telephone) "
                    "WHERE id_nurses = NEW.id;"
                    "END;");
        if (qry.exec()){
            return true;
        } else {
            qWarning(logWarning()) << tr("%1 - createTableFullNameNurses()").arg(metaObject()->className())
                                   << tr("Nu a fost creata tabela \"fullNameNurses\".") + qry.lastError().text();
            return false;
        }

    } else if (globals::connectionMade == "Sqlite") {

        db.transaction();
        try {
            qry.prepare("CREATE TABLE fullNameNurses ("
                        "id              INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                        "id_nurses       INTEGER NOT NULL CONSTRAINT fullNameNurses_nurses_id REFERENCES nurses (id) ON DELETE CASCADE ON UPDATE RESTRICT,"
                        "name            TEXT (200) NOT NULL, "
                        "nameAbbreviated TEXT (250), "
                        "nameTelephone   TEXT (250)"
                        ");");
            qry.exec();
            qry.prepare("CREATE TRIGGER `create_full_name_nurse` "
                        "AFTER INSERT ON `nurses` FOR EACH ROW "
                        "BEGIN"
                        "  INSERT INTO fullNameNurses (id_nurses, name, nameAbbreviated, nameTelephone) "
                        "  VALUES (NEW.id, "
                        "    NEW.name || ' ' || NEW.fName,"
                        "    NEW.name || ' ' || SUBSTR(NEW.fName,1,1) || '.',"
                        "    NEW.name || ' ' || NEW.fName || ', tel.: ' || NEW.telephone"
                        "  );"
                        "END;");
            qry.exec();
            qry.prepare("CREATE TRIGGER `update_full_name_nurse` "
                        "AFTER UPDATE ON `nurses` FOR EACH ROW "
                        "BEGIN"
                        " UPDATE fullNameNurses SET "
                        "  name = NEW.name || ' ' || NEW.fName,"
                        "  nameAbbreviated = NEW.name || ' ' || SUBSTR(NEW.fName,1,1) || '.',"
                        "  nameTelephone = NEW.name || ' ' || NEW.fName || ', tel.: ' || NEW.telephone "
                        "WHERE id_nurses = NEW.id;"
                        "END;");
            qry.exec();
            db.commit();
            return true;
        } catch (...) {
            qWarning(logWarning()) << tr("%1 - createTableFullNameNurses()").arg(metaObject()->className())
                                   << tr("Nu a fost creata tabela \"fullNameNurses\".") + qry.lastError().text();
            db.rollback();
            return false;
            throw;
        }
    } else {
        return false;
    }

}

bool DataBase::createTablePacients()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists pacients ("
                    "id            INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "deletionMark  INT NOT Null,"
                    "IDNP          VARCHAR (20),"
                    "name          VARCHAR (80) NOT Null,"
                    "fName         VARCHAR (50),"
                    "mName         VARCHAR (50),"
                    "medicalPolicy VARCHAR (12),"
                    "birthday      DATE NOT Null,"
                    "address       VARCHAR (255),"
                    "telephone     VARCHAR (100),"
                    "email         VARCHAR (100),"
                    "comment       VARCHAR (255));");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE pacients ("
                    "id            INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                    "deletionMark  INTEGER NOT NULL,"
                    "IDNP          TEXT (20),"
                    "name          TEXT (80) NOT NULL,"
                    "fName         TEXT (50),"
                    "mName         TEXT (50),"
                    "medicalPolicy TEXT (12),"
                    "birthday      TEXT (10) NOT NULL,"
                    "address       TEXT (255),"
                    "telephone     TEXT (100),"
                    "email         TEXT (100),"
                    "comment       TEXT (255));");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTablePacients()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela \"pacients\".") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableFullNamePacients()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL") {

        qry.prepare("CREATE TABLE `fullNamePacients` ("
                    "`id`               int NOT Null AUTO_INCREMENT,"
                    "`id_pacients`      int NOT Null,"
                    "`name`             varchar(200) DEFAULT Null,"
                    "`nameIDNP`         varchar(250) DEFAULT Null,"
                    "`nameBirthday`     varchar(250) DEFAULT Null,"
                    "`nameTelephone`    varchar(250) DEFAULT Null,"
                    "`nameBirthdayIDNP` varchar(300) DEFAULT Null,"
                    "PRIMARY KEY (`id`),"
                    "KEY `fullNamePacients_pacients_id_idx` (`id_pacients`),"
                    "CONSTRAINT `fullNamePacients_pacients_id` FOREIGN KEY (`id_pacients`) REFERENCES `pacients` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");"
                    "CREATE TRIGGER `create_full_name_pacient` "
                    "AFTER INSERT ON `pacients` FOR EACH ROW "
                    "BEGIN"
                    "  INSERT INTO fullNamePacients (id_pacients, name, nameIDNP, nameBirthday, nameTelephone, nameBirthdayIDNP) "
                    "  VALUES (NEW.id, "
                    "    CONCAT(NEW.name,' ', NEW.fName),"
                    "    CONCAT(NEW.name,' ', NEW.fName,', idnp: ', NEW.IDNP),"
                    "    CONCAT(NEW.name,' ', NEW.fName,', ', SUBSTRING(NEW.birthday, 9, 2),'.', SUBSTRING(NEW.birthday, 6, 2),'.',SUBSTRING(NEW.birthday, 1, 4)),"
                    "    CONCAT(NEW.name,' ', NEW.fName,', tel.: ', NEW.telephone),"
                    "    CONCAT(NEW.name,' ', NEW.fName,', ', SUBSTRING(NEW.birthday, 9, 2),'.', SUBSTRING(NEW.birthday, 6, 2),'.',SUBSTRING(NEW.birthday, 1, 4), ', idnp: ' , NEW.IDNP)"
                    "  );"
                    "END;"
                    "CREATE TRIGGER `update_full_name_pacient` "
                    "AFTER UPDATE ON `pacients` FOR EACH ROW "
                    "BEGIN"
                    "  UPDATE fullNamePacients SET "
                    "    name = CONCAT(NEW.name,' ', NEW.fName),"
                    "    nameIDNP = CONCAT(NEW.name,' ', NEW.fName,', idnp: ', NEW.IDNP),"
                    "    nameBirthday = CONCAT(NEW.name,' ', NEW.fName,', ', SUBSTRING(NEW.birthday, 9, 2),'.', SUBSTRING(NEW.birthday, 6, 2),'.',SUBSTRING(NEW.birthday, 1, 4)),"
                    "    nameTelephone = CONCAT(NEW.name,' ', NEW.fName,', tel.: ', NEW.telephone),"
                    "    nameBirthdayIDNP = CONCAT(NEW.name,' ', NEW.fName,', ', SUBSTRING(NEW.birthday, 9, 2),'.', SUBSTRING(NEW.birthday, 6, 2),'.',SUBSTRING(NEW.birthday, 1, 4), ', idnp: ' , NEW.IDNP)"
                    "  WHERE id_pacients = NEW.id;"
                    "END"
                    ";");
        if (qry.exec()){
            return true;
        } else {
            qWarning(logWarning()) << tr("%1 - createTableFullNamePacients()").arg(metaObject()->className())
                                   << tr("Nu a fost creata tabela \"fullNamePacients\".") + qry.lastError().text();
            return false;
        }

    } else if (globals::connectionMade == "Sqlite") {

        db.transaction();
        try {
            qry.prepare("CREATE TABLE fullNamePacients ("
                        "id               INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                        "id_pacients      INTEGER NOT NULL CONSTRAINT fullNamePacients_pacients_id REFERENCES pacients (id) ON DELETE CASCADE ON UPDATE RESTRICT,"
                        "name             TEXT(200) DEFAULT NULL,"
                        "nameIDNP         TEXT(250) DEFAULT NULL,"
                        "nameBirthday     TEXT(250) DEFAULT NULL,"
                        "nameTelephone    TEXT(250) DEFAULT NULL,"
                        "nameBirthdayIDNP TEXT (300)"
                        ");");
            qry.exec();

            qry.prepare("CREATE TRIGGER `create_full_name_pacient` "
                        "AFTER INSERT ON `pacients` FOR EACH ROW "
                        "BEGIN"
                        "  INSERT INTO fullNamePacients (id_pacients, name, nameIDNP, nameBirthday, nameTelephone, nameBirthdayIDNP) "
                        "  VALUES (NEW.id, "
                        "    NEW.name || ' ' || NEW.fName,"
                        "    NEW.name || ' ' || NEW.fName || ', idnp: ' || NEW.IDNP,"
                        "    NEW.name || ' ' || NEW.fName || ', ' || SUBSTR(NEW.birthday, 9, 2) || '.' || SUBSTR(NEW.birthday, 6, 2) || '.' || SUBSTR(NEW.birthday, 1, 4),"
                        "    NEW.name || ' ' || NEW.fName || ', tel.: ' || NEW.telephone,"
                        "    NEW.name || ' ' || NEW.fName || ', ' || SUBSTR(NEW.birthday, 9, 2) || '.' || SUBSTR(NEW.birthday, 6, 2) || '.' || SUBSTR(NEW.birthday, 1, 4) || ', idnp: ' || NEW.IDNP"
                        "  );"
                        "END;");
            qry.exec();

            qry.prepare("CREATE TRIGGER `update_full_name_pacient` "
                        "AFTER UPDATE ON `pacients` FOR EACH ROW "
                        "BEGIN"
                        "  UPDATE fullNamePacients SET "
                        "    name = NEW.name || ' ' || NEW.fName,"
                        "    nameIDNP = NEW.name || ' ' || NEW.fName || ', idnp: ' || NEW.IDNP,"
                        "    nameBirthday = NEW.name || ' ' || NEW.fName || ', ' || SUBSTR(NEW.birthday, 9, 2) || '.' || SUBSTR(NEW.birthday, 6, 2) || '.' || SUBSTR(NEW.birthday, 1, 4),"
                        "    nameTelephone = NEW.name || ' ' || NEW.fName || ', tel.: ' || NEW.telephone,"
                        "    nameBirthdayIDNP = NEW.name || ' ' || NEW.fName || ', ' || SUBSTR(NEW.birthday, 9, 2) || '.' || SUBSTR(NEW.birthday, 6, 2) || '.' || SUBSTR(NEW.birthday, 1, 4) || ', idnp: ' || NEW.IDNP"
                        "  WHERE id_pacients = NEW.id;"
                        "END"
                        ";");
            qry.exec();
            db.commit();
            return true;
        } catch (...) {
            qWarning(logWarning()) << tr("%1 - createTableFullNamePacients()").arg(metaObject()->className())
                                   << tr("Nu a fost creata tabela \"fullNamePacients\".") + qry.lastError().text();
            db.rollback();
            return false;
            throw;
        }

    } else {
        return false;
    }

}

bool DataBase::createTableTypesPrices()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists typesPrices ("
                    "id	          INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "deletionMark INT NOT Null,"
                    "name	      VARCHAR (50) NOT Null,"
                    "discount	  DECIMAL (15,3) DEFAULT Null,"
                    "noncomercial BOOLEAN);");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE typesPrices ("
                    "id	          INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "deletionMark INTEGER NOT NULL,"
                    "name	      TEXT (50) NOT NULL,"
                    "discount	  REAL,"
                    "noncomercial INTEGER);");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableTypesPrices()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela \"typesPrices\".") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableOrganizations()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists organizations ("
                    "id           INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "deletionMark INT NOT Null,"
                    "IDNP         VARCHAR (15),"
                    "TVA          VARCHAR (10),"
                    "name         VARCHAR (100) NOT Null,"
                    "address      VARCHAR (255),"
                    "telephone    VARCHAR (100),"
                    "email        VARCHAR (100),"
                    "comment      VARCHAR (255),"
                    "id_contracts INTEGER,"
                    "stamp        LONGBLOB);");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE organizations ("
                    "id           INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                    "deletionMark INTEGER NOT NULL,"
                    "IDNP         TEXT (15),"
                    "TVA          TEXT (10),"
                    "name         TEXT (100) NOT NULL,"
                    "address      TEXT (255),"
                    "telephone    TEXT (100),"
                    "email        TEXT (100),"
                    "comment      TEXT (255),"
                    "id_contracts INTEGER CONSTRAINT organizations_contracts_id REFERENCES contracts (id),"
                    "stamp        BLOB);");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableOrganizations()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela \"organizations\".") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableContracts()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists contracts ("
                    "id               INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "deletionMark     INT NOT Null,"
                    "id_organizations INT NOT Null,"
                    "id_typesPrices   INT,"
                    "name             VARCHAR (50) NOT Null,"
                    "dateInit         DATE,"
                    "notValid         BOOLEAN,"
                    "comment          VARCHAR (255),"
                    "KEY `contracts_organizations_id_idx` (`id_organizations`),"
                    "KEY `contracts_typesPrices_id_idx` (`id_typesPrices`),"
                    "CONSTRAINT `contracts_organizations_id` FOREIGN KEY (`id_organizations`) REFERENCES `organizations` (`id`) ON DELETE CASCADE,"
                    "CONSTRAINT `contracts_typesPrices_id` FOREIGN KEY (`id_typesPrices`) REFERENCES `typesPrices` (`id`) ON DELETE SET NULL ON UPDATE RESTRICT);"
                    "ALTER TABLE `organizations` ADD INDEX `organizations_contracts_id_idx` (`id_contracts` ASC) VISIBLE;"
                    "ALTER TABLE `organizations` ADD CONSTRAINT `organizations_contracts_id` FOREIGN KEY (`id_contracts`) REFERENCES `contracts` (`id`) ON DELETE SET NULL ON UPDATE RESTRICT"
                    ";");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE contracts ("
                    "id               INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                    "deletionMark     INTEGER NOT NULL,"
                    "id_organizations INTEGER NOT NULL CONSTRAINT contracts_organizations_id REFERENCES organizations (id) ON DELETE CASCADE,"
                    "id_typesPrices   INTEGER CONSTRAINT contracts_typesPrices_id REFERENCES typesPrices (id) ON DELETE SET NULL,"
                    "name             TEXT (50) NOT NULL,"
                    "dateInit         TEXT (10),"
                    "notValid         INTEGER NOT NULL,"
                    "comment          TEXT (255));");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableContracts()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela \"contracts\".") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableInvestigations()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists investigations ("
                    "id           INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "deletionMark INT,"
                    "cod          VARCHAR (10),"
                    "name         VARCHAR (500),"
                    "`use`        BOOLEAN,"
                    "KEY `idx_investigations_cod` (`cod`),"
                    "KEY `idx_investigations_name` (`name`));");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE investigations ("
                    "id           INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                    "deletionMark INTEGER NOT NULL,"
                    "cod          TEXT (10) NOT NULL,"
                    "name         TEXT (500) NOT NULL,"
                    "use          INTEGER NOT NULL);");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableInvestigations()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela \"investigations\".") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTablePricings()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists pricings ("
                    "id                 INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "deletionMark       INT NOT Null,"
                    "numberDoc          VARCHAR (15) NOT Null,"
                    "dateDoc            DATETIME NOT Null,"
                    "id_typesPrices     INT NOT Null,"
                    "id_organizations   INT NOT Null,"
                    "id_contracts       INT NOT Null,"
                    "id_users           INT NOT Null,"
                    "comment            VARCHAR (255),"
                    "KEY `pricings_typesProces_id_idx` (`id_typesPrices`),"
                    "KEY `pricings_organizations_id_idx` (`id_organizations`),"
                    "KEY `pricings_contracts_id_idx` (`id_contracts`),"
                    "KEY `pricings_users_id_idx` (`id_users`),"
                    "CONSTRAINT `pricings_contracts_id` FOREIGN KEY (`id_contracts`) REFERENCES `contracts` (`id`),"
                    "CONSTRAINT `pricings_organizations_id` FOREIGN KEY (`id_organizations`) REFERENCES `organizations` (`id`),"
                    "CONSTRAINT `pricings_typesProces_id` FOREIGN KEY (`id_typesPrices`) REFERENCES `typesPrices` (`id`),"
                    "CONSTRAINT `pricings_users_id` FOREIGN KEY (`id_users`) REFERENCES `users` (`id`));");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE pricings ("
                    "id                 INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                    "deletionMark       INTEGER NOT NULL,"
                    "numberDoc          TEXT (15) NOT NULL,"
                    "dateDoc            TEXT (19) NOT NULL,"
                    "id_typesPrices     INTEGER NOT NULL CONSTRAINT pricings_typesPrices_id REFERENCES typesPrices (id) ON DELETE RESTRICT,"
                    "id_organizations   INTEGER NOT NULL CONSTRAINT pricings_organizations_id REFERENCES organizations (id) ON DELETE RESTRICT,"
                    "id_contracts       INTEGER NOT NULL CONSTRAINT pricings_contracts_id REFERENCES contracts (id) ON DELETE RESTRICT,"
                    "id_users           INTEGER NOT NULL CONSTRAINT pricings_users_id REFERENCES users (id) ON DELETE RESTRICT,"
                    "comment            TEXT (255));");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableInvestigations()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela \"pricings\".") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTable_PricingsTable()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists pricingsTable ("
                    "id	          INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "deletionMark INT NOT Null,"
                    "id_pricings  INT NOT Null,"
                    "cod	      VARCHAR (10) NOT Null,"
                    "name	      VARCHAR (500) NOT Null,"
                    "price	      DECIMAL (15,3) DEFAULT '0.00',"
                    "KEY `pricingsTable_pricing_id_idx` (`id_pricings`),"
                    "CONSTRAINT `pricingsTable_pricing_id` FOREIGN KEY (`id_pricings`) REFERENCES `pricings` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE pricingsTable ("
                    "id	          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                    "deletionMark INTEGER NOT NULL,"
                    "id_pricings  INTEGER NOT NULL CONSTRAINT pricingsTable_pricings_id REFERENCES pricings (id) ON DELETE CASCADE,"
                    "cod	      TEXT (10) NOT NULL,"
                    "name	      TEXT (500) NOT NULL,"
                    "price	      REAL);");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTable_PricingsTable()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela \"pricingsTable\".") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTablePricingsPresentation()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL") {

#if defined(Q_OS_LINUX)
        qry.prepare("CREATE TABLE if not exists `pricingsPresentation` ("
                    "`id`                  INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "`id_pricings`         INT NOT Null,"
                    "`docPresentation`     VARCHAR (250) NOT Null,"
                    "`docPresentationDate` VARCHAR (250) NOT Null,"
                    "KEY `pricingsPresentation_pricings_id_idx` (`id_pricings`),"
                    "CONSTRAINT `pricingsPresentation_pricings_id` FOREIGN KEY (`id_pricings`) REFERENCES `pricings` (`id`) ON DELETE CASCADE"
                    ");"
                    "CREATE TRIGGER `create_Pricings_presentation` "
                    "AFTER INSERT ON `pricings` FOR EACH ROW "
                    "BEGIN"
                    "  INSERT INTO pricingsPresentation (id_pricings , docPresentation, docPresentationDate) "
                    "  VALUES (NEW.id,"
                    "    CONCAT('Formarea prețurilor nr.', NEW.numberDoc ,' din ',SUBSTRING(NEW.dateDoc,9,2) ,'.', SUBSTRING(NEW.dateDoc,6,2),'.', SUBSTRING(NEW.dateDoc,1,4),' ',SUBSTRING(NEW.dateDoc,12,8)),"
                    "    CONCAT('Formarea prețurilor nr.', NEW.numberDoc ,' din ',SUBSTRING(NEW.dateDoc,9,2) ,'.', SUBSTRING(NEW.dateDoc,6,2),'.', SUBSTRING(NEW.dateDoc,1,4))"
                    "  );"
                    "END;"
                    "CREATE TRIGGER `update_Pricings_presentation` "
                    "AFTER UPDATE ON `pricings` FOR EACH ROW "
                    "BEGIN"
                    " UPDATE pricingsPresentation SET "
                    "  docPresentation = CONCAT('Formarea prețurilor nr.', NEW.numberDoc ,' din ', SUBSTRING(NEW.dateDoc,9,2) ,'.', SUBSTRING(NEW.dateDoc,6,2),'.', SUBSTRING(NEW.dateDoc,1,4),' ',SUBSTRING(NEW.dateDoc,12,8)),"
                    "  docPresentationDate = CONCAT('Formarea prețurilor nr.', NEW.numberDoc ,' din ', SUBSTRING(NEW.dateDoc,9,2) ,'.', SUBSTRING(NEW.dateDoc,6,2),'.', SUBSTRING(NEW.dateDoc,1,4)) "
                    " WHERE id_pricings = NEW.id; "
                    "END;");
#elif defined(Q_OS_MACOS)
        qry.prepare("CREATE TABLE if not exists `pricingsPresentation` ("
                    "`id`                  INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "`id_pricings`         INT NOT Null,"
                    "`docPresentation`     VARCHAR (250) NOT Null,"
                    "`docPresentationDate` VARCHAR (250) NOT Null,"
                    "KEY `pricingsPresentation_pricings_id_idx` (`id_pricings`),"
                    "CONSTRAINT `pricingsPresentation_pricings_id` FOREIGN KEY (`id_pricings`) REFERENCES `pricings` (`id`) ON DELETE CASCADE"
                    ");"
                    "CREATE TRIGGER `create_Pricings_presentation` "
                    "AFTER INSERT ON `pricings` FOR EACH ROW "
                    "BEGIN"
                    "  INSERT INTO pricingsPresentation (id_pricings , docPresentation, docPresentationDate) "
                    "  VALUES (NEW.id,"
                    "    CONCAT('Formarea prețurilor nr.', NEW.numberDoc ,' din ',SUBSTRING(NEW.dateDoc,9,2) ,'.', SUBSTRING(NEW.dateDoc,6,2),'.', SUBSTRING(NEW.dateDoc,1,4),' ',SUBSTRING(NEW.dateDoc,12,8)),"
                    "    CONCAT('Formarea prețurilor nr.', NEW.numberDoc ,' din ',SUBSTRING(NEW.dateDoc,9,2) ,'.', SUBSTRING(NEW.dateDoc,6,2),'.', SUBSTRING(NEW.dateDoc,1,4))"
                    "  );"
                    "END;"
                    "CREATE TRIGGER `update_Pricings_presentation` "
                    "AFTER UPDATE ON `pricings` FOR EACH ROW "
                    "BEGIN"
                    " UPDATE pricingsPresentation SET "
                    "  docPresentation = CONCAT('Formarea prețurilor nr.', NEW.numberDoc ,' din ', SUBSTRING(NEW.dateDoc,9,2) ,'.', SUBSTRING(NEW.dateDoc,6,2),'.', SUBSTRING(NEW.dateDoc,1,4),' ',SUBSTRING(NEW.dateDoc,12,8)),"
                    "  docPresentationDate = CONCAT('Formarea prețurilor nr.', NEW.numberDoc ,' din ', SUBSTRING(NEW.dateDoc,9,2) ,'.', SUBSTRING(NEW.dateDoc,6,2),'.', SUBSTRING(NEW.dateDoc,1,4)) "
                    " WHERE id_pricings = NEW.id; "
                    "END;");
#elif defined(Q_OS_WIN)
        qry.prepare("CREATE TABLE if not exists `pricingsPresentation` ("
                    "`id`                  INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "`id_pricings`         INT NOT Null,"
                    "`docPresentation`     VARCHAR (250) NOT Null,"
                    "`docPresentationDate` VARCHAR (250) NOT Null,"
                    "KEY `pricingsPresentation_pricings_id_idx` (`id_pricings`),"
                    "CONSTRAINT `pricingsPresentation_pricings_id` FOREIGN KEY (`id_pricings`) REFERENCES `pricings` (`id`) ON DELETE CASCADE"
                    ");"
                    "CREATE TRIGGER `create_Pricings_presentation` "
                    "AFTER INSERT ON `pricings` FOR EACH ROW "
                    "BEGIN"
                    "  INSERT INTO pricingsPresentation (id_pricings , docPresentation, docPresentationDate) "
                    "  VALUES (NEW.id,"
                    "    CONCAT('Formarea pre\310\233urilor nr.', NEW.numberDoc ,' din ',SUBSTRING(NEW.dateDoc,9,2) ,'.', SUBSTRING(NEW.dateDoc,6,2),'.', SUBSTRING(NEW.dateDoc,1,4),' ',SUBSTRING(NEW.dateDoc,12,8)),"
                    "    CONCAT('Formarea pre\310\233urilor nr.', NEW.numberDoc ,' din ',SUBSTRING(NEW.dateDoc,9,2) ,'.', SUBSTRING(NEW.dateDoc,6,2),'.', SUBSTRING(NEW.dateDoc,1,4))"
                    "  );"
                    "END;"
                    "CREATE TRIGGER `update_Pricings_presentation` "
                    "AFTER UPDATE ON `pricings` FOR EACH ROW "
                    "BEGIN"
                    " UPDATE pricingsPresentation SET "
                    "  docPresentation = CONCAT('Formarea pre\310\233urilor nr.', NEW.numberDoc ,' din ', SUBSTRING(NEW.dateDoc,9,2) ,'.', SUBSTRING(NEW.dateDoc,6,2),'.', SUBSTRING(NEW.dateDoc,1,4),' ',SUBSTRING(NEW.dateDoc,12,8)),"
                    "  docPresentationDate = CONCAT('Formarea pre\310\233urilor nr.', NEW.numberDoc ,' din ', SUBSTRING(NEW.dateDoc,9,2) ,'.', SUBSTRING(NEW.dateDoc,6,2),'.', SUBSTRING(NEW.dateDoc,1,4)) "
                    " WHERE id_pricings = NEW.id; "
                    "END;");
#endif
        if (qry.exec()){
            return true;
        } else {
            qWarning(logWarning()) << tr("%1 - createTablePricingsPresentation()").arg(metaObject()->className())
                                   << tr("Nu a fost creata tabela \"pricingsPresentation\".") + qry.lastError().text();
            return false;
        }

    } else if (globals::connectionMade == "Sqlite") {

        db.transaction();

        try {
            qry.prepare("CREATE TABLE pricingsPresentation ("
                        "id              INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                        "id_pricings     INTEGER NOT NULL CONSTRAINT pricingsPresentation_pricings_id REFERENCES pricings (id) ON DELETE CASCADE ON UPDATE RESTRICT,"
                        "docPresentation TEXT (250) NOT NULL,"
                        "docPresentationDate TEXT (250) NOT NULL"
                        ");");
            qry.exec();

#if defined(Q_OS_LINUX)
            qry.prepare("CREATE TRIGGER `create_Pricings_presentation` "
                        "AFTER INSERT ON `pricings` FOR EACH ROW "
                        "BEGIN"
                        "  INSERT INTO pricingsPresentation (id_pricings , docPresentation, docPresentationDate) "
                        "  VALUES (NEW.id,"
                        "    'Formarea prețurilor nr.' || NEW.numberDoc || ' din ' || SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) || ' ' || SUBSTR(NEW.dateDoc,12,8),"
                        "    'Formarea prețurilor nr.' || NEW.numberDoc || ' din ' || SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4)"
                        "  );"
                        "END;");
            qry.exec();

            qry.prepare("CREATE TRIGGER `update_Pricings_presentation` "
                        "AFTER UPDATE ON `pricings` FOR EACH ROW "
                        "BEGIN"
                        "  UPDATE pricingsPresentation SET "
                        "  docPresentation = 'Formarea prețurilor nr.' || NEW.numberDoc || ' din ' || SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) || ' ' || SUBSTR(NEW.dateDoc,12,8),"
                        "  docPresentationDate = 'Formarea prețurilor nr.' || NEW.numberDoc || ' din ' || SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) "
                        " WHERE id_pricings = NEW.id;"
                        "END;");
            qry.exec();
#elif defined(Q_OS_MACOS)
            qry.prepare("CREATE TRIGGER `create_Pricings_presentation` "
                        "AFTER INSERT ON `pricings` FOR EACH ROW "
                        "BEGIN"
                        "  INSERT INTO pricingsPresentation (id_pricings , docPresentation, docPresentationDate) "
                        "  VALUES (NEW.id,"
                        "    'Formarea prețurilor nr.' || NEW.numberDoc || ' din ' || SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) || ' ' || SUBSTR(NEW.dateDoc,12,8),"
                        "    'Formarea prețurilor nr.' || NEW.numberDoc || ' din ' || SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4)"
                        "  );"
                        "END;");
            qry.exec();

            qry.prepare("CREATE TRIGGER `update_Pricings_presentation` "
                        "AFTER UPDATE ON `pricings` FOR EACH ROW "
                        "BEGIN"
                        "  UPDATE pricingsPresentation SET "
                        "  docPresentation = 'Formarea prețurilor nr.' || NEW.numberDoc || ' din ' || SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) || ' ' || SUBSTR(NEW.dateDoc,12,8),"
                        "  docPresentationDate = 'Formarea prețurilor nr.' || NEW.numberDoc || ' din ' || SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) "
                        " WHERE id_pricings = NEW.id;"
                        "END;");
            qry.exec();
#elif defined(Q_OS_WIN)
            qry.prepare("CREATE TRIGGER `create_Pricings_presentation` "
                        "AFTER INSERT ON `pricings` FOR EACH ROW "
                        "BEGIN"
                        "  INSERT INTO pricingsPresentation (id_pricings , docPresentation, docPresentationDate) "
                        "  VALUES (NEW.id,"
                        "    'Formarea pre\310\233urilor nr.' || NEW.numberDoc || ' din ' || SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) || ' ' || SUBSTR(NEW.dateDoc,12,8),"
                        "    'Formarea pre\310\233urilor nr.' || NEW.numberDoc || ' din ' || SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4)"
                        "  );"
                        "END;");
            qry.exec();

            qry.prepare("CREATE TRIGGER `update_Pricings_presentation` "
                        "AFTER UPDATE ON `pricings` FOR EACH ROW "
                        "BEGIN"
                        "  UPDATE pricingsPresentation SET "
                        "  docPresentation = 'Formarea pre\310\233urilor nr.' || NEW.numberDoc || ' din ' || SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) || ' ' || SUBSTR(NEW.dateDoc,12,8),"
                        "  docPresentationDate = 'Formarea pre\310\233urilor nr.' || NEW.numberDoc || ' din ' || SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) "
                        " WHERE id_pricings = NEW.id;"
                        "END;");
            qry.exec();
#endif
            db.commit();
            return true;

        } catch (...) {
            db.rollback();
            return false;
            throw;
        }

    } else {
        return false;
    }
}

bool DataBase::createTableOrderEcho()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists orderEcho ("
                    "id                 INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "deletionMark       INT NOT Null,"
                    "numberDoc          VARCHAR (15),"
                    "dateDoc            DATETIME,"
                    "id_organizations   INT,"
                    "id_contracts       INT,"
                    "id_typesPrices     INT,"
                    "id_doctors         INT,"
                    "id_doctors_execute INT,"
                    "id_nurses          INT,"
                    "id_pacients        INT,"
                    "id_users           INT,"
                    "sum                DECIMAL (15,2) DEFAULT '0.00',"
                    "comment            VARCHAR (255),"
                    "cardPayment        INT,"
                    "attachedImages     INT,"
                    "KEY `orderEcho_organizations_id_idx` (`id_organizations`),"
                    "KEY `orderEcho_contracts_idx` (`id_contracts`),"
                    "KEY `orderEcho_typesPrices_id_idx` (`id_typesPrices`),"
                    "KEY `orderEcho_doctors_id_idx` (`id_doctors`),"
                    "KEY `orderEcho_doctors_execute_id_idx` (`id_doctors_execute`),"
                    "KEY `orderEcho_nurses_id_idx` (`id_nurses`),"
                    "KEY `orderEcho_pacients_id_idx` (`id_pacients`),"
                    "KEY `orderEcho_users_id_idx` (`id_users`),"
                    "CONSTRAINT `orderEcho_contracts_id` FOREIGN KEY (`id_contracts`) REFERENCES `contracts` (`id`),"
                    "CONSTRAINT `orderEcho_doctors_id` FOREIGN KEY (`id_doctors`) REFERENCES `doctors` (`id`),"
                    "CONSTRAINT `orderEcho_doctors_execute_id` FOREIGN KEY (`id_doctors_execute`) REFERENCES `doctors` (`id`),"
                    "CONSTRAINT `orderEcho_nurses_id` FOREIGN KEY (`id_nurses`) REFERENCES `nurses` (`id`),"
                    "CONSTRAINT `orderEcho_organizations_id` FOREIGN KEY (`id_organizations`) REFERENCES `organizations` (`id`),"
                    "CONSTRAINT `orderEcho_pacients_id` FOREIGN KEY (`id_pacients`) REFERENCES `pacients` (`id`),"
                    "CONSTRAINT `orderEcho_typesPrices_id` FOREIGN KEY (`id_typesPrices`) REFERENCES `typesPrices` (`id`),"
                    "CONSTRAINT `orderEcho_users_id` FOREIGN KEY (`id_users`) REFERENCES `users` (`id`)"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE orderEcho ("
                    "id                 INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                    "deletionMark       INTEGER NOT NULL,"
                    "numberDoc          TEXT (15),"
                    "dateDoc            TEXT (19),"
                    "id_organizations   INTEGER CONSTRAINT orderEcho_organizations_id REFERENCES organizations (id) ON DELETE RESTRICT,"
                    "id_contracts       INTEGER CONSTRAINT orderEcho_contracts_id REFERENCES contracts (id) ON DELETE RESTRICT,"
                    "id_typesPrices     INTEGER CONSTRAINT orderEcho_typesPrices_id REFERENCES typesPrices (id) ON DELETE RESTRICT,"
                    "id_doctors         INTEGER CONSTRAINT orderEcho_doctors_id REFERENCES doctors (id) ON DELETE RESTRICT,"
                    "id_doctors_execute INTEGER CONSTRAINT orderEcho_doctors_execute_id REFERENCES doctors (id) ON DELETE RESTRICT,"
                    "id_nurses          INTEGER CONSTRAINT orderEcho_nurses_id REFERENCES nurses (id) ON DELETE RESTRICT,"
                    "id_pacients        INTEGER CONSTRAINT orderEcho_pacients_id REFERENCES pacients (id) ON DELETE RESTRICT,"
                    "id_users           INTEGER CONSTRAINT orderEcho_users_id REFERENCES users (id) ON DELETE RESTRICT,"
                    "sum                REAL,"
                    "comment            TEXT (255),"
                    "cardPayment        INTEGER,"
                    "attachedImages     INTEGER"
                    ");");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableOrderEcho()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela \"orderEcho\".") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTable_OrderEchoTable()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists orderEchoTable ("
                    "id           INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "deletionMark INT NOT Null,"
                    "id_orderEcho INT,"
                    "cod          VARCHAR (10),"
                    "name         VARCHAR (500) NOT Null,"
                    "price        DECIMAL (15,3) DEFAULT '0.00',"
                    "KEY `orderEchoTable_orderEcho_id_idx` (`id_orderEcho`),"
                    "CONSTRAINT `orderEchoTable_orderEcho_id` FOREIGN KEY (`id_orderEcho`) REFERENCES `orderEcho` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE orderEchoTable ("
                    "id           INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                    "deletionMark INTEGER NOT NULL,"
                    "id_orderEcho INTEGER CONSTRAINT orderEchoTable_orderEcho_id REFERENCES orderEcho (id) ON DELETE CASCADE,"
                    "cod          TEXT (10),"
                    "name         TEXT (500),"
                    "price        REAL"
                    ");");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTable_OrderEchoTable()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela \"orderEchoTable\".") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableOrderEchoPresentation()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL") {

#if defined(Q_OS_LINUX)
        qry.prepare("CREATE TABLE `orderEchoPresentation` ("
                    "`id`                  INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "`id_orderEcho`        INT NOT Null,"
                    "`docPresentation`     VARCHAR (250) NOT Null,"
                    "`docPresentationDate` VARCHAR (250) NOT Null,"
                    "KEY `orderEchoPresentation_orderEcho_id_idx` (`id_orderEcho`),"
                    "CONSTRAINT `orderEchoPresentation_orderEcho_id` FOREIGN KEY (`id_orderEcho`) REFERENCES `orderEcho` (`id`) ON DELETE CASCADE"
                    ");"
                    "CREATE TRIGGER `create_orderDoc_presentation` "
                    "AFTER INSERT ON `orderEcho` FOR EACH ROW "
                    "BEGIN"
                    "  INSERT INTO orderEchoPresentation (id_orderEcho, docPresentation, docPresentationDate) "
                    "  VALUES (NEW.id,"
                    "    CONCAT('Comanda ecografică nr.', NEW.numberDoc ,' din ',"
                    "    SUBSTRING(NEW.dateDoc,9,2) ,'.', SUBSTRING(NEW.dateDoc,6,2),'.', SUBSTRING(NEW.dateDoc,1,4),' ',SUBSTRING(NEW.dateDoc,12,8)),"
                    "    CONCAT('Comanda ecografică nr.', NEW.numberDoc ,' din ',"
                    "    SUBSTRING(NEW.dateDoc,9,2) ,'.', SUBSTRING(NEW.dateDoc,6,2),'.', SUBSTRING(NEW.dateDoc,1,4))"
                    "  );"
                    "END;"
                    "CREATE TRIGGER `update_orderDoc_presentation` "
                    "AFTER UPDATE ON `orderEcho` FOR EACH ROW "
                    "BEGIN"
                    "  UPDATE orderEchoPresentation SET "
                    "    docPresentation = CONCAT('Comanda ecografică nr.', NEW.numberDoc , ' din ', SUBSTRING(NEW.dateDoc, 9, 2) , '.', SUBSTRING(NEW.dateDoc, 6, 2), '.', SUBSTRING(NEW.dateDoc, 1, 4), ' ', SUBSTRING(NEW.dateDoc, 12, 8)),"
                    "    docPresentationDate = CONCAT('Comanda ecografică nr.', NEW.numberDoc , ' din ', SUBSTRING(NEW.dateDoc, 9, 2) , '.', SUBSTRING(NEW.dateDoc, 6, 2), '.', SUBSTRING(NEW.dateDoc, 1, 4)) "
                    "  WHERE id_orderEcho = NEW.id;"
                    "END;");
#elif defined(Q_OS_MACOS)
        qry.prepare("CREATE TABLE `orderEchoPresentation` ("
                    "`id`                  INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "`id_orderEcho`        INT NOT Null,"
                    "`docPresentation`     VARCHAR (250) NOT Null,"
                    "`docPresentationDate` VARCHAR (250) NOT Null,"
                    "KEY `orderEchoPresentation_orderEcho_id_idx` (`id_orderEcho`),"
                    "CONSTRAINT `orderEchoPresentation_orderEcho_id` FOREIGN KEY (`id_orderEcho`) REFERENCES `orderEcho` (`id`) ON DELETE CASCADE"
                    ");"
                    "CREATE TRIGGER `create_orderDoc_presentation` "
                    "AFTER INSERT ON `orderEcho` FOR EACH ROW "
                    "BEGIN"
                    "  INSERT INTO orderEchoPresentation (id_orderEcho, docPresentation, docPresentationDate) "
                    "  VALUES (NEW.id,"
                    "    CONCAT('Comanda ecografică nr.', NEW.numberDoc ,' din ',"
                    "    SUBSTRING(NEW.dateDoc,9,2) ,'.', SUBSTRING(NEW.dateDoc,6,2),'.', SUBSTRING(NEW.dateDoc,1,4),' ',SUBSTRING(NEW.dateDoc,12,8)),"
                    "    CONCAT('Comanda ecografică nr.', NEW.numberDoc ,' din ',"
                    "    SUBSTRING(NEW.dateDoc,9,2) ,'.', SUBSTRING(NEW.dateDoc,6,2),'.', SUBSTRING(NEW.dateDoc,1,4))"
                    "  );"
                    "END;"
                    "CREATE TRIGGER `update_orderDoc_presentation` "
                    "AFTER UPDATE ON `orderEcho` FOR EACH ROW "
                    "BEGIN"
                    "  UPDATE orderEchoPresentation SET "
                    "    docPresentation = CONCAT('Comanda ecografică nr.', NEW.numberDoc , ' din ', SUBSTRING(NEW.dateDoc, 9, 2) , '.', SUBSTRING(NEW.dateDoc, 6, 2), '.', SUBSTRING(NEW.dateDoc, 1, 4), ' ', SUBSTRING(NEW.dateDoc, 12, 8)),"
                    "    docPresentationDate = CONCAT('Comanda ecografică nr.', NEW.numberDoc , ' din ', SUBSTRING(NEW.dateDoc, 9, 2) , '.', SUBSTRING(NEW.dateDoc, 6, 2), '.', SUBSTRING(NEW.dateDoc, 1, 4)) "
                    "  WHERE id_orderEcho = NEW.id;"
                    "END;");
#elif defined(Q_OS_WIN)
        qry.prepare("CREATE TABLE `orderEchoPresentation` ("
                    "`id`                  INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "`id_orderEcho`        INT NOT Null,"
                    "`docPresentation`     VARCHAR (250) NOT Null,"
                    "`docPresentationDate` VARCHAR (250) NOT Null,"
                    "KEY `orderEchoPresentation_orderEcho_id_idx` (`id_orderEcho`),"
                    "CONSTRAINT `orderEchoPresentation_orderEcho_id` FOREIGN KEY (`id_orderEcho`) REFERENCES `orderEcho` (`id`) ON DELETE CASCADE"
                    ");"
                    "CREATE TRIGGER `create_orderDoc_presentation` "
                    "AFTER INSERT ON `orderEcho` FOR EACH ROW "
                    "BEGIN"
                    "  INSERT INTO orderEchoPresentation (id_orderEcho, docPresentation, docPresentationDate) "
                    "  VALUES (NEW.id,"
                    "    CONCAT('Comanda ecografic\304\203 nr.', NEW.numberDoc ,' din ',"
                    "    SUBSTRING(NEW.dateDoc,9,2) ,'.', SUBSTRING(NEW.dateDoc,6,2),'.', SUBSTRING(NEW.dateDoc,1,4),' ',SUBSTRING(NEW.dateDoc,12,8)),"
                    "    CONCAT('Comanda ecografic\304\203 nr.', NEW.numberDoc ,' din ',"
                    "    SUBSTRING(NEW.dateDoc,9,2) ,'.', SUBSTRING(NEW.dateDoc,6,2),'.', SUBSTRING(NEW.dateDoc,1,4))"
                    "  );"
                    "END;"
                    "CREATE TRIGGER `update_orderDoc_presentation` "
                    "AFTER UPDATE ON `orderEcho` FOR EACH ROW "
                    "BEGIN"
                    "  UPDATE orderEchoPresentation SET "
                    "    docPresentation = CONCAT('Comanda ecografic\304\203 nr.', NEW.numberDoc , ' din ', SUBSTRING(NEW.dateDoc, 9, 2) , '.', SUBSTRING(NEW.dateDoc, 6, 2), '.', SUBSTRING(NEW.dateDoc, 1, 4), ' ', SUBSTRING(NEW.dateDoc, 12, 8)),"
                    "    docPresentationDate = CONCAT('Comanda ecografic\304\203 nr.', NEW.numberDoc , ' din ', SUBSTRING(NEW.dateDoc, 9, 2) , '.', SUBSTRING(NEW.dateDoc, 6, 2), '.', SUBSTRING(NEW.dateDoc, 1, 4)) "
                    "  WHERE id_orderEcho = NEW.id;"
                    "END;");
#endif
        if (qry.exec()){
            return true;
        } else {
            qWarning(logWarning()) << tr("%1 - createTableOrderEchoPresentation()").arg(metaObject()->className())
                                   << tr("Nu a fost creata tabela \"orderEchoPresentation\".") + qry.lastError().text();
            return false;
        }

    } else if (globals::connectionMade == "Sqlite") {

        db.transaction();

        try {

            qry.prepare("CREATE TABLE orderEchoPresentation ("
                        "id              INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                        "id_orderEcho    INTEGER NOT NULL CONSTRAINT orderEchoPresentation_orderEcho_id REFERENCES orderEcho (id) ON DELETE CASCADE ON UPDATE RESTRICT,"
                        "docPresentation TEXT (250) NOT NULL,"
                        "docPresentationDate TEXT (250)"
                        ");");
            qry.exec();

#if defined(Q_OS_LINUX)
            qry.prepare("CREATE TRIGGER `create_orderDoc_presentation` "
                        "AFTER INSERT ON `orderEcho` FOR EACH ROW "
                        "BEGIN"
                        "  INSERT INTO orderEchoPresentation (id_orderEcho, docPresentation, docPresentationDate) "
                        "  VALUES (NEW.id,"
                        "    'Comanda ecografică nr.' || NEW.numberDoc || ' din ' ||"
                        "    SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) || ' ' || SUBSTR(NEW.dateDoc,12,8),"
                        "    'Comanda ecografică nr.' || NEW.numberDoc || ' din ' ||"
                        "    SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4)"
                        "  );"
                        "END;");
            qry.exec();

            qry.prepare("CREATE TRIGGER `update_orderDoc_presentation` "
                        "AFTER UPDATE ON `orderEcho` FOR EACH ROW "
                        "BEGIN"
                        "  UPDATE orderEchoPresentation SET "
                        "    docPresentation = 'Comanda ecografică nr.' || NEW.numberDoc || ' din ' || SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) || ' ' || SUBSTR(NEW.dateDoc,12,8),"
                        "    docPresentationDate = 'Comanda ecografică nr.' || NEW.numberDoc || ' din ' || SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) "
                        "  WHERE id_orderEcho = NEW.id;"
                        "END;");
            qry.exec();
#elif defined(Q_OS_MACOS)
            qry.prepare("CREATE TRIGGER `create_orderDoc_presentation` "
                        "AFTER INSERT ON `orderEcho` FOR EACH ROW "
                        "BEGIN"
                        "  INSERT INTO orderEchoPresentation (id_orderEcho, docPresentation, docPresentationDate) "
                        "  VALUES (NEW.id,"
                        "    'Comanda ecografică nr.' || NEW.numberDoc || ' din ' ||"
                        "    SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) || ' ' || SUBSTR(NEW.dateDoc,12,8),"
                        "    'Comanda ecografică nr.' || NEW.numberDoc || ' din ' ||"
                        "    SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4)"
                        "  );"
                        "END;");
            qry.exec();

            qry.prepare("CREATE TRIGGER `update_orderDoc_presentation` "
                        "AFTER UPDATE ON `orderEcho` FOR EACH ROW "
                        "BEGIN"
                        "  UPDATE orderEchoPresentation SET "
                        "    docPresentation = 'Comanda ecografică nr.' || NEW.numberDoc || ' din ' || SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) || ' ' || SUBSTR(NEW.dateDoc,12,8),"
                        "    docPresentationDate = 'Comanda ecografică nr.' || NEW.numberDoc || ' din ' || SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) "
                        "  WHERE id_orderEcho = NEW.id;"
                        "END;");
            qry.exec();
#elif defined(Q_OS_WIN)
            qry.prepare("CREATE TRIGGER `create_orderDoc_presentation` "
                        "AFTER INSERT ON `orderEcho` FOR EACH ROW "
                        "BEGIN"
                        "  INSERT INTO orderEchoPresentation (id_orderEcho, docPresentation, docPresentationDate) "
                        "  VALUES (NEW.id,"
                        "    'Comanda ecografic\304\203 nr.' || NEW.numberDoc || ' din ' ||"
                        "    SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) || ' ' || SUBSTR(NEW.dateDoc,12,8),"
                        "    'Comanda ecografic\304\203 nr.' || NEW.numberDoc || ' din ' ||"
                        "    SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4)"
                        "  );"
                        "END;");
            qry.exec();

            qry.prepare("CREATE TRIGGER `update_orderDoc_presentation` "
                        "AFTER UPDATE ON `orderEcho` FOR EACH ROW "
                        "BEGIN"
                        "  UPDATE orderEchoPresentation SET "
                        "    docPresentation = 'Comanda ecografic\304\203 nr.' || NEW.numberDoc || ' din ' || SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) || ' ' || SUBSTR(NEW.dateDoc,12,8),"
                        "    docPresentationDate = 'Comanda ecografic\304\203 nr.' || NEW.numberDoc || ' din ' || SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) "
                        "  WHERE id_orderEcho = NEW.id;"
                        "END;");
            qry.exec();
#endif
            return db.commit();
        } catch (...) {
            qWarning(logWarning()) << tr("%1 - createTableOrderEchoPresentation()").arg(metaObject()->className())
                                   << tr("Nu a fost creata tabela \"orderEchoPresentation\".") + qry.lastError().text();
            db.rollback();
            return false;
            throw;
        }

    } else {
        return false;
    }

}

bool DataBase::createTableConstants()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists constants ("
                    "id_users	        INT NOT Null,"
                    "id_organizations	INT,"
                    "id_doctors	        INT,"
                    "id_nurses	        INT,"
                    "brandUSG	        VARCHAR (200),"
                    "logo               LONGBLOB,"
                    "KEY `constants_users_id_idx` (`id_users`),"
                    "KEY `constants_organizations_id_idx` (`id_organizations`),"
                    "KEY `constants_doctors_id_idx` (`id_doctors`),"
                    "KEY `constants_nurses_id_idx` (`id_nurses`),"
                    "CONSTRAINT `constants_doctors_id` FOREIGN KEY (`id_doctors`) REFERENCES `doctors` (`id`) ON DELETE SET NULL ON UPDATE RESTRICT,"
                    "CONSTRAINT `constants_nurses_id` FOREIGN KEY (`id_nurses`) REFERENCES `nurses` (`id`) ON DELETE SET NULL ON UPDATE RESTRICT,"
                    "CONSTRAINT `constants_organizations_id` FOREIGN KEY (`id_organizations`) REFERENCES `organizations` (`id`) ON DELETE SET NULL ON UPDATE RESTRICT,"
                    "CONSTRAINT `constants_users_id` FOREIGN KEY (`id_users`) REFERENCES `users` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE constants ("
                    "id_users	        INTEGER NOT NULL CONSTRAINT constants_users_id REFERENCES users (id) ON DELETE CASCADE,"
                    "id_organizations	INTEGER CONSTRAINT constants_organizations_id REFERENCES organizations (id) ON DELETE SET NULL,"
                    "id_doctors	        INTEGER CONSTRAINT constants_doctors_id REFERENCES doctors (id) ON DELETE SET NULL,"
                    "id_nurses	        INTEGER CONSTRAINT constants_nurses_id REFERENCES nurses (id) ON DELETE SET NULL,"
                    "brandUSG	        TEXT (200),"
                    "logo               BLOB);");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableConstants()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela \"constants\".") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableReportEcho()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists reportEcho ("
                    "id                INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "deletionMark      INT NOT Null,"
                    "numberDoc         VARCHAR (15) NOT Null,"
                    "dateDoc           DATETIME NOT Null,"
                    "id_pacients       INT NOT Null,"
                    "id_orderEcho      INT NOT Null,"
                    "t_organs_internal BOOLEAN,"
                    "t_urinary_system  BOOLEAN,"
                    "t_prostate        BOOLEAN,"
                    "t_gynecology      BOOLEAN,"
                    "t_breast          BOOLEAN,"
                    "t_thyroid         BOOLEAN,"
                    "t_gestation0      BOOLEAN,"
                    "t_gestation1      BOOLEAN,"
                    "t_gestation2      BOOLEAN,"
                    "t_gestation3      BOOLEAN,"
                    "id_users          INT,"
                    "concluzion        VARCHAR (700),"
                    "comment           VARCHAR (255),"
                    "attachedImages    INT,"
                    "KEY `reportEcho_pacients_id_idx` (`id_pacients`),"
                    "KEY `reportEcho_order_id_idx` (`id_orderEcho`),"
                    "KEY `reportEcho_users_id_idx` (`id_users`),"
                    "CONSTRAINT `reportEcho_orderEcho_id` FOREIGN KEY (`id_orderEcho`) REFERENCES `orderEcho` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT,"
                    "CONSTRAINT `reportEcho_pacients_id` FOREIGN KEY (`id_pacients`) REFERENCES `pacients` (`id`),"
                    "CONSTRAINT `reportEcho_users_id` FOREIGN KEY (`id_users`) REFERENCES `users` (`id`)"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE reportEcho ("
                    "id                INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "deletionMark      INTEGER NOT NULL,"
                    "numberDoc         TEXT (15),"
                    "dateDoc           TEXT (19),"
                    "id_pacients       INTEGER NOT NULL CONSTRAINT reportEcho_pacients_id REFERENCES pacients (id) ON DELETE RESTRICT,"
                    "id_orderEcho      INTEGER NOT NULL CONSTRAINT reportEcho_orderEcho_id REFERENCES orderEcho (id) ON DELETE CASCADE,"
                    "t_organs_internal INTEGER,"
                    "t_urinary_system  INTEGER,"
                    "t_prostate        INTEGER,"
                    "t_gynecology      INTEGER,"
                    "t_breast          INTEGER,"
                    "t_thyroid         INTEGER,"
                    "t_gestation0      INTEGER,"
                    "t_gestation1      INTEGER,"
                    "t_gestation2      INTEGER,"
                    "t_gestation3      INTEGER,"
                    "id_users          INTEGER CONSTRAINT reportEcho_users_id REFERENCES users (id) ON DELETE RESTRICT,"
                    "concluzion        TEXT (700),"
                    "comment           TEXT (255),"
                    "attachedImages    INTEGER"
                    ");");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableReportEcho()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela \"reportEcho\".") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableReportEchoPresentation()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL") {

        qry.prepare("CREATE TABLE `reportEchoPresentation` ("
                    "`id`                  INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "`id_reportEcho`       INT NOT Null,"
                    "`docPresentation`     VARCHAR (250) NOT Null,"
                    "`docPresentationDate` VARCHAR (250) NOT Null,"
                    "KEY `reportEchoPresentation_reportEcho_id_idx` (`id_reportEcho`),"
                    "CONSTRAINT `reportEchoPresentation_reportEcho_id` FOREIGN KEY (`id_reportEcho`) REFERENCES `reportEcho` (`id`) ON DELETE CASCADE"
                    ");"
                    "CREATE TRIGGER `create_reportEcho_presentation` "
                    "AFTER INSERT ON `reportEcho` FOR EACH ROW "
                    "BEGIN"
                    "  INSERT INTO reportEchoPresentation (id_reportEcho , docPresentation, docPresentationDate) "
                    "  VALUES (NEW.id,"
                    "    CONCAT('Raport ecografic nr.', NEW.numberDoc ,' din ',"
                    "    SUBSTRING(NEW.dateDoc,9,2) ,'.', SUBSTRING(NEW.dateDoc,6,2),'.', SUBSTRING(NEW.dateDoc,1,4),' ',SUBSTRING(NEW.dateDoc,12,8)),"
                    "    CONCAT('Raport ecografic nr.', NEW.numberDoc ,' din ',"
                    "    SUBSTRING(NEW.dateDoc,9,2) ,'.', SUBSTRING(NEW.dateDoc,6,2),'.', SUBSTRING(NEW.dateDoc,1,4))"
                    "  );"
                    "END;"
                    "CREATE TRIGGER `update_reportEcho_presentation` "
                    "AFTER UPDATE ON `reportEcho` FOR EACH ROW "
                    "BEGIN"
                    "  UPDATE reportEchoPresentation SET "
                    "    docPresentation = CONCAT('Raport ecografic nr.', NEW.numberDoc , ' din ', SUBSTRING(NEW.dateDoc, 9, 2) , '.', SUBSTRING(NEW.dateDoc, 6, 2), '.', SUBSTRING(NEW.dateDoc, 1, 4), ' ', SUBSTRING(NEW.dateDoc, 12, 8)),"
                    "    docPresentationDate = CONCAT('Raport ecografic nr.', NEW.numberDoc , ' din ', SUBSTRING(NEW.dateDoc, 9, 2) , '.', SUBSTRING(NEW.dateDoc, 6, 2), '.', SUBSTRING(NEW.dateDoc, 1, 4))"
                    "  WHERE id_reportEcho = NEW.id;"
                    "END;");
        if (qry.exec()){
            return true;
        } else {
            qWarning(logWarning()) << tr("%1 - createTableReportEchoPresentation()").arg(metaObject()->className())
                                   << tr("Nu a fost creata tabela \"reportEchoPresentation\".") + qry.lastError().text();
            return false;
        }

    } else if (globals::connectionMade == "Sqlite") {

        db.transaction();

        try {
            qry.prepare("CREATE TABLE reportEchoPresentation ("
                        "id                  INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                        "id_reportEcho       INTEGER NOT NULL CONSTRAINT reportEchoPresentation_reportEcho_id REFERENCES reportEcho (id) ON DELETE CASCADE ON UPDATE RESTRICT,"
                        "docPresentation     TEXT (250) NOT NULL,"
                        "docPresentationDate TEXT (250)"
                        ");");
            qry.exec();

            qry.prepare("CREATE TRIGGER `create_reportEcho_presentation` "
                        "AFTER INSERT ON `reportEcho` FOR EACH ROW "
                        "BEGIN"
                        "  INSERT INTO reportEchoPresentation (id_reportEcho , docPresentation, docPresentationDate) "
                        "  VALUES (NEW.id,"
                        "    'Raport ecografic nr.' || NEW.numberDoc || ' din ' ||"
                        "    SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) || ' ' ||SUBSTR(NEW.dateDoc,12,8),"
                        "    'Raport ecografic nr.' || NEW.numberDoc || ' din ' ||"
                        "    SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4)"
                        "  );"
                        "END;");
            qry.exec();

            qry.prepare("CREATE TRIGGER `update_reportEcho_presentation` "
                        "AFTER UPDATE ON `reportEcho` FOR EACH ROW "
                        "BEGIN"
                        "  UPDATE reportEchoPresentation SET "
                        "    docPresentation = 'Raport ecografic nr.' || NEW.numberDoc || ' din ' || SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) || ' ' ||SUBSTR(NEW.dateDoc,12,8),"
                        "    docPresentationDate = 'Raport ecografic nr.' || NEW.numberDoc || ' din ' || SUBSTR(NEW.dateDoc,9,2) || '.' || SUBSTR(NEW.dateDoc,6,2) || '.' || SUBSTR(NEW.dateDoc,1,4) "
                        "  WHERE id_reportEcho = NEW.id;"
                        "END;");
            qry.exec();

            return db.commit();

        } catch (...) {
            db.rollback();
            return false;
            throw;
        }

    } else {
        return false;
    }
}

bool DataBase::createTableLiver()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists tableLiver ("
                    "id                INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "id_reportEcho     INT NOT Null,"
                    "`left`            VARCHAR (5),"
                    "`right`           VARCHAR (5),"
                    "contur            VARCHAR (20),"
                    "parenchim         VARCHAR (20),"
                    "ecogenity         VARCHAR (30),"
                    "formations        VARCHAR (300),"
                    "ductsIntrahepatic VARCHAR (50),"
                    "porta             VARCHAR (5),"
                    "lienalis          VARCHAR (5),"
                    "concluzion        VARCHAR (500),"
                    "recommendation    VARCHAR (255),"
                    "KEY `tableLiver_reportEcho_id_idx` (`id_reportEcho`),"
                    "CONSTRAINT `tableLiver_reportEcho_id` FOREIGN KEY (`id_reportEcho`) REFERENCES `reportEcho` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE tableLiver ("
                    "id                INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "id_reportEcho     INTEGER NOT NULL CONSTRAINT tableLiver_reportEcho_id REFERENCES reportEcho (id) ON DELETE CASCADE,"
                    "[left]            TEXT (5),"
                    "[right]           TEXT (5),"
                    "contur            TEXT (20),"
                    "parenchim         TEXT (20),"
                    "ecogenity         TEXT (30),"
                    "formations        TEXT (300),"
                    "ductsIntrahepatic TEXT (50),"
                    "porta             TEXT (5),"
                    "lienalis          TEXT (5),"
                    "concluzion        TEXT (500),"
                    "recommendation    TEXT (255)"
                    ");");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableLiver()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela \"tableLiver\".") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableCholecist()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists tableCholecist ("
                    "id            INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "id_reportEcho INT NOT Null,"
                    "form          VARCHAR (150),"
                    "dimens        VARCHAR (15),"
                    "walls         VARCHAR (5),"
                    "choledoc      VARCHAR (5),"
                    "formations    VARCHAR (300),"
                    "KEY `tableCholecist_reportEcho_id_idx` (`id_reportEcho`),"
                    "CONSTRAINT `tableCholecist_reportEcho_id` FOREIGN KEY (`id_reportEcho`) REFERENCES `reportEcho` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE tableCholecist ("
                    "id            INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "id_reportEcho INTEGER NOT NULL CONSTRAINT tableCholecist_reportEcho_id REFERENCES reportEcho (id) ON DELETE CASCADE,"
                    "form          TEXT (150),"
                    "dimens        TEXT (15),"
                    "walls         TEXT (5),"
                    "choledoc      TEXT (5),"
                    "formations    TEXT (300)"
                    ");");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableCholecist()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela \"tableCholecist\".") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTablePancreas()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists tablePancreas ("
                    "id            INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "id_reportEcho INT NOT Null,"
                    "cefal         VARCHAR (5),"
                    "corp          VARCHAR (5),"
                    "tail          VARCHAR (5),"
                    "texture       VARCHAR (20),"
                    "ecogency      VARCHAR (30),"
                    "formations    VARCHAR (300),"
                    "KEY `tablePancreas_reportEcho_id_idx` (`id_reportEcho`),"
                    "CONSTRAINT `tablePancreas_reportEcho_id` FOREIGN KEY (`id_reportEcho`) REFERENCES `reportEcho` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE tablePancreas ("
                    "id            INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "id_reportEcho INTEGER NOT NULL CONSTRAINT tablePancreas_reportEcho_id REFERENCES reportEcho (id) ON DELETE CASCADE,"
                    "cefal         TEXT (5),"
                    "corp          TEXT (5),"
                    "tail          TEXT (5),"
                    "texture       TEXT (20),"
                    "ecogency      TEXT (30),"
                    "formations    TEXT (300)"
                    ");");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTablePancreas()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela \"tablePancreas\".") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableSpleen()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists tableSpleen ("
                    "id            INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "id_reportEcho INT NOT Null,"
                    "dimens        VARCHAR (15),"
                    "contur        VARCHAR (20),"
                    "parenchim     VARCHAR (30),"
                    "formations    VARCHAR (300),"
                    "KEY `tableSpleen_reportEcho_id_idx` (`id_reportEcho`),"
                    "CONSTRAINT `tableSpleen_reportEcho_id` FOREIGN KEY (`id_reportEcho`) REFERENCES `reportEcho` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE tableSpleen ("
                    "id            INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "id_reportEcho INTEGER NOT NULL CONSTRAINT tableSpleen_reportEcho_id REFERENCES reportEcho (id) ON DELETE CASCADE,"
                    "dimens        TEXT (15),"
                    "contur        TEXT (20),"
                    "parenchim     TEXT (30),"
                    "formations    TEXT (300)"
                    ");");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableSpleen()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela \"tableSpleen\".") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableKidney()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists tableKidney ("
                    "id                  INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "id_reportEcho       INT NOT Null,"
                    "dimens_right        VARCHAR (15),"
                    "dimens_left         VARCHAR (15),"
                    "corticomed_right    VARCHAR (5),"
                    "corticomed_left     VARCHAR (5),"
                    "pielocaliceal_right VARCHAR (30),"
                    "pielocaliceal_left  VARCHAR (30),"
                    "formations          VARCHAR (500),"
                    "concluzion          VARCHAR (500),"
                    "recommendation      VARCHAR (255),"
                    "KEY `tableKidney_reportEcho_id_idx` (`id_reportEcho`),"
                    "CONSTRAINT `tableKidney_reportEcho_id` FOREIGN KEY (`id_reportEcho`) REFERENCES `reportEcho` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE tableKidney ("
                    "id                  INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "id_reportEcho       INTEGER NOT NULL CONSTRAINT tableKidney_reportEcho_id REFERENCES reportEcho (id) ON DELETE CASCADE,"
                    "dimens_right        TEXT (15),"
                    "dimens_left         TEXT (15),"
                    "corticomed_right    TEXT (5),"
                    "corticomed_left     TEXT (5),"
                    "pielocaliceal_right TEXT (30),"
                    "pielocaliceal_left  TEXT (30),"
                    "formations          TEXT (500),"
                    "concluzion          TEXT (500),"
                    "recommendation      TEXT (255)"
                    ");");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableKidney()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela 'tableKidney'.") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableBladder()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists tableBladder ("
                    "id            INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "id_reportEcho INT NOT Null,"
                    "volum         VARCHAR (5),"
                    "walls         VARCHAR (5),"
                    "formations    VARCHAR (300),"
                    "KEY `tableBladder_reportEcho_id_idx` (`id_reportEcho`),"
                    "CONSTRAINT `tableBladder_reportEcho_id` FOREIGN KEY (`id_reportEcho`) REFERENCES `reportEcho` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE tableBladder ("
                    "id            INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "id_reportEcho INTEGER NOT NULL CONSTRAINT tableBladder_reportEcho_id REFERENCES reportEcho (id) ON DELETE CASCADE,"
                    "volum         TEXT (5),"
                    "walls         TEXT (5),"
                    "formations    TEXT (300)"
                    ");");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableBladder()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela 'tableBladder'.") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableProstate()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists tableProstate ("
                    "id             INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "id_reportEcho  INT NOT Null,"
                    "dimens         VARCHAR (25),"
                    "volume         VARCHAR (5),"
                    "ecostructure   VARCHAR (30),"
                    "contour        VARCHAR (20),"
                    "ecogency       VARCHAR (30),"
                    "formations     VARCHAR (300),"
                    "transrectal    BOOLEAN,"
                    "concluzion     VARCHAR (500),"
                    "recommendation VARCHAR (255),"
                    "KEY `tableProstate_reportEcho_id_idx` (`id_reportEcho`),"
                    "CONSTRAINT `tableProstate_reportEcho_id` FOREIGN KEY (`id_reportEcho`) REFERENCES `reportEcho` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE tableProstate ("
                    "id             INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "id_reportEcho  INTEGER NOT NULL CONSTRAINT tableProstate_reportEcho_id REFERENCES reportEcho (id) ON DELETE CASCADE,"
                    "dimens         TEXT (25),"
                    "volume         TEXT (5),"
                    "ecostructure   TEXT (30),"
                    "contour        TEXT (20),"
                    "ecogency       TEXT (30),"
                    "formations     TEXT (300),"
                    "transrectal    INTEGER NOT NULL,"
                    "concluzion     TEXT (500),"
                    "recommendation TEXT (255)"
                    ");");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableProstate()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela 'tableProstate'.") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableGynecology()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists tableGynecology ("
                    "id                     INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "id_reportEcho          INT NOT Null,"
                    "transvaginal           BOOLEAN,"
                    "dateMenstruation       DATE,"
                    "antecedent             VARCHAR (150),"
                    "uterus_dimens          VARCHAR (25),"
                    "uterus_pozition        VARCHAR (30),"
                    "uterus_ecostructure    VARCHAR (30),"
                    "uterus_formations      VARCHAR (500),"
                    "ecou_dimens            VARCHAR (5),"
                    "ecou_ecostructure      VARCHAR (100),"
                    "cervix_dimens          VARCHAR (25),"
                    "cervix_ecostructure    VARCHAR (100),"
                    "douglas                VARCHAR (100),"
                    "plex_venos             VARCHAR (150),"
                    "ovary_right_dimens     VARCHAR (25),"
                    "ovary_left_dimens      VARCHAR (25),"
                    "ovary_right_volum      VARCHAR (5),"
                    "ovary_left_volum       VARCHAR (5),"
                    "ovary_right_follicle   VARCHAR (100),"
                    "ovary_left_follicle    VARCHAR (100),"
                    "ovary_right_formations VARCHAR (300),"
                    "ovary_left_formations  VARCHAR (300),"
                    "concluzion             VARCHAR (500),"
                    "recommendation         VARCHAR (255),"
                    "KEY `tableGynecology_reportEcho_id_idx` (`id_reportEcho`),"
                    "CONSTRAINT `tableGynecology_reportEcho_id` FOREIGN KEY (`id_reportEcho`) REFERENCES `reportEcho` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE tableGynecology ("
                    "id                     INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "id_reportEcho          INTEGER NOT NULL CONSTRAINT tableGynecology_reportEcho_id REFERENCES reportEcho (id) ON DELETE CASCADE,"
                    "transvaginal           INTEGER,"
                    "dateMenstruation       TEXT (10),"
                    "antecedent             TEXT (150),"
                    "uterus_dimens          TEXT (25),"
                    "uterus_pozition        TEXT (30),"
                    "uterus_ecostructure    TEXT (30),"
                    "uterus_formations      TEXT (500),"
                    "ecou_dimens            TEXT (5),"
                    "ecou_ecostructure      TEXT (100),"
                    "cervix_dimens          TEXT (25),"
                    "cervix_ecostructure    TEXT (100),"
                    "douglas                TEXT (100),"
                    "plex_venos             TEXT (150),"
                    "ovary_right_dimens     TEXT (25),"
                    "ovary_left_dimens      TEXT (25),"
                    "ovary_right_volum      TEXT (5),"
                    "ovary_left_volum       TEXT (5),"
                    "ovary_right_follicle   TEXT (100),"
                    "ovary_left_follicle    TEXT (100),"
                    "ovary_right_formations TEXT (300),"
                    "ovary_left_formations  TEXT (300),"
                    "concluzion             TEXT (500),"
                    "recommendation         TEXT (255));");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableGynecology()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela 'tableGynecology'.") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableBreast()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists tableBreast ("
                    "id                       INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "id_reportEcho            INT NOT Null,"
                    "breast_right_ecostrcture VARCHAR (255),"
                    "breast_right_duct        VARCHAR (20),"
                    "breast_right_ligament    VARCHAR (20),"
                    "breast_right_formations  VARCHAR (500),"
                    "breast_right_ganglions   VARCHAR (300),"
                    "breast_left_ecostrcture  VARCHAR (255),"
                    "breast_left_duct         VARCHAR (20),"
                    "breast_left_ligament     VARCHAR (20),"
                    "breast_left_formations   VARCHAR (500),"
                    "breast_left_ganglions    VARCHAR (300),"
                    "concluzion               VARCHAR (500),"
                    "recommendation           VARCHAR (255),"
                    "KEY `tableBreast_reportEcho_id_idx` (`id_reportEcho`),"
                    "CONSTRAINT `tableBreast_reportEcho_id` FOREIGN KEY (`id_reportEcho`) REFERENCES `reportEcho` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE tableBreast ("
                    "id                       INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "id_reportEcho            INTEGER NOT NULL CONSTRAINT tableBreast_reportEcho_id REFERENCES reportEcho (id) ON DELETE CASCADE,"
                    "breast_right_ecostrcture TEXT (255),"
                    "breast_right_duct        TEXT (20),"
                    "breast_right_ligament    TEXT (20),"
                    "breast_right_formations  TEXT (500),"
                    "breast_right_ganglions   TEXT (300),"
                    "breast_left_ecostrcture  TEXT (255),"
                    "breast_left_duct         TEXT (20),"
                    "breast_left_ligament     TEXT (20),"
                    "breast_left_formations   TEXT (500),"
                    "breast_left_ganglions    TEXT (300),"
                    "concluzion               TEXT (500),"
                    "recommendation           TEXT (255));");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableBreast()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela 'tableBreast'.") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableThyroid()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists tableThyroid ("
                    "id                   INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "id_reportEcho        INT NOT Null,"
                    "thyroid_right_dimens VARCHAR (20),"
                    "thyroid_right_volum  VARCHAR (5),"
                    "thyroid_left_dimens  VARCHAR (20),"
                    "thyroid_left_volum   VARCHAR (5),"
                    "thyroid_istm         VARCHAR (4),"
                    "thyroid_ecostructure VARCHAR (20),"
                    "thyroid_formations   VARCHAR (500),"
                    "thyroid_ganglions    VARCHAR (300),"
                    "concluzion           VARCHAR (500),"
                    "recommendation       VARCHAR (255),"
                    "KEY `tableThyroid_reportEcho_id_idx` (`id_reportEcho`),"
                    "CONSTRAINT `tableThyroid_reportEcho_id` FOREIGN KEY (`id_reportEcho`) REFERENCES `reportEcho` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE tableThyroid ("
                    "id                   INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "id_reportEcho        INTEGER NOT NULL CONSTRAINT tableThyroid_reportEcho_id REFERENCES reportEcho (id) ON DELETE CASCADE,"
                    "thyroid_right_dimens TEXT (20),"
                    "thyroid_right_volum  TEXT (5),"
                    "thyroid_left_dimens  TEXT (20),"
                    "thyroid_left_volum   TEXT (5),"
                    "thyroid_istm         TEXT (4),"
                    "thyroid_ecostructure TEXT (20),"
                    "thyroid_formations   TEXT (500),"
                    "thyroid_ganglions    TEXT (300),"
                    "concluzion           TEXT (500),"
                    "recommendation       TEXT (255));");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableThyroid()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela 'tableThyroid'.") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableGestation0()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists tableGestation0 ("
                    "id               INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "id_reportEcho    INT NOT Null,"
                    "view_examination INT,"
                    "antecedent       VARCHAR (150),"
                    "gestation_age    VARCHAR (20),"
                    "GS               VARCHAR (5),"
                    "GS_age           VARCHAR (20),"
                    "CRL              VARCHAR (5),"
                    "CRL_age          VARCHAR (20),"
                    "BCF              VARCHAR (30),"
                    "liquid_amniotic  VARCHAR (40),"
                    "miometer         VARCHAR (200),"
                    "cervix           VARCHAR (200),"
                    "ovary            VARCHAR (200),"
                    "concluzion       VARCHAR (500),"
                    "recommendation   VARCHAR (255),"
                    "KEY `tableGestation0_reportEcho_id_idx` (`id_reportEcho`),"
                    "CONSTRAINT `tableGestation0_reportEcho_id` FOREIGN KEY (`id_reportEcho`) REFERENCES `reportEcho` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE tableGestation0 ("
                    "id               INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "id_reportEcho    INTEGER NOT NULL CONSTRAINT tableGestation0_reportEcho_id REFERENCES reportEcho (id) ON DELETE CASCADE,"
                    "view_examination INTEGER,"
                    "antecedent       TEXT (150),"
                    "gestation_age    TEXT (20),"
                    "GS               TEXT (5),"
                    "GS_age           TEXT (20),"
                    "CRL              TEXT (5),"
                    "CRL_age          TEXT (20),"
                    "BCF              TEXT (30),"
                    "liquid_amniotic  TEXT (40),"
                    "miometer         TEXT (200),"
                    "cervix           TEXT (200),"
                    "ovary            TEXT (200),"
                    "concluzion       TEXT (500),"
                    "recommendation   TEXT (255));");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableGestation0()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela 'tableGestation0'.") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableGestation1()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists tableGestation1 ("
                    "id                INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "id_reportEcho     INT NOT Null,"
                    "view_examination  INT,"
                    "antecedent        VARCHAR (150),"
                    "gestation_age     VARCHAR (20),"
                    "CRL               VARCHAR (5),"
                    "CRL_age           VARCHAR (20),"
                    "BPD               VARCHAR (5),"
                    "BPD_age           VARCHAR (20),"
                    "NT                VARCHAR (5),"
                    "NT_percent        VARCHAR (20),"
                    "BN                VARCHAR (5),"
                    "BN_percent        VARCHAR (20),"
                    "BCF               VARCHAR (30),"
                    "FL                VARCHAR (5),"
                    "FL_age            VARCHAR (20),"
                    "callote_cranium   VARCHAR (50),"
                    "plex_choroid      VARCHAR (50),"
                    "vertebral_column  VARCHAR (50),"
                    "stomach           VARCHAR (50),"
                    "bladder           VARCHAR (50),"
                    "diaphragm         VARCHAR (50),"
                    "abdominal_wall    VARCHAR (50),"
                    "location_placenta VARCHAR (50),"
                    "sac_vitelin       VARCHAR (50),"
                    "amniotic_liquid   VARCHAR (50),"
                    "miometer          VARCHAR (200),"
                    "cervix            VARCHAR (200),"
                    "ovary             VARCHAR (200),"
                    "concluzion        VARCHAR (500),"
                    "recommendation    VARCHAR (255),"
                    "KEY `tableGestation1_reportEcho_id_idx` (`id_reportEcho`),"
                    "CONSTRAINT `tableGestation1_reportEcho_id` FOREIGN KEY (`id_reportEcho`) REFERENCES `reportEcho` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE tableGestation1 ("
                    "id                INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "id_reportEcho     INTEGER NOT NULL CONSTRAINT tableGestation1_reportEcho_id REFERENCES reportEcho (id) ON DELETE CASCADE,"
                    "view_examination  INTEGER,"
                    "antecedent        TEXT (150),"
                    "gestation_age     TEXT (20),"
                    "CRL               TEXT (5),"
                    "CRL_age           TEXT (20),"
                    "BPD               TEXT (5),"
                    "BPD_age           TEXT (20),"
                    "NT                TEXT (5),"
                    "NT_percent        TEXT (20),"
                    "BN                TEXT (5),"
                    "BN_percent        TEXT (20),"
                    "BCF               TEXT (30),"
                    "FL                TEXT (5),"
                    "FL_age            TEXT (20),"
                    "callote_cranium   TEXT (50),"
                    "plex_choroid      TEXT (50),"
                    "vertebral_column  TEXT (50),"
                    "stomach           TEXT (50),"
                    "bladder           TEXT (50),"
                    "diaphragm         TEXT (50),"
                    "abdominal_wall    TEXT (50),"
                    "location_placenta TEXT (50),"
                    "sac_vitelin       TEXT (50),"
                    "amniotic_liquid   TEXT (50),"
                    "miometer          TEXT (200),"
                    "cervix            TEXT (200),"
                    "ovary             TEXT (200),"
                    "concluzion        TEXT (500),"
                    "recommendation    TEXT (255));");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableGestation1()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela 'tableGestation1'.") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableRegistrationPatients()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists registrationPatients ("
                    "id                 INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "dateDoc            DATE NOT NULL,"
                    "time_registration  INT,"
                    "execute            BOOLEAN,"
                    "dataPatient        VARCHAR (200) NOT Null,"
                    "name_organizations VARCHAR (150),"
                    "id_organizations   INT,"
                    "name_doctors       VARCHAR (150),"
                    "id_doctors         INT,"
                    "investigations     VARCHAR (150),"
                    "comment            VARCHAR (255),"
                    "KEY `registrationPatients_organizations_id_idx` (`id_organizations`),"
                    "KEY `registrationPatients_doctors_id_idx` (`id_doctors`),"
                    "CONSTRAINT `registrationPatients_organizations_id` FOREIGN KEY (`id_organizations`) REFERENCES `organizations` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT,"
                    "CONSTRAINT `registrationPatients_doctors_id` FOREIGN KEY (`id_doctors`) REFERENCES `doctors` (`id`) ON DELETE SET NULL ON UPDATE RESTRICT"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE if not exists registrationPatients ("
                    "id                 INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "dateDoc            TEXT (10) NOT NULL,"
                    "time_registration  INTEGER,"
                    "execute            INT,"
                    "dataPatient        TEXT (200) NOT NULL,"
                    "name_organizations TEXT (150),"
                    "id_organizations   INT CONSTRAINT registrationPatients_organizations_id REFERENCES organizations (id) ON DELETE RESTRICT,"
                    "name_doctors       TEXT (150),"
                    "id_doctors         INT CONSTRAINT registrationPatients_doctors_id REFERENCES doctors (id) ON DELETE RESTRICT,"
                    "investigations     TEXT (150),"
                    "comment            TEXT (255));");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableRegistrationPatients()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela \"registrationPatients\".") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableSettingsForm()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists settingsForms ("
                    "id               INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "id_users         INT NOT Null,"
                    "typeForm         VARCHAR (20) NOT Null,"
                    "numberSection    INT,"
                    "sizeSection      INT,"
                    "directionSorting INT,"
                    "dateStart        DATETIME,"
                    "dateEnd          DATETIME,"
                    "id_organizations INT,"
                    "id_contracts     INT,"
                    "id_doctors       INT,"
                    "id_nurses        INT,"
                    "KEY `settingsForms_users_id_idx` (`id_users`),"
                    "KEY `settingsForms_organizations_id_idx` (`id_organizations`),"
                    "KEY `settingsForms_contracts_id_idx` (`id_contracts`),"
                    "KEY `settingsForms_doctors_id_idx` (`id_doctors`),"
                    "KEY `settingsForms_nurses_id_idx` (`id_nurses`),"
                    "CONSTRAINT `settingsForms_users_id` FOREIGN KEY (`id_users`) REFERENCES `users` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT,"
                    "CONSTRAINT `settingsForms_organizations_id` FOREIGN KEY (`id_organizations`) REFERENCES `organizations` (`id`) ON DELETE SET NULL,"
                    "CONSTRAINT `settingsForms_contracts_id` FOREIGN KEY (`id_contracts`) REFERENCES `contracts` (`id`) ON DELETE SET NULL,"
                    "CONSTRAINT `settingsForms_doctors_id` FOREIGN KEY (`id_doctors`) REFERENCES `doctors` (`id`) ON DELETE SET NULL,"
                    "CONSTRAINT `settingsForms_nurses_id` FOREIGN KEY (`id_nurses`) REFERENCES `nurses` (`id`) ON DELETE SET NULL"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE settingsForms ("
                    "id               INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                    "id_users         INT     NOT NULL CONSTRAINT settingsForms_users_id REFERENCES users (id) ON DELETE CASCADE,"
                    "typeForm         TEXT (20) NOT NULL,"
                    "numberSection    INT,"
                    "sizeSection      INT,"
                    "directionSorting INT,"
                    "dateStart        TEXT (19),"
                    "dateEnd          TEXT (19),"
                    "id_organizations INT       CONSTRAINT settingsForms_organizations_id REFERENCES organizations (id) ON DELETE SET NULL,"
                    "id_contracts     INT       CONSTRAINT settingsForms_contracts_id REFERENCES contracts (id) ON DELETE SET NULL,"
                    "id_doctors       INT       CONSTRAINT settingsForms_doctors_id REFERENCES doctors (id) ON DELETE SET NULL,"
                    "id_nurses        INT       CONSTRAINT settingsForms_nurses_id REFERENCES nurses (id) ON DELETE SET NULL"
                    ");");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableSettingsForm()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela 'settingsForm'.") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableSettingsReports()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists settingsReports ("
                    "id                   INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "nameReport           VARCHAR (150) NOT Null,"
                    "dateStart            DATETIME NOT Null,"
                    "dateEnd              DATETIME NOT Null,"
                    "id_organizations     INT,"
                    "id_contracts         INT,"
                    "id_doctors           INT,"
                    "showOnLaunch         BOOLEAN,"
                    "hideLogo             BOOLEAN,"
                    "hideTitle            BOOLEAN,"
                    "hideDataOrganization BOOLEAN,"
                    "hideFooter           BOOLEAN,"
                    "hideSignatureStamped BOOLEAN,"
                    "hidePrice            BOOLEAN,"
                    "KEY `settingsReports_organizations_id_idx` (`id_organizations`),"
                    "KEY `settingsReports_contracts_id_idx` (`id_contracts`),"
                    "KEY `settingsReports_doctors_id_idx` (`id_doctors`),"
                    "CONSTRAINT `settingsReports_organizations_id` FOREIGN KEY (`id_organizations`) REFERENCES `organizations` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT,"
                    "CONSTRAINT `settingsReports_contracts_id` FOREIGN KEY (`id_contracts`) REFERENCES `contracts` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT,"
                    "CONSTRAINT `settingsReports_doctors_id` FOREIGN KEY (`id_doctors`) REFERENCES `doctors` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE settingsReports ("
                    "id                   INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "nameReport           TEXT (150) NOT NULL,"
                    "dateStart            TEXT (19) NOT NULL,"
                    "dateEnd              TEXT (19) NOT NULL,"
                    "id_organizations     INT CONSTRAINT settingsReports_organizations_id REFERENCES organizations (id),"
                    "id_contracts         INT CONSTRAINT settingsReports_contracts_id REFERENCES contracts (id),"
                    "id_doctors           INT CONSTRAINT settingsReports_doctors_id REFERENCES doctors (id),"
                    "showOnLaunch         INT,"
                    "hideLogo             INT,"
                    "hideTitle            INT,"
                    "hideDataOrganization INT,"
                    "hideFooter           INT,"
                    "hideSignatureStamped INT,"
                    "hidePrice            INT);");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableSettingsReports()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela 'settingsReports'.") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableSettingsUsers()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE if not exists settingsUsers ("
                    "id                    INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "id_users              INT NOT Null,"
                    "owner                 VARCHAR (50),"
                    "nameOption            VARCHAR (150),"
                    "versionApp            VARCHAR (8),"
                    "showQuestionCloseApp  BOOLEAN,"
                    "showUserManual        BOOLEAN,"
                    "showHistoryVersion    BOOLEAN,"
                    "order_splitFullName   BOOLEAN,"
                    "updateListDoc         VARCHAR (3),"
                    "showDesignerMenuPrint BOOLEAN,"
                    "KEY `settingsUsers_users_id_idx` (`id_users`),"
                    "CONSTRAINT `settingsUsers_users_id` FOREIGN KEY (`id_users`) REFERENCES `users` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE settingsUsers ("
                    "id                    INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "id_users              INT NOT NULL CONSTRAINT settingsUsers_users_id REFERENCES users (id) ON DELETE CASCADE,"
                    "owner                 TEXT (50),"
                    "nameOption            TEXT (150),"
                    "versionApp            TEXT (8),"
                    "showQuestionCloseApp  INT,"
                    "showUserManual        INT,"
                    "showHistoryVersion    INT,"
                    "order_splitFullName   INT,"
                    "updateListDoc         TEXT (3),"
                    "showDesignerMenuPrint INT"
                    ");");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableSettingsUsers()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela 'settingsUsers'.") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableUserPreferences()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE userPreferences ("
                    "id                    INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "id_users              INT NOT Null,"
                    "versionApp            VARCHAR (8),"
                    "showQuestionCloseApp  BOOLEAN,"
                    "showUserManual        BOOLEAN,"
                    "showHistoryVersion    BOOLEAN,"
                    "order_splitFullName   BOOLEAN,"
                    "updateListDoc         VARCHAR (3),"
                    "showDesignerMenuPrint BOOLEAN,"
                    "checkNewVersionApp    BOOLEAN,"
                    "databasesArchiving    BOOLEAN,"
                    "showAsistantHelper    BOOLEAN,"
                    "KEY `userPreferences_users_id_idx` (`id_users`),"
                    "CONSTRAINT `userPreferences_users_id` FOREIGN KEY (`id_users`) REFERENCES `users` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE userPreferences ("
                    "id                    INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "id_users              INT NOT NULL CONSTRAINT userPreferences_users_id REFERENCES users (id) ON DELETE CASCADE,"
                    "versionApp            TEXT (8),"
                    "showQuestionCloseApp  INT,"
                    "showUserManual        INT,"
                    "showHistoryVersion    INT,"
                    "order_splitFullName   INT,"
                    "updateListDoc         TEXT (3),"
                    "showDesignerMenuPrint INT,"
                    "checkNewVersionApp    INT,"
                    "databasesArchiving    INT,"
                    "showAsistantHelper    INT"
                    ");");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableUserPreferences()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela 'userPreferences'.") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableConclusionTemplates()
{
    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE conclusionTemplates ("
                    "id           INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                    "deletionMark INT NOT Null,"
                    "cod          VARCHAR (10) NOT Null, "
                    "name         VARCHAR (500) NOT Null,"
                    "`system`     VARCHAR (150)"
                    ");");
    else if (globals::connectionMade == "Sqlite")
        qry.prepare("CREATE TABLE conclusionTemplates ("
                    "id           INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "deletionMark INT NOT NULL,"
                    "cod          TEXT (10) NOT NULL, "
                    "name         TEXT (500) NOT NULL,"
                    "system       TEXT (150)"
                    ");");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableConclusionTemplates()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela 'conclusionTemplates'.") + qry.lastError().text();
        return false;
    }
}

bool DataBase::createTableImagesReports()
{
    if (globals::connectionMade == "Sqlite" || globals::connectionMade == nullptr)
        return false;

    QSqlQuery qry;
    if (globals::connectionMade == "MySQL")
        qry.prepare("CREATE TABLE imagesReports ("
                    "id            INT NOT NULL PRIMARY KEY AUTO_INCREMENT,"
                    "id_reportEcho INT NOT NULL,"
                    "id_orderEcho  INT NOT NULL,"
                    "id_patients   INT NOT NULL,"
                    "image_1       LONGBLOB,"
                    "image_2       LONGBLOB,"
                    "image_3       LONGBLOB,"
                    "image_4       LONGBLOB,"
                    "image_5       LONGBLOB,"
                    "comment_1     VARCHAR (150),"
                    "comment_2     VARCHAR (150),"
                    "comment_3     VARCHAR (150),"
                    "comment_4     VARCHAR (150),"
                    "comment_5     VARCHAR (150),"
                    "id_user       INT NOT NULL,"
                    "KEY `imagesReports_reportEcho_id_idx` (`id_reportEcho`),"
                    "KEY `imagesReports_orderEcho_id_idx` (`id_orderEcho`),"
                    "KEY `imagesReports_patients_id_idx` (`id_patients`),"
                    "KEY `imagesReports_user_id_idx` (`id_user`),"
                    "CONSTRAINT `imagesReports_reportEcho_id` FOREIGN KEY (`id_reportEcho`) REFERENCES `reportEcho` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT,"
                    "CONSTRAINT `imagesReports_orderEcho_id` FOREIGN KEY (`id_orderEcho`) REFERENCES `orderEcho` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT,"
                    "CONSTRAINT `imagesReports_patients_id` FOREIGN KEY (`id_patients`) REFERENCES `pacients` (`id`),"
                    "CONSTRAINT `imagesReports_user_id_idx` FOREIGN KEY (`id_user`) REFERENCES `users` (`id`)"
                    ");");
    else
        return false;

    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - createTableImagesReports()").arg(metaObject()->className())
                               << tr("Nu a fost creata tabela 'imagesReports'.") + qry.lastError().text();
        return false;
    }
}

void DataBase::updateVariableFromTableSettingsUser()
{
    QSqlQuery qry;
    qry.prepare("SELECT "
                "showQuestionCloseApp,"
                "showUserManual,"
                "showHistoryVersion,"
                "order_splitFullName,"
                "updateListDoc,"
                "showDesignerMenuPrint,"
                "checkNewVersionApp,"
                "databasesArchiving,"
                "showAsistantHelper FROM userPreferences WHERE id_users = :id_users;");
    qry.bindValue(":id_users", globals::idUserApp);
    if (qry.exec() && qry.next()){
        QSqlRecord rec = qry.record();
        globals::showQuestionCloseApp  = qry.value(rec.indexOf("showQuestionCloseApp")).toBool();
        globals::showUserManual        = qry.value(rec.indexOf("showUserManual")).toBool();
        globals::showHistoryVersion    = qry.value(rec.indexOf("showHistoryVersion")).toBool();
        globals::order_splitFullName   = qry.value(rec.indexOf("order_splitFullName")).toBool();
        globals::updateIntervalListDoc = qry.value(rec.indexOf("updateListDoc")).toInt();
        globals::showDesignerMenuPrint = qry.value(rec.indexOf("showDesignerMenuPrint")).toBool();
        globals::checkNewVersionApp    = qry.value(rec.indexOf("checkNewVersionApp")).toBool();
        globals::databasesArchiving    = qry.value(rec.indexOf("databasesArchiving")).toBool();
        globals::showAsistantHelper    = qry.value(rec.indexOf("showAsistantHelper")).toBool();
    }
}

// *******************************************************************
// ***************** FUNTIILE SUPLIMENTARE ***************************

void DataBase::getNameColumnTable(const QString nameTable/*,QMap<QString, QString> &items*/)
{
    QSqlQuery qry;
    qry.prepare("PRAGMA table_info('" + nameTable + "');");
    qry.exec();

    //    int n = -1;
    while (qry.next()){
        //        n += 1;
        qInfo(logInfo()) << "Name column: " + qry.value(1).toString()
                         << "Type column: " + qry.value(2).toString();
        //        items.insert(qry.value(1).toString(), qry.value(2).toString());
    }
}

int DataBase::getCountSectionTableSQLite(const QString nameTable) const
{
    QSqlQuery qry;
    qry.prepare("PRAGMA table_info('" + nameTable + "');");

    int result = 0;
    if (qry.exec()){
        if (qry.last()){
            result = qry.at() + 1;
        }
    }
    return result;
}

QMap<int, QString> DataBase::getMapDataQuery(const QString strQuery)
{
    QMap<int, QString> data;

    QSqlQuery qry;
    qry.prepare(strQuery);
    if(qry.exec()){
        while (qry.next()){
            data[qry.value(0).toInt()] = qry.value(1).toString();
        }
    } else {
        qWarning(logWarning()) << tr("%1 - getMapDataQuery()").arg(metaObject()->className())
                               << tr("Solicitarea nereusita - %1").arg(qry.lastError().text());
    }
    return data;
}

// *******************************************************************
// *************** SOLICITARI PU DOCUMENTE****************************

bool DataBase::existIdDocument(const QString nameTable, const QString name_condition, const QString value_condition, QSqlDatabase nameDatabase)
{
    bool exist_id = false;
    QSqlQuery qry(nameDatabase);
    qry.prepare(QString("SELECT count(id) FROM " + nameTable + " WHERE " + name_condition + " = :value;"));
    qry.bindValue(":value", value_condition);
    if (qry.exec()){
        qry.next();
        if (qry.value(0).toInt() > 0)
            exist_id = true;
    } else {
        qWarning(logWarning()) << tr("Eroare determinarii existentii 'count(id)' a documentului cu 'id=%1' din tabela 'DB_IMAGE':\n").arg(value_condition) << qry.lastError();
    }

    return exist_id;
}

bool DataBase::existSubalternDocument(const QString nameTable, const QString name_condition, const QString value_condition, int &id_doc)
{
    QSqlQuery qry;
    qry.prepare("SELECT id  FROM " + nameTable + " WHERE " + name_condition + " = :value ;");
    qry.bindValue(":value", value_condition);
    if (qry.exec()){
        qry.next();
        if (qry.value(0).toInt() > 0){
            id_doc = qry.value(0).toInt();
            return id_doc > 0;
        } else {
            return false;
        }
    } else {
        qWarning(logWarning()) << tr("%1 - existSubalternDocument()").arg(metaObject()->className())
                               << tr("Solicitarea nereusita: %1").arg(qry.lastError().text());
        return false;
    }
}

QString DataBase::getQryFromTableConstantById(const int id_user) const
{
    return QString("SELECT constants.id_organizations, "
                   "  organizations.IDNP, "
                   "  organizations.name, "
                   "  organizations.address, "
                   "  organizations.telephone, "
                   "  fullNameDoctors.nameAbbreviated AS doctor,"
                   "  organizations.email FROM constants "
                   "INNER JOIN "
                   "  organizations ON constants.id_organizations = organizations.id "
                   "INNER JOIN "
                   "  fullNameDoctors ON constants.id_doctors = fullNameDoctors.id_doctors "
                   "WHERE "
                   "  constants.id_users = %1;").arg(QString::number(id_user));
}

QString DataBase::getQryFromTablePatientById(const int id_patient) const
{
    if (globals::thisMySQL)
        return QString("SELECT CONCAT(pacients.name,' ', pacients.fName) AS FullName,"
                       "  CONCAT(SUBSTRING(pacients.birthday, 9, 2) ,'.', SUBSTRING(pacients.birthday, 6, 2) ,'.', SUBSTRING(pacients.birthday, 1, 4)) AS birthday,"
                       "  pacients.IDNP,"
                       "  pacients.medicalPolicy,"
                       "  pacients.address "
                       "FROM "
                       "  pacients "
                       "WHERE "
                       "  id = '%1' AND deletionMark = '0';").arg(QString::number(id_patient));
    else
        return QString("SELECT pacients.name ||' '|| pacients.fName AS FullName,"
                       "  substr(pacients.birthday, 9, 2) ||'.'|| substr(pacients.birthday, 6, 2) ||'.'|| substr(pacients.birthday, 1, 4) AS birthday,"
                       "  pacients.IDNP,"
                       "  pacients.medicalPolicy,"
                       "  pacients.address "
                       "FROM "
                       "  pacients "
                       "WHERE "
                       "  id = '%1' AND deletionMark = '0';").arg(QString::number(id_patient));
}

QString DataBase::getQryForTableOrgansInternalById(const int id_doc) const
{
    return QString("SELECT "
                   "  reportEchoPresentation.docPresentationDate AS title_report,"
                   "  %1 AS liver_left,"
                   "  %2 AS liver_right,"
                   "  tableLiver.contur AS liver_contur,"
                   "  tableLiver.parenchim AS liver_parenchim,"
                   "  tableLiver.ecogenity AS liver_ecogenity,"
                   "  tableLiver.formations AS liver_formations,"
                   "  tableLiver.ductsIntrahepatic AS liver_duct_hepatic,"
                   "  tableLiver.porta AS liver_porta,"
                   "  tableLiver.lienalis AS liver_lienalis,"
                   "  tableCholecist.form AS cholecist_form,"
                   "  tableCholecist.dimens AS cholecist_dimens,"
                   "  tableCholecist.walls AS cholecist_walls,"
                   "  tableCholecist.formations AS cholecist_formations,"
                   "  tableCholecist.choledoc AS cholecist_choledoc,"
                   "  tablePancreas.cefal AS pancreas_cefal,"
                   "  tablePancreas.corp AS pancreas_corp,"
                   "  tablePancreas.tail AS pancreas_tail,"
                   "  tablePancreas.ecogency AS pancreas_ecogenity,"
                   "  tablePancreas.texture AS pancreas_parenchim,"
                   "  tablePancreas.formations AS pancreas_formations,"
                   "  tableSpleen.dimens AS spleen_dimens,"
                   "  tableSpleen.contur AS spleen_contur,"
                   "  tableSpleen.parenchim AS spleen_parenchim,"
                   "  tableSpleen.formations AS spleen_formations,"
                   "  tableLiver.concluzion AS organs_internal_concluzion,"
                   "  tableLiver.recommendation AS recommendation,"
                   "  reportEcho.concluzion AS concluzion "
                   "FROM "
                   "  reportEcho "
                   "INNER JOIN "
                   "  reportEchoPresentation ON reportEcho.id = reportEchoPresentation.id_reportEcho "
                   "INNER JOIN "
                   "  tableLiver ON reportEcho.id = tableLiver.id_reportEcho "
                   "INNER JOIN "
                   "  tableCholecist ON reportEcho.id = tableCholecist.id_reportEcho "
                   "INNER JOIN "
                   "  tablePancreas ON reportEcho.id = tablePancreas.id_reportEcho "
                   "INNER JOIN "
                   "  tableSpleen ON reportEcho.id = tableSpleen.id_reportEcho "
                   "WHERE "
                   "  reportEcho.deletionMark = '2' AND "
                   "  reportEcho.id = '%3';")
        .arg((globals::thisMySQL) ? "tableLiver.left" : "tableLiver.[left]",
             (globals::thisMySQL) ? "tableLiver.right" : "tableLiver.[right]",
             QString::number(id_doc));
}

QString DataBase::getQryForTableUrinarySystemById(const int id_doc) const
{
    return QString("SELECT "
                   "      reportEchoPresentation.docPresentationDate AS title_report,"
                   "      tableKidney.dimens_right,"
                   "      tableKidney.dimens_left,"
                   "      tableKidney.corticomed_right,"
                   "      tableKidney.corticomed_left,"
                   "      tableKidney.pielocaliceal_right,"
                   "      tableKidney.pielocaliceal_left,"
                   "      tableKidney.formations AS kidney_formations,"
                   "      tableBladder.volum AS bladder_volum,"
                   "      tableBladder.walls AS bladder_walls,"
                   "      tableBladder.formations AS bladder_formations,"
                   "      tableKidney.concluzion AS urinary_system_concluzion,"
                   "      tableKidney.recommendation AS recommendation,"
                   "      reportEcho.concluzion "
                   "FROM reportEcho "
                   "  INNER JOIN reportEchoPresentation ON reportEcho.id = reportEchoPresentation.id_reportEcho "
                   "  INNER JOIN tableKidney ON reportEcho.id = tableKidney.id_reportEcho "
                   "  INNER JOIN tableBladder ON reportEcho.id = tableBladder.id_reportEcho "
                   "WHERE reportEcho.deletionMark = '2' AND reportEcho.id = '%1';").arg(QString::number(id_doc));
}

QString DataBase::getQryForTableProstateById(const int id_doc) const
{
    return QString("SELECT "
                   "      reportEchoPresentation.docPresentationDate AS title_report,"
                   "      tableProstate.dimens AS prostate_dimens,"
                   "      tableProstate.volume AS prostate_volume,"
                   "      tableProstate.ecostructure AS prostate_ecostructure,"
                   "      tableProstate.contour AS prostate_contur,"
                   "      tableProstate.ecogency AS prostate_ecogency,"
                   "      tableProstate.formations AS prostate_formations,"
                   "      tableProstate.concluzion AS prostate_concluzion,"
                   "      tableProstate.recommendation AS prostate_recommendation,"
                   "      reportEcho.concluzion "
                   "  FROM reportEcho "
                   "  INNER JOIN reportEchoPresentation ON reportEcho.id = reportEchoPresentation.id_reportEcho "
                   "  INNER JOIN tableProstate ON reportEcho.id = tableProstate.id_reportEcho "
                   "WHERE reportEcho.deletionMark = '2' AND reportEcho.id = '%1';").arg(QString::number(id_doc));
}

QString DataBase::getQryForTableGynecologyById(const int id_doc) const
{
    return QString("SELECT "
                   "  reportEchoPresentation.docPresentationDate AS title_report,"
                   "  tableGynecology.transvaginal,"
                   "  %1 AS dateMenstruation,"
                   "  tableGynecology.antecedent,"
                   "  tableGynecology.uterus_dimens,"
                   "  tableGynecology.uterus_pozition,"
                   "  tableGynecology.uterus_ecostructure,"
                   "  tableGynecology.uterus_formations,"
                   "  tableGynecology.ecou_dimens,"
                   "  tableGynecology.ecou_ecostructure,"
                   "  tableGynecology.cervix_dimens,"
                   "  tableGynecology.cervix_ecostructure,"
                   "  tableGynecology.douglas,"
                   "  tableGynecology.plex_venos,"
                   "  tableGynecology.ovary_right_dimens,"
                   "  tableGynecology.ovary_left_dimens,"
                   "  tableGynecology.ovary_right_volum,"
                   "  tableGynecology.ovary_left_volum,"
                   "  tableGynecology.ovary_right_follicle,"
                   "  tableGynecology.ovary_left_follicle,"
                   "  tableGynecology.ovary_right_formations,"
                   "  tableGynecology.ovary_left_formations,"
                   "  tableGynecology.concluzion AS gynecology_concluzion,"
                   "  tableGynecology.recommendation AS gynecology_recommendation,"
                   "  reportEcho.concluzion "
                   " FROM reportEcho "
                   "  INNER JOIN reportEchoPresentation ON reportEcho.id = reportEchoPresentation.id_reportEcho "
                   "  INNER JOIN tableGynecology ON reportEcho.id = tableGynecology.id_reportEcho "
                   "WHERE reportEcho.deletionMark = '2' AND reportEcho.id = " + QString::number(id_doc) + ";")
        .arg((globals::thisMySQL) ?
                 "CONCAT(SUBSTRING(tableGynecology.dateMenstruation, 9, 2) , '.' , SUBSTRING(tableGynecology.dateMenstruation, 6, 2) , '.' , SUBSTRING(tableGynecology.dateMenstruation, 1, 4))" :
                 "substr(tableGynecology.dateMenstruation, 9, 2) || '.' || substr(tableGynecology.dateMenstruation, 6, 2) || '.' || substr(tableGynecology.dateMenstruation, 1, 4)");
}

QString DataBase::getQryForTableBreastById(const int id_doc) const
{
    return QString("SELECT "
                   "  reportEchoPresentation.docPresentationDate AS title_report,"
                   "  tableBreast.breast_right_ecostrcture,"
                   "  tableBreast.breast_right_duct,"
                   "  tableBreast.breast_right_ligament,"
                   "  tableBreast.breast_right_formations,"
                   "  tableBreast.breast_right_ganglions,"
                   "  tableBreast.breast_left_ecostrcture,"
                   "  tableBreast.breast_left_duct,"
                   "  tableBreast.breast_left_ligament,"
                   "  tableBreast.breast_left_formations,"
                   "  tableBreast.breast_left_ganglions,"
                   "  tableBreast.concluzion AS breast_concluzion,"
                   "  tableBreast.recommendation AS recommendation,"
                   "  reportEcho.concluzion "
                   "FROM reportEcho "
                   "  INNER JOIN reportEchoPresentation ON reportEcho.id = reportEchoPresentation.id_reportEcho "
                   "  INNER JOIN tableBreast ON reportEcho.id = tableBreast.id_reportEcho "
                   "WHERE reportEcho.deletionMark = '2' AND reportEcho.id = '%1';").arg(QString::number(id_doc));
}

QString DataBase::getQryForTableThyroidById(const int id_doc) const
{
    return QString("SELECT "
                   "  reportEchoPresentation.docPresentationDate AS title_report,"
                   "  tableThyroid.thyroid_right_dimens,"
                   "  tableThyroid.thyroid_right_volum,"
                   "  tableThyroid.thyroid_left_dimens,"
                   "  tableThyroid.thyroid_left_volum,"
                   "  tableThyroid.thyroid_istm,"
                   "  tableThyroid.thyroid_ecostructure,"
                   "  tableThyroid.thyroid_formations,"
                   "  tableThyroid.thyroid_ganglions,"
                   "  tableThyroid.concluzion AS thyroid_concluzion,"
                   "  tableThyroid.recommendation AS thyroid_recommendation,"
                   "  reportEcho.concluzion "
                   "FROM reportEcho "
                   "  INNER JOIN reportEchoPresentation ON reportEcho.id = reportEchoPresentation.id_reportEcho "
                   "  INNER JOIN tableThyroid ON reportEcho.id = tableThyroid.id_reportEcho "
                   "WHERE reportEcho.deletionMark = '2' AND reportEcho.id = '%1';").arg(QString::number(id_doc));
}

QString DataBase::getQryForTableGestation0dById(const int id_doc) const
{
    QString str_qry;
    str_qry =  QString("SELECT "
                      "  reportEchoPresentation.docPresentationDate AS title_report,"
                      "  tableGestation0.antecedent,"
                      "  tableGestation0.gestation_age,"
                      "  tableGestation0.GS,"
                      "  tableGestation0.GS_age,"
                      "  tableGestation0.CRL,"
                      "  tableGestation0.CRL_age,"
                      "  tableGestation0.BCF,"
                      "  tableGestation0.liquid_amniotic,"
                      "  tableGestation0.miometer,"
                      "  tableGestation0.cervix,"
                      "  tableGestation0.ovary,"
                      "  tableGestation0.concluzion AS gestation0_concluzion,"
                      "  tableGestation0.recommendation AS gestation0_recommendation,"
                      "  reportEcho.concluzion "
                      "FROM reportEcho "
                      "  INNER JOIN reportEchoPresentation ON reportEcho.id = reportEchoPresentation.id_reportEcho "
                      "  INNER JOIN tableGestation0 ON reportEcho.id = tableGestation0.id_reportEcho "
                      "WHERE reportEcho.deletionMark = '2' AND reportEcho.id = '%1';").arg(id_doc);

    return str_qry;
}

QString DataBase::getQryForTableGestation1dById(const int id_doc) const
{
    QString str_qry;
    str_qry = QString("SELECT "
                      "  reportEchoPresentation.docPresentationDate AS title_report,"
                      "  tableGestation1.antecedent,"
                      "  tableGestation1.gestation_age,"
                      "  tableGestation1.CRL,"
                      "  tableGestation1.CRL_age,"
                      "  tableGestation1.BPD,"
                      "  tableGestation1.BPD_age,"
                      "  tableGestation1.NT,"
                      "  tableGestation1.NT_percent,"
                      "  tableGestation1.BN,"
                      "  tableGestation1.BN_percent,"
                      "  tableGestation1.BCF,"
                      "  tableGestation1.FL,"
                      "  tableGestation1.FL_age,"
                      "  tableGestation1.callote_cranium,"
                      "  tableGestation1.plex_choroid,"
                      "  tableGestation1.vertebral_column,"
                      "  tableGestation1.stomach,"
                      "  tableGestation1.bladder,"
                      "  tableGestation1.diaphragm,"
                      "  tableGestation1.abdominal_wall,"
                      "  tableGestation1.location_placenta,"
                      "  tableGestation1.sac_vitelin,"
                      "  tableGestation1.amniotic_liquid,"
                      "  tableGestation1.miometer,"
                      "  tableGestation1.cervix,"
                      "  tableGestation1.ovary,"
                      "  tableGestation1.concluzion AS gestation1_concluzion,"
                      "  tableGestation1.recommendation AS gestation1_recommendation,"
                      "  reportEcho.concluzion "
                      "FROM reportEcho "
                      "  INNER JOIN reportEchoPresentation ON reportEcho.id = reportEchoPresentation.id_reportEcho "
                      "  INNER JOIN tableGestation1 ON reportEcho.id = tableGestation1.id_reportEcho "
                      "WHERE reportEcho.deletionMark = '2' AND reportEcho.id = '%1';").arg(id_doc);

    return str_qry;
}

QString DataBase::getQryForTableOrderById(const int id_doc, const QString str_price) const
{
    QString str_qry;
    str_qry = QString("SELECT "
                      "orderEchoPresentation.docPresentationDate AS title_order,"
                      "orderEchoTable.cod,"
                      "orderEchoTable.name AS Investigation, "
                      "%1 AS Price "
                      "FROM orderEcho "
                      "  INNER JOIN orderEchoTable ON orderEcho.id = orderEchoTable.id_orderEcho "
                      "  INNER JOIN orderEchoPresentation ON orderEcho.id = orderEchoPresentation.id_orderEcho "
                      "WHERE orderEcho.id = '%2' AND orderEcho.deletionMark = '2' GROUP BY orderEchoTable.cod;")
                  .arg(str_price, QString::number(id_doc));
    return str_qry;
}

// *******************************************************************
// ****** ARHIVAREA BAZEI DE DATE SI A FISIERELOR APLICATIEI *********

QByteArray DataBase::fileChecksum(const QString &fileName, QCryptographicHash::Algorithm hashAlgorithm)
{
    QFile f(fileName);
    if (f.open(QFile::ReadOnly)) {
        QCryptographicHash hash(hashAlgorithm);
        if (hash.addData(&f)) {
            return hash.result();
        }
    }
    return QByteArray();
}

QByteArray DataBase::getOutByteArrayImage(const QString nameTable, const QString selectedColumn, const QString whereColumn, const int _id)
{
    QSqlQuery qry;

    qry.prepare(QString("SELECT %1 FROM %2 WHERE %3 = '%4';").arg(selectedColumn, nameTable, whereColumn, QString::number(_id)));
    if (!qry.exec())
        qDebug() << "Error getting image from table:\n" << qry.lastError();
    qry.first();
    return QByteArray::fromBase64(qry.value(0).toByteArray());
}

QByteArray DataBase::getOutByteArrayImageByDatabase(const QString nameTable, const QString selectedColumn, const QString whereColumn, const int _id, QSqlDatabase m_base)
{
    QSqlQuery qry(m_base);

    qry.prepare(QString("SELECT %1 FROM %2 WHERE %3 = '%4';").arg(selectedColumn, nameTable, whereColumn, QString::number(_id)));
    if (!qry.exec())
        qDebug() << "Error getting image from table:\n" << qry.lastError();
    qry.first();

    return QByteArray::fromBase64(qry.value(0).toByteArray());
}

// *******************************************************************
// ************************* CRYPTAREA *******************************

QString DataBase::encode_string(const QString &str)
{
    QByteArray arr(str.toUtf8());
    for(int i =0; i<arr.size(); i++)
        arr[i] = arr[i] ^ key_encoding;

    return QString::fromLatin1(arr.toBase64());
}

QString DataBase::decode_string(const QString &str)
{
    QByteArray arr = QByteArray::fromBase64(str.toLatin1());
    for(int i =0; i<arr.size(); i++)
        arr[i] =arr[i] ^ key_encoding;

    return QString::fromUtf8(arr);
}

QString DataBase::getVersionSQLite()
{
    QString str = "unknow";
    QSqlQuery qry;
    qry.exec("select sqlite_version();");
    while (qry.next()){
        str = qry.value(0).toString();
    }
    return str;
}

QString DataBase::getVersionMySQL()
{
    QString str = "unknow";
    QSqlQuery qry;
    qry.exec("SELECT VERSION();");
    while (qry.next()){
        str = qry.value(0).toString();
    }
    return str;
}

QString DataBase::getHTMLImageInfo()
{
    return QString("<img src = \"qrc:///img/info_x32.png\" alt = \"info\" width=\"20\" height=\"20\" align=\"left\" />");
}

QString DataBase::getHTMLImageWarning()
{
    return QString("<img src = \"qrc:///img/warning.png\" alt = \"info\" width=\"20\" height=\"20\" align=\"left\" />");
}

// *******************************************************************
// ******** FUNCTIILE PRIVATE DE CONECTARE LA BD *********************

bool DataBase::openDataBase()
{
    if (globals::connectionMade == "MySQL"){

        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
        db.setHostName(globals::mySQLhost);         // localhost
        db.setDatabaseName(globals::mySQLnameBase); // USGdb
        db.setPort(globals::mySQLport.toInt());
        db.setConnectOptions(globals::mySQLoptionConnect);
        db.setUserName(globals::mySQLuser);
        db.setPassword(globals::mySQLpasswdUser);
        if (db.open()){
            qInfo(logInfo()) << "Conectarea cu baza de date MYSQL instalata cu succes";
            if (globals::firstLaunch)
                creatingTables();
        } else {
            qWarning(logWarning()) << "Conectarea cu baza de date MYSQL lipseste: " + db.lastError().text();
            return false;
        }

    } else {

        //----------------------------------------------------------------------------------------------------
        // baza de date implicita
        if (globals::sqlitePathBase.isEmpty() || globals::sqlitePathBase.isNull()){
            qCritical(logCritical()) << tr("Nu este indicata variabila globala 'sqlitePathBase'.");
            return false;
        }
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setHostName(globals::sqliteNameBase);
        db.setDatabaseName(globals::sqlitePathBase);
        if(db.open()){
            qInfo(logInfo()) << "";
            qInfo(logInfo()) << "=~=~=~=~=~=~=~=~=~~=~=~=~=~= LANSARE NOUA =~=~=~=~=~=~=~=~~=~=~=~=~=~=~=~=~=";
            qInfo(logInfo()) << tr("Conectarea la baza de date '%1' este instalata cu succes.").arg(globals::sqliteNameBase);
            if (globals::firstLaunch)
                creatingTables();
        } else {
            qWarning(logWarning()) << tr("Conectarea la baza de date '%1' nu a fost instalata.").arg(globals::sqliteNameBase)
                                   << db.lastError().text();
            return false;
        }

        //----------------------------------------------------------------------------------------------------
        // baza de date image
        if (globals::pathImageBaseAppSettings.isEmpty() || globals::pathImageBaseAppSettings.isNull()){
            qWarning(logWarning()) << tr("Nu este indicata variabila globala 'pathImageBaseAppSettings'.");
        } else {
            db_image = QSqlDatabase::addDatabase("QSQLITE", "db_image");
            db_image.setHostName("db_image");
            db_image.setDatabaseName(globals::pathImageBaseAppSettings);
            if (db_image.open()){
                qInfo(logInfo()) << tr("Conectarea la baza de date 'db_image' este instalata cu succes.");
                if (globals::firstLaunch){
                    creatingTables_DbImage();
                }
            } else {
                qWarning(logWarning()) << tr("Conectarea la baza de date 'db_image' nu a fost instalata.")
                                       << db.lastError().text();
                return false;
            }
        }
    }
    return true;
}

bool DataBase::restoreDataDase()
{
    if (this->openDataBase())
        return true;
    else
        return false;
}

void DataBase::closeDataBase()
{
    db.close();
}
