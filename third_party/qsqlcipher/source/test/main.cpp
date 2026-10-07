// Test QSQLCIPHER: creează o bază criptată, verifică cheia corectă/greșită
// și că fișierul nu e citibil ca SQLite obișnuit.
#include <QCoreApplication>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

static int fails = 0;
#define CHECK(cond, msg) do { if (cond) qInfo() << "OK  " << msg; \
                              else { qCritical() << "FAIL" << msg; ++fails; } } while (0)

static QSqlDatabase openDb(const QString &name, const QString &file, const QString &key)
{
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLCIPHER"), name);
    db.setDatabaseName(file);
    db.setPassword(key);
    db.open();
    return db;
}

int main(int argc, char **argv)
{
#ifdef QSQLCIPHER_BUILD_PLUGINS
    QCoreApplication::addLibraryPath(QStringLiteral(QSQLCIPHER_BUILD_PLUGINS));
#endif
    QCoreApplication app(argc, argv);
    qInfo() << "Drivere disponibile:" << QSqlDatabase::drivers();
    CHECK(QSqlDatabase::isDriverAvailable(QStringLiteral("QSQLCIPHER")), "driver QSQLCIPHER găsit");

    const QString file = QStringLiteral("qsqlcipher_test.db");
    QFile::remove(file);

    {   // 1. creare cu cheie
        QSqlDatabase db = openDb(QStringLiteral("c1"), file, QStringLiteral("parola-secreta"));
        CHECK(db.isOpen(), "deschidere cu cheie");
        QSqlQuery q(db);
        q.exec(QStringLiteral("PRAGMA cipher_version"));
        const QString ver = q.next() ? q.value(0).toString() : QString();
        qInfo() << "     cipher_version =" << ver;
        CHECK(!ver.isEmpty(), "rulează pe SQLCipher");
        CHECK(q.exec(QStringLiteral("CREATE TABLE t(id INTEGER PRIMARY KEY, txt TEXT)")), "CREATE TABLE");
        q.prepare(QStringLiteral("INSERT INTO t(txt) VALUES (?)"));
        q.addBindValue(QStringLiteral("salut, Chișinău"));
        CHECK(q.exec(), "INSERT");
        db.close();
    }
    QSqlDatabase::removeDatabase(QStringLiteral("c1"));

    {   // 2. redeschidere cu cheia corectă
        QSqlDatabase db = openDb(QStringLiteral("c2"), file, QStringLiteral("parola-secreta"));
        QSqlQuery q(QStringLiteral("SELECT txt FROM t"), db);
        CHECK(q.next() && q.value(0).toString() == QStringLiteral("salut, Chișinău"), "citire cu cheia corectă");
    }
    QSqlDatabase::removeDatabase(QStringLiteral("c2"));

    {   // 3. cheie greșită
        QSqlDatabase db = openDb(QStringLiteral("c3"), file, QStringLiteral("gresit"));
        CHECK(!db.isOpen(), "cheia greșită este respinsă");
        qInfo() << "     eroare:" << db.lastError().text();
    }
    QSqlDatabase::removeDatabase(QStringLiteral("c3"));

    {   // 4. fișierul nu are antetul SQLite în clar
        QFile f(file);
        f.open(QIODevice::ReadOnly);
        CHECK(!f.read(16).startsWith("SQLite format 3"), "fișierul este criptat pe disc");
    }

    qInfo() << (fails ? "TEST EȘUAT" : "TOATE TESTELE AU TRECUT");
    return fails ? 1 : 0;
}
