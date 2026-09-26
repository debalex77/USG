#ifndef APPLICATIONPATHSCONTEXT_H
#define APPLICATIONPATHSCONTEXT_H

#include <QReadWriteLock>
#include <QString>

struct ApplicationPathsData
{
    QString templatesDirectory;
    QString reportsDirectory;
    QString settingsFilePath;
    QString logFilePath;
    QString videoDirectory;

    // pu compararea automata
    friend bool operator==(const ApplicationPathsData &,
                           const ApplicationPathsData &) = default;
};

/********************************************************
 * QReadLocker și QWriteLocker se folosesc împreună cu QReadWriteLock,
 * atunci când mai multe thread-uri accesează aceeași resursă, iar tu vrei:
 *  - mai multe thread-uri să poată citi simultan;
 *  - dar numai un singur thread să poată modifica resursa;
 *  - în timpul modificării, nimeni altcineva să nu poată nici citi, nici scrie.
 *
******************************************************************************************/
class ApplicationPathsContext final
{
public:
    static ApplicationPathsContext &instance();

    [[nodiscard("data() - verifica corectitudinea datelor")]]
    ApplicationPathsData data() const;

    void setData(const ApplicationPathsData &data);
    void setSettingsFilePath(const QString &path);
    void clear();

    [[nodiscard("configDirectory() - Verifica directoriu cu fisierul .conf")]]
    QString configDirectory() const;

    [[nodiscard("uiSettingsDirectory() - verifica")]]
    QString uiSettingsDirectory() const;

    [[nodiscard]] QString startupSettingsFilePath() const;

    [[nodiscard("tableSettingsFilePath() - verifica corectitudinea drumului spre setarile tabelelor")]]
    QString tableSettingsFilePath() const;

    [[nodiscard("reportSettingsFilePath() - verifica corectitudinea drumului spre rapoarte")]]
    QString reportSettingsFilePath() const;

    [[nodiscard("exportDirectory() -  verifica drumul spre directoriu pentru export")]]
    QString exportDirectory() const;

private:
    ApplicationPathsContext() = default;

    mutable QReadWriteLock m_lock;
    ApplicationPathsData m_data;

};

#endif // APPLICATIONPATHSCONTEXT_H
