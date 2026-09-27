#include "database/uuidmigrationplan.h"
#include <QCoreApplication>
#include <QSqlQuery>
#include <QSqlError>
#include <QUuid>
#include <QDebug>
#include <cstdlib>

static void check(bool ok, const char *message)
{
    if (!ok) { qCritical() << message; std::exit(1); }
}
static void sql(QSqlDatabase db, const QString &text)
{
    QSqlQuery q(db);
    if (!q.exec(text)) { qCritical() << q.lastError(); std::exit(1); }
}
static QSqlDatabase database(const QString &name)
{
    const bool maria = name == "cloud" && !qEnvironmentVariableIsEmpty("USG_UUID_TEST_SOCKET");
    auto db = QSqlDatabase::addDatabase(maria ? "QMYSQL" : "QSQLITE", name);
    db.setDatabaseName(maria ? "uuid_migration_test" : ":memory:");
    if (maria) {
        db.setUserName("root");
        db.setConnectOptions("UNIX_SOCKET=" + qEnvironmentVariable("USG_UUID_TEST_SOCKET"));
    }
    check(db.open(), "open");
    const QStringList definitions = {
        "pacients (name TEXT, fName TEXT, birthday TEXT, IDNP TEXT)",
        "organizations (name TEXT, address TEXT, IDNP TEXT)",
        "doctors (name TEXT, fName TEXT, mName TEXT)",
        "nurses (name TEXT, fName TEXT, mName TEXT)",
        "users (name TEXT)", "typesPrices (name TEXT, discount REAL, noncomercial INT)",
        "investigationsGroup (name TEXT)", "investigations (cod TEXT, name TEXT)",
        "conclusionTemplates (cod TEXT, name TEXT, system TEXT)",
        "formationsSystemTemplates (name TEXT, typeSystem TEXT)",
        "contracts (name TEXT, dateInit TEXT, id_organizations INT, id_typesPrices INT)",
        "pricings (numberDoc TEXT, dateDoc TEXT, id_organizations INT, id_typesPrices INT, id_contracts INT)",
        "orderEcho (id_pacients INT, id_organizations INT, numberDoc TEXT, dateDoc TEXT)",
        "reportEcho (id_pacients INT, id_orderEcho INT, numberDoc TEXT, dateDoc TEXT)",
        "imagesReports (id_patients INT, id_orderEcho INT, id_reportEcho INT, image_1 BLOB, image_2 BLOB, image_3 BLOB, image_4 BLOB, image_5 BLOB)"
    };
    for (QString definition : definitions) {
        definition.insert(definition.indexOf('(') + 1, "id INTEGER PRIMARY KEY, uuid BLOB, ");
        sql(db, "CREATE TABLE " + definition);
    }
    return db;
}
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    auto local = database("local");
    auto cloud = database("cloud");
    sql(local, "INSERT INTO pacients (id,name,fName,birthday,IDNP) VALUES (1,'Test','One','2000-01-02','123')");
    sql(cloud, "INSERT INTO pacients (id,name,fName,birthday,IDNP) VALUES (100,'Test','One','2000-01-02','123')");
    sql(cloud, "INSERT INTO pacients (id,name,fName,birthday,IDNP) VALUES (1,'Other','Person','1990-01-02','456')");
    sql(local, "INSERT INTO organizations (id,name,address,IDNP) VALUES (1,'Clinic','Test','777')");
    sql(cloud, "INSERT INTO organizations (id,name,address,IDNP) VALUES (50,'Clinic','Test','777')");
    sql(local, "INSERT INTO typesPrices (id,name,discount,noncomercial) VALUES (1,'Commercial',0,0)");
    sql(cloud, "INSERT INTO typesPrices (id,name,discount,noncomercial) VALUES (55,'Коммерческие',15,0)");
    sql(local, "INSERT INTO contracts (id,name,dateInit,id_organizations,id_typesPrices) VALUES (1,'Contract','2025-01-01',1,1)");
    sql(cloud, "INSERT INTO contracts (id,name,dateInit,id_organizations,id_typesPrices) VALUES (56,'Contract','2025-01-01',50,55)");
    sql(local, "INSERT INTO pricings (id,numberDoc,dateDoc,id_organizations,id_typesPrices,id_contracts) VALUES (1,'P-1','2025-01-02 09:00:00',1,1,1)");
    sql(cloud, "INSERT INTO pricings (id,numberDoc,dateDoc,id_organizations,id_typesPrices,id_contracts) VALUES (57,'P-1','2025-01-02 09:00:00',50,55,56)");
    sql(local, "INSERT INTO orderEcho (id,id_pacients,id_organizations,numberDoc,dateDoc) VALUES (1,1,1,'12/2025','2025-01-02 12:30:00')");
    sql(cloud, "INSERT INTO orderEcho (id,id_pacients,id_organizations,numberDoc,dateDoc) VALUES (60,100,50,'12/2025','2025-01-02 12:30:00')");
    sql(local, "INSERT INTO reportEcho (id,id_pacients,id_orderEcho,numberDoc,dateDoc) VALUES (1,1,1,'12/2025','2025-01-02 12:30:00')");
    sql(cloud, "INSERT INTO reportEcho (id,id_pacients,id_orderEcho,numberDoc,dateDoc) VALUES (70,100,60,'12/2025','2025-01-02 12:30:00')");
    sql(local, "INSERT INTO imagesReports (id,id_patients,id_orderEcho,id_reportEcho,image_1) VALUES (1,1,1,1,X'1234')");
    sql(cloud, "INSERT INTO imagesReports (id,id_patients,id_orderEcho,id_reportEcho,image_1) VALUES (80,100,60,70,X'1234')");
    const auto audit = UuidMigration::build(local, local, cloud);
    check(audit.valid() && audit.matches.size() == 8, "different IDs must match using relationships");
    check(audit.matches.first().cloudId == 100, "must not match same numeric ID");
    if (cloud.driverName() == "QMYSQL") {
        check(cloud.transaction(), "begin MariaDB transaction");
        check(UuidMigration::build(local, local, cloud, false, true).valid(), "locked MariaDB audit");
        check(cloud.rollback(), "rollback MariaDB transaction");
    }
    check(!UuidMigration::build(local, local, cloud, true).valid(), "apply requires UUIDs");
    sql(cloud, "UPDATE pacients SET name='Different' WHERE id=100");
    const auto sameIdnpDifferentPerson = UuidMigration::build(local, local, cloud);
    check(sameIdnpDifferentPerson.valid() && !sameIdnpDifferentPerson.warnings.isEmpty(),
          "equal IDNP with different NPP/birthday must be skipped");
    sql(cloud, "UPDATE pacients SET name='Test' WHERE id=100");
    sql(cloud, "UPDATE pacients SET IDNP='999' WHERE id=100");
    const auto differentIdnp = UuidMigration::build(local, local, cloud);
    check(differentIdnp.valid() && !differentIdnp.warnings.isEmpty(),
          "conflicting IDNP must be skipped");
    sql(local, "UPDATE pacients SET IDNP=NULL");
    sql(cloud, "UPDATE pacients SET IDNP=NULL WHERE id=100");
    check(UuidMigration::build(local, local, cloud).valid(), "null IDNP uses demographics");
    sql(cloud, "INSERT INTO pacients (id,name,fName,birthday) VALUES (101,'Test','One','2000-01-02')");
    const auto duplicate = UuidMigration::build(local, local, cloud);
    check(duplicate.valid() && !duplicate.warnings.isEmpty(),
          "duplicate demographics must be skipped");
    sql(cloud, "DELETE FROM pacients WHERE id=101");
    sql(local, "INSERT INTO pacients (id,name,fName,birthday) VALUES (2,'Test','One','2000-01-02')");
    const auto manyToOne = UuidMigration::build(local, local, cloud);
    check(manyToOne.valid() && !manyToOne.warnings.isEmpty(),
          "many-to-one identity must be skipped");
    sql(local, "DELETE FROM pacients WHERE id=2");
    sql(cloud, "UPDATE pacients SET uuid=X'00112233445566778899aabbccddeeff' WHERE id=100");
    check(!UuidMigration::build(local, local, cloud).valid(), "existing different UUID must block");
    sql(local, "UPDATE pacients SET uuid=X'00112233445566778899aabbccddeeff'");
    check(UuidMigration::build(local, local, cloud).valid(), "existing same UUID is idempotent");
    sql(cloud, "ALTER TABLE pacients RENAME TO patients");
    sql(cloud, "ALTER TABLE patients RENAME COLUMN name TO last_name");
    sql(cloud, "ALTER TABLE patients RENAME COLUMN fName TO first_name");
    check(UuidMigration::build(local, local, cloud).valid(), "mixed legacy/new patient schema");
    sql(cloud, "DELETE FROM reportEcho");
    const auto missing = UuidMigration::build(local, local, cloud);
    check(missing.valid() && missing.matches.size() == 6, "missing parent must not map child by ID");
    QSqlQuery verify(cloud);
    check(verify.exec("SELECT COUNT(*) FROM organizations WHERE uuid IS NOT NULL") && verify.next()
          && verify.value(0).toInt() == 0, "audit must not write");
    qInfo() << "PASS: identity, parent mapping, ambiguity, UUID conflicts, idempotency, read-only audit.";
}
