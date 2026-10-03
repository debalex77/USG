#ifndef ARCHIVECREATIONHANDLER_H
#define ARCHIVECREATIONHANDLER_H

#include <QDialog>
#include <QFileDialog>
#include <QProcess>
#include <QStandardPaths>
#include <database/database.h>
#include <common/globals.h>

namespace Ui {
class ArchiveCreationHandler;
}

class ArchiveCreationHandler : public QDialog
{
    Q_OBJECT

public:
    explicit ArchiveCreationHandler(DataBase &db, QWidget *parent = nullptr);
    ~ArchiveCreationHandler();

    // Arhivarea automată la închiderea aplicației: pornește singură,
    // afișează progresul și se închide la final. true – arhiva a fost creată.
    bool execAutomatic();

    // Caută 7-Zip în PATH, iar pe Windows și în directoarele Program Files.
    static QString find7z();

public slots:
    void reject() override;

private slots:
    void onAddFiles();
    void onRemoveSelected();
    void onClearList();
    void onBrowseArchive();
    void onStart();
    void onCancel();
    void onArchiveOptionsChanged();
    void onCryptoOptionClicked(bool checked);
    void onEncryptToggled(bool checked);
    void onShowPasswordToggled(bool checked);

    void onProcReadyStdout();
    void onProcFinished(int exitCode, QProcess::ExitStatus status);
    void onProcError(QProcess::ProcessError e);

private:
    void appendLog(const QString &str);
    // Rândurile din list_files: bazele de date sau fișierele de configurare.
    enum ConfigKind { NotConfig = 0, SettingsConfig = 1, CryptoConfig = 2 };
    static constexpr int ConfigKindRole = Qt::UserRole + 1;

    QStringList currentFileList() const;     // doar bazele de date
    QStringList configFileList() const;      // setări + crypto, după bife
    QStringList configFileList(ConfigKind kind) const;
    void refreshConfigItems();
    QString defaultArchivePath() const;
    void setRunning(bool running);
    void finishAutomatic(bool success);

    // Parola arhivei: cea introdusă (validată și salvată) sau cea salvată anterior.
    bool resolveArchivePassword(QString *password, QString *error);
    QString archiveKeySettingsPath() const;
    void updateEncryptionWidgets();
    void startProcess(const QStringList &args, const QByteArray &input = {});
    void startVerification();
    void removeIncompleteArchive();

private:
    Ui::ArchiveCreationHandler *ui;

    DataBase &m_db;
    QString sevenZipPath;
    bool m_automatic = false;

    // Arhiva în lucru: dacă nu exista înainte, la eșec/anulare se șterge.
    QString m_outArchive;
    bool    m_archiveExisted = false;

    // După compresie arhiva se verifică (7z t).
    enum class Phase { Compress, Verify };
    Phase   m_phase = Phase::Compress;
    QString m_password;            // doar pe durata operației
    bool    m_cancelRequested = false;

    // Proc
    QProcess    *m_proc = nullptr;
    QByteArray   m_stdoutBuf;
};

#endif // ARCHIVECREATIONHANDLER_H
