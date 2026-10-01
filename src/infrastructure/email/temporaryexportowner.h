#ifndef TEMPORARYEXPORTOWNER_H
#define TEMPORARYEXPORTOWNER_H

#include <QCoreApplication>
#include <QString>

#include <memory>

// Deține un director temporar de export pentru e-mail și îl șterge în destructor.
//
// Directorul se creează doar prin prepare(), care îl înregistrează ca
// nerevendicat. claim() acceptă numai o cale înregistrată și o scoate din
// registru, deci fiecare director are un singur proprietar. Proprietarul se
// împarte prin shared_ptr între AgentSendEmail (firul GUI) și EmailCore
// (firul SMTP): directorul dispare la eliberarea ultimei referințe.
//
// Clasa nu este QObject și nu folosește SessionContext după prepare()
// (directorul de bază este memorat), deci poate fi distrusă pe orice fir.
class TemporaryExportOwner final
{
    Q_DECLARE_TR_FUNCTIONS(TemporaryExportOwner)

public:
    // Creează un subdirector privat și unic în directorul de export al
    // utilizatorului curent și îl înregistrează ca nerevendicat.
    static bool prepare(QString *directory, QString *error = nullptr);

    // Preia un director creat de prepare() și încă nerevendicat;
    // altfel returnează nullptr.
    [[nodiscard]]
    static std::shared_ptr<TemporaryExportOwner> claim(const QString &directory);

    // Șterge un director creat de prepare() și încă nerevendicat
    // (ex. exportul a eșuat înainte de deschiderea agentului e-mail).
    static void discard(const QString &directory);

    ~TemporaryExportOwner();

    TemporaryExportOwner(const TemporaryExportOwner &) = delete;
    TemporaryExportOwner &operator=(const TemporaryExportOwner &) = delete;

    [[nodiscard]]
    const QString &directory() const { return m_directory; }

private:
    TemporaryExportOwner(QString directory, QString basePath);

    static void removeDirectory(const QString &directory, const QString &basePath);

    const QString m_directory;
    const QString m_basePath;
};

#endif // TEMPORARYEXPORTOWNER_H
