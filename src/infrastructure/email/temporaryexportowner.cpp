#include "temporaryexportowner.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QTemporaryDir>

#include <common/applicationpathscontext.h>
#include <core/loggingcategories.h>

namespace {

// Directoare create de prepare() și încă nerevendicate: cale -> director de bază.
QMutex &registryMutex()
{
    static QMutex mutex;
    return mutex;
}

QHash<QString, QString> &unclaimedDirectories()
{
    static QHash<QString, QString> directories;
    return directories;
}

// Scoate calea din registru; returnează directorul de bază sau șir gol.
QString takeUnclaimed(const QString &directory)
{
    const QString path = QDir::cleanPath(directory);
    if (path.isEmpty())
        return {};
    QMutexLocker locker(&registryMutex());
    return unclaimedDirectories().take(path);
}

} // namespace

bool TemporaryExportOwner::prepare(QString *directory, QString *error)
{
    const QString basePath = QDir::cleanPath(ApplicationPathsContext::instance().exportDirectory());

    // Același mutex protejează ștergerea directorului gol al utilizatorului
    // (posibil din firul SMTP) între mkpath() și crearea subdirectorului.
    QMutexLocker locker(&registryMutex());
    if (!QDir().mkpath(basePath)) {
        if (error)
            *error = tr("Nu poate fi creat directorul temporar pentru export:\n%1")
                         .arg(QDir::toNativeSeparators(basePath));
        qWarning(logWarning()) << "Nu s-a putut crea directorul de export:" << basePath;
        return false;
    }

    QTemporaryDir temporary(QDir(basePath).filePath(QStringLiteral("mail-XXXXXX")));
    temporary.setAutoRemove(false);
    if (!temporary.isValid()) {
        if (error)
            *error = tr("Nu poate fi creat un director temporar unic pentru export:\n%1")
                         .arg(QDir::toNativeSeparators(basePath));
        return false;
    }
    const QString path = QDir::cleanPath(temporary.path());

    // Documentele medicale exportate sunt accesibile doar contului curent.
    if (!QFile::setPermissions(path, QFileDevice::ReadOwner
                                         | QFileDevice::WriteOwner
                                         | QFileDevice::ExeOwner)) {
        qWarning(logWarning()) << "Nu s-au putut restrânge permisiunile directorului de export:" << path;
    }

    unclaimedDirectories().insert(path, basePath);

    if (directory)
        *directory = path;
    return true;
}

std::shared_ptr<TemporaryExportOwner> TemporaryExportOwner::claim(const QString &directory)
{
    const QString basePath = takeUnclaimed(directory);
    if (basePath.isEmpty()) {
        qWarning(logWarning())
            << "Directorul de export nu a fost creat de prepare() sau este deja revendicat:"
            << directory;
        return nullptr;
    }
    // Constructorul privat nu este accesibil pentru std::make_shared.
    return std::shared_ptr<TemporaryExportOwner>(
        new TemporaryExportOwner(QDir::cleanPath(directory), basePath));
}

void TemporaryExportOwner::discard(const QString &directory)
{
    const QString basePath = takeUnclaimed(directory);
    if (basePath.isEmpty()) {
        if (!directory.isEmpty())
            qWarning(logWarning())
                << "Refuz ștergerea unui director de export nerevendicabil:" << directory;
        return;
    }
    removeDirectory(QDir::cleanPath(directory), basePath);
}

TemporaryExportOwner::TemporaryExportOwner(QString directory, QString basePath)
    : m_directory(std::move(directory)), m_basePath(std::move(basePath))
{
}

TemporaryExportOwner::~TemporaryExportOwner()
{
    removeDirectory(m_directory, m_basePath);
}

void TemporaryExportOwner::removeDirectory(const QString &directory, const QString &basePath)
{
    // cleanPath() folosește mereu '/', inclusiv pe Windows.
    if (directory.isEmpty() || basePath.isEmpty() || directory == basePath
        || !directory.startsWith(basePath + QLatin1Char('/'))) {
        qWarning(logWarning()) << "Refuz ștergerea unui director de export invalid:" << directory;
        return;
    }
    QDir dir(directory);
    if (!dir.exists())
        return;

    if (dir.removeRecursively()) {
        qInfo(logInfo()) << "Directorul de export" << directory << "a fost șters.";
    } else {
        qWarning(logWarning()) << "Nu s-a putut șterge directorul de export:" << directory;
        return;
    }

    // După ultima expediere nu păstrăm nici directorul temporar al
    // utilizatorului. Dacă există un alt export activ, directorul nu este
    // gol și rămâne disponibil pentru acel export.
    QMutexLocker locker(&registryMutex());
    QDir userDirectory(basePath);
    if (userDirectory.exists()
        && userDirectory.entryList(QDir::AllEntries | QDir::NoDotAndDotDot).isEmpty()) {
        const QFileInfo userDirectoryInfo(basePath);
        QDir parentDirectory = userDirectoryInfo.dir();
        if (parentDirectory.rmdir(userDirectoryInfo.fileName()))
            qInfo(logInfo()) << "Directorul temporar gol al utilizatorului"
                             << basePath << "a fost șters.";
        else
            qWarning(logWarning())
                << "Nu s-a putut șterge directorul temporar gol al utilizatorului:"
                << basePath;
    }
}
