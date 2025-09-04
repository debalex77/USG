#include "archivecreationhandler.h"
#include "ui_archivecreationhandler.h"

#include <QMessageBox>

ArchiveCreationHandler::ArchiveCreationHandler(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ArchiveCreationHandler)
{
    ui->setupUi(this);

    //--- setam titlu ---
    setWindowTitle(tr("Crearea arhivei 7zip"));

    //--- alocarea memoriei ---
    db = new DataBase(this);

    //--- Drum spre arhiva ---
    ui->archivePath->setText(QDir::homePath() +
                             "/Database_usg/" +
                             globals().sqliteNameBase + "_" +
                             QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss"));

    //--- Lista cu BD ---
    ui->list_files->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->list_files->addItem(globals().sqlitePathBase);
    ui->list_files->addItem(globals().pathImageBaseAppSettings);

    //--- compresia ---
    ui->compression->setValue(9);

    //--- progres bar ---
    ui->progress->setRange(0, 100);
    ui->progress->setValue(0);
    ui->progress->setVisible(false);
    ui->btnCancel->setEnabled(false);

    //--- status ---
    ui->txt_status->setText(tr("Initierea ..."));
    ui->txt_status->setVisible(false);

    //--- log ---
    ui->txt_logs->setReadOnly(true);
    ui->txt_logs->setVisible(false);

    //--- detectarea 7zip ---
    sevenZipPath = find7z();
    if (sevenZipPath.isEmpty()) {
        appendLog(tr("Atenție: nu am găsit 7-Zip în PATH.\n"
                     "Debian/Ubuntu: `sudo apt install 7zip` sau `sudo apt install p7zip-full`\n"
                     "Windows: Instalează 7-Zip și adaugă-l în PATH."));
        ui->txt_logs->setVisible(true);
        ui->btnStart->setEnabled(false);
    }

    if (globals().thisMySQL) {
        ui->archivePath->setText("");
        ui->archivePath->setEnabled(false);
        ui->compression->setValue(0);
        ui->compression->setEnabled(false);
        ui->list_files->clear();
        ui->list_files->setEnabled(false);
        appendLog(tr("Procesarea destinată doar pentru baza de date .sqlite !!!"));
        ui->btnAdd->setEnabled(false);
        ui->btnRemove->setEnabled(false);
        ui->btnClear->setEnabled(false);
        ui->btnStart->setEnabled(false);
        ui->btnCancel->setEnabled(false);
        ui->txt_logs->setVisible(true);
        ui->txt_status->setText(tr("Procesarea nu este accesibilă !!!"));
        ui->txt_status->setVisible(true);
    }

    //--- setam stilul butoanelor ---
    QString style_toolButton = db->getStyleForToolButton();
    ui->btnAdd->setStyleSheet(style_toolButton);
    ui->btnRemove->setStyleSheet(style_toolButton);
    ui->btnClear->setStyleSheet(style_toolButton);
    ui->btnStart->setStyleSheet(style_toolButton);
    ui->btnCancel->setStyleSheet(style_toolButton);

    //--- conectarile ----
    connect(ui->archivePath, &LineEditCustom::onClickAddItem, this, &ArchiveCreationHandler::onBrowseArchive);
    connect(ui->archivePath, &LineEditCustom::onClickSelect, this, &ArchiveCreationHandler::onBrowseArchive);
    connect(ui->btnAdd, &QAbstractButton::clicked, this, &ArchiveCreationHandler::onAddFiles);
    connect(ui->btnClear, &QAbstractButton::clicked, this, &ArchiveCreationHandler::onClearList);
    connect(ui->btnRemove, &QAbstractButton::clicked, this, &ArchiveCreationHandler::onRemoveSelected);
    connect(ui->btnStart, &QAbstractButton::clicked, this, &ArchiveCreationHandler::onStart);
    connect(ui->btnCancel, &QAbstractButton::clicked, this, &ArchiveCreationHandler::onCancel);
}

ArchiveCreationHandler::~ArchiveCreationHandler()
{
    if (m_proc) {
        m_proc->kill();
        m_proc->waitForFinished(1000);
        delete m_proc;
    }
    delete ui;
}

void ArchiveCreationHandler::onAddFiles()
{
    const QStringList sel = QFileDialog::getOpenFileNames(this,
                                                          tr("Alege baze SQLite"),
                                                          QString(),
                                                          tr("SQLite DB (*.sqlite *.db *.sqlite3);;Toate fișierele (*)"));
    if (sel.isEmpty())
        return;

    // evităm dublurile
    QSet<QString> existing;
    for (int i = 0; i < ui->list_files->count(); ++i)
        existing.insert(ui->list_files->item(i)->text());

    for (const QString &f : sel) {
        if (! existing.contains(f)) {
            ui->list_files->addItem(f);
            existing.insert(f);
        }
    }

    if (ui->archivePath->text().trimmed().isEmpty())
        ui->archivePath->setText(globals().main_name_organization);
}

void ArchiveCreationHandler::onRemoveSelected()
{
    const auto selected = ui->list_files->selectedItems();
    for (QListWidgetItem *it : selected) {
        delete it;
    }
}

void ArchiveCreationHandler::onClearList()
{
    ui->list_files->clear();
}

void ArchiveCreationHandler::onBrowseArchive()
{
    QString f = QFileDialog::getSaveFileName(
        this,
        tr("Alege arhiva 7z"),
        ui->archivePath->text().trimmed().isEmpty()
            ? globals().main_name_organization
            : ui->archivePath->text(),
        tr("Arhive 7z (*.7z)"));

    if (! f.isEmpty()) {
        if (! f.endsWith(".7z", Qt::CaseInsensitive))
            f += ".7z";
        ui->archivePath->setText(f);
    }
}

void ArchiveCreationHandler::onStart()
{
    const QStringList files = currentFileList();
    if (files.isEmpty()) {
        QMessageBox::warning(this, tr("Eroare"), tr("Adaugă cel puțin un fișier SQLite."));
        return;
    }
    if (sevenZipPath.isEmpty()) {
        QMessageBox::warning(this, tr("Eroare"), tr("7-Zip nu este disponibil în PATH."));
        return;
    }
    QString outArchive = ui->archivePath->text().trimmed();
    if (outArchive.isEmpty()) {
        outArchive = globals().main_name_organization;
        ui->archivePath->setText(outArchive);
    }

    // Curățare UI
    ui->progress->setVisible(true);
    ui->progress->setValue(0);
    ui->txt_status->setVisible(true);
    ui->txt_status->setText(tr("Rulează compresia…"));
    ui->txt_logs->setVisible(true);
    appendLog(QString("\n=== Start → %1 ===").arg(outArchive));
    appendLog(tr("Fișiere:"));

    if (! ui->btnCancel->isEnabled())
        ui->btnCancel->setEnabled(true);

    for (const auto &f : files)
        appendLog("  - " + f);

    if (m_proc) {
        m_proc->deleteLater();
        m_proc = nullptr;
    }
    m_proc = new QProcess(this);

    // Opțiuni 7z:
    //  a      – add to archive
    //  -t7z   – format 7z
    //  -mx=9  – compresie maximă
    //  -mmt=on– multi-thread
    //  -ms=on – solid archive (mai bun pentru multe fișiere similare)
    //  -bsp1  – emite progres %
    //  -bso0  – reduce zgomot stdout
    //  -bd    – fără progress bar TTY
    QStringList args;
    args << "a" << "-t7z"
         << outArchive
         << "-mx=9"
         << "-mmt=on"
         << "-ms=on"
         << "-bsp1"
         << "-bso0"
         << "-bd";

    // Adaugăm toate fișierele selectate
    for (const QString &f : files)
        args << f;

    m_proc->setProcessChannelMode(QProcess::SeparateChannels);

    connect(m_proc, &QProcess::readyReadStandardOutput, this, &ArchiveCreationHandler::onProcReadyStdout);
    connect(m_proc, &QProcess::readyReadStandardError, this, [this](){
        const QByteArray chunk = m_proc->readAllStandardError();
        if (!chunk.isEmpty())
            appendLog(QString::fromLocal8Bit(chunk));
    });
    connect(m_proc, &QProcess::finished, this, &ArchiveCreationHandler::onProcFinished);
    connect(m_proc, &QProcess::errorOccurred, this, &ArchiveCreationHandler::onProcError);

    m_stdoutBuf.clear();

    m_proc->start(sevenZipPath, args);
    if (! m_proc->waitForStarted(3000)) {
        QMessageBox::critical(this, tr("Eroare"), tr("Nu pot porni 7-Zip."));
    }
}

void ArchiveCreationHandler::onCancel()
{
    if (! m_proc)
        return;

    appendLog(tr("Se oprește compresia…"));
    m_proc->terminate();

    if (! m_proc->waitForFinished(2000))
        m_proc->kill();
}

void ArchiveCreationHandler::onProcReadyStdout()
{
    m_stdoutBuf += m_proc->readAllStandardOutput();

    // Parsăm procentele raportate de 7-Zip (agregat pentru toate fișierele).
    static QRegularExpression rx(R"((\d+)\s*%)");
    QRegularExpressionMatchIterator it = rx.globalMatch(QString::fromLocal8Bit(m_stdoutBuf));
    int lastPct = -1;
    while (it.hasNext()) {
        const auto m = it.next();
        lastPct = m.captured(1).toInt();
    }
    if (lastPct >= 0) {
        if (lastPct > 100)
            lastPct = 100;
        ui->progress->setValue(lastPct);
        ui->txt_status->setText(tr("Progres: %1%").arg(lastPct));
    }

    // Trimitem linii întregi în jurnal
    int lastNl = m_stdoutBuf.lastIndexOf('\n');
    if (lastNl >= 0) {
        QByteArray lines = m_stdoutBuf.left(lastNl + 1);
        m_stdoutBuf = m_stdoutBuf.mid(lastNl + 1);
        appendLog(QString::fromLocal8Bit(lines));
    }
}

void ArchiveCreationHandler::onProcFinished(int exitCode, QProcess::ExitStatus status)
{
    if (status == QProcess::NormalExit && exitCode == 0) {
        ui->progress->setValue(100);
        ui->txt_status->setText(tr("Complet."));
        appendLog(tr("Compresie finalizată cu succes."));
    } else {
        ui->txt_status->setText(tr("Eșec (exit=%1).").arg(exitCode));
        appendLog(tr("Compresie oprită/eronată (exit=%1).").arg(exitCode));
    }
}

void ArchiveCreationHandler::onProcError(QProcess::ProcessError e)
{
    appendLog(tr("Eroare proces: %1").arg(static_cast<int>(e)));
    QMessageBox::critical(this,
                          tr("Eroare 7-Zip"),
                          tr("Nu pot rula 7-Zip (cod=%1).")
                              .arg(static_cast<int>(e)));
}

QString ArchiveCreationHandler::find7z() const
{
#if defined(Q_OS_WIN)
    const QStringList candidates = {"7zz.exe", "7za.exe", "7z.exe"};
#else
    const QStringList candidates = {"7zz", "7za", "7z"};
#endif
    for (const QString &name : candidates) {
        QString path = QStandardPaths::findExecutable(name);
        if (! path.isEmpty())
            return path;
    }
    return QString();
}

void ArchiveCreationHandler::appendLog(const QString &str)
{
    ui->txt_logs->append(str.trimmed().isEmpty()
                             ? QStringLiteral(" ")
                             : str.trimmed());
    // ui->txt_logs->adjustSize();
    // this->adjustSize();
}

QStringList ArchiveCreationHandler::currentFileList() const
{
    QStringList files;
    files.reserve(ui->list_files->count());

    for (int i = 0; i < ui->list_files->count(); ++i)
        files << ui->list_files->item(i)->text();

    return files;
}
