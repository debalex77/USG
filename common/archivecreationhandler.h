#ifndef ARCHIVECREATIONHANDLER_H
#define ARCHIVECREATIONHANDLER_H

#include <QDialog>
#include <QFileDialog>
#include <QProcess>
#include <QStandardPaths>
#include <data/database.h>
#include <data/globals.h>

namespace Ui {
class ArchiveCreationHandler;
}

class ArchiveCreationHandler : public QDialog
{
    Q_OBJECT

public:
    explicit ArchiveCreationHandler(QWidget *parent = nullptr);
    ~ArchiveCreationHandler();

private slots:
    void onAddFiles();
    void onRemoveSelected();
    void onClearList();
    void onBrowseArchive();
    void onStart();
    void onCancel();

    void onProcReadyStdout();
    void onProcFinished(int exitCode, QProcess::ExitStatus status);
    void onProcError(QProcess::ProcessError e);

private:
    QString find7z() const;
    void appendLog(const QString &str);
    QStringList currentFileList() const;

private:
    Ui::ArchiveCreationHandler *ui;

    DataBase *db;
    QString sevenZipPath;

    // Proc
    QProcess    *m_proc = nullptr;
    QByteArray   m_stdoutBuf;
};

#endif // ARCHIVECREATIONHANDLER_H
