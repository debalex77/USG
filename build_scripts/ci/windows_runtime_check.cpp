#include <QCoreApplication>
#include <QDebug>
#include <QSqlDatabase>
#include <QSslSocket>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    // Only plugins from the deployed package may satisfy this check.
    QCoreApplication::setLibraryPaths({QCoreApplication::applicationDirPath()});
    for (const auto &driver : {"QMYSQL", "QSQLITE"}) {
        const auto db = QSqlDatabase::addDatabase(driver, driver);
        if (!db.isValid()) {
            qCritical() << "Cannot load deployed SQL driver" << driver;
            return 1;
        }
    }
    if (!QSslSocket::supportsSsl()) {
        qCritical() << "No working deployed TLS backend";
        return 2;
    }
    qInfo() << "SQL drivers and TLS loaded successfully";
    return 0;
}
