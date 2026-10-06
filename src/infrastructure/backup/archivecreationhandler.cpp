/*****************************************************************************
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2025 Codreanu Alexandru <alovada.med@gmail.com>
 *
 * This file is part of the USG project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include "archivecreationhandler.h"
#include "ui_archivecreationhandler.h"
#include "common/applicationpathscontext.h"
#include "common/organizationcontext.h"
#include "settings/appsettingsstore.h"

#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QTimer>

ArchiveCreationHandler::ArchiveCreationHandler(DataBase &db, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ArchiveCreationHandler)
    , m_db(db)
{
    ui->setupUi(this);

    const MainDatabaseConnectionData connection =
        MainDatabaseConnectionContext::instance().data();

    //--- setam titlu ---
    setWindowTitle(tr("Crearea arhivei 7zip"));

    //--- Drum spre arhiva ---
    ui->archivePath->setText(defaultArchivePath());

    //--- Lista cu BD ---
    ui->list_files->setSelectionMode(QAbstractItemView::ExtendedSelection);
    for (const QString &path : {connection.sqliteDatabasePath,
                                connection.imageDatabasePath}) {
        if (!path.trimmed().isEmpty())
            ui->list_files->addItem(QDir::toNativeSeparators(path));
    }

    //--- compresia ---
    ui->compression->setValue(9);

    //--- fisierele de configurare (optiuni retinute in startup.ini) ---
    const AppSettingsStore::ArchiveOptions options =
        AppSettingsStore::readArchiveOptions(
            ApplicationPathsContext::instance().startupSettingsFilePath());
    ui->checkIncludeSettings->setChecked(options.includeSettings);
    ui->checkIncludeCrypto->setChecked(options.includeCrypto);
    ui->checkEncrypt->setChecked(options.encrypt);
    refreshConfigItems();

    //--- criptarea arhivei ---
    // Doar ASCII: parolele cu diacritice se transmit diferit de 7-Zip pe
    // Windows/Linux, iar arhiva nu ar mai putea fi deschisă pe alt sistem.
    const auto *passwordValidator = new QRegularExpressionValidator(
        QRegularExpression(QStringLiteral("[\\x20-\\x7E]*")), this);
    ui->editPassword->setValidator(passwordValidator);
    ui->editPasswordConfirm->setValidator(passwordValidator);
    ui->editPassword->setMaxLength(128);
    ui->editPasswordConfirm->setMaxLength(128);
    ui->checkEncrypt->setToolTip(
        tr("Fără parolă arhiva nu poate fi restaurată. Notați parola într-un loc sigur."));
    updateEncryptionWidgets();
    ui->checkIncludeSettings->setToolTip(
        QDir::toNativeSeparators(ApplicationPathsContext::instance().uiSettingsDirectory()));
    ui->checkIncludeCrypto->setToolTip(
        tr("Atenție: cu cheile de criptare, oricine are arhiva poate decripta "
           "parolele salvate (cloud, e-mail, profil). Păstrați arhiva într-un loc sigur."));

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

    if (connection.backend == MainDatabaseBackend::MariaDb) {
        ui->archivePath->setText("");
        ui->archivePath->setEnabled(false);
        ui->compression->setValue(0);
        ui->compression->setEnabled(false);
        ui->list_files->clear();
        ui->list_files->setEnabled(false);
        ui->groupConfig->setEnabled(false);
        ui->groupEncryption->setEnabled(false);
        appendLog(tr("Arhivarea este disponibilă pentru bazele locale SQLite/SQLCipher."));
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
    QString style_toolButton = m_db.toolButtonStyleForText();
    ui->btnAdd->setStyleSheet(style_toolButton);
    ui->btnRemove->setStyleSheet(style_toolButton);
    ui->btnClear->setStyleSheet(style_toolButton);
    ui->btnStart->setStyleSheet(style_toolButton);
    ui->btnCancel->setStyleSheet(style_toolButton);

    //--- conectarile ----
    connect(ui->archivePath, &LineEditCustom::onClickAddItem,
            this, &ArchiveCreationHandler::onBrowseArchive, Qt::UniqueConnection);
    connect(ui->archivePath, &LineEditCustom::onClickSelect,
            this, &ArchiveCreationHandler::onBrowseArchive, Qt::UniqueConnection);
    connect(ui->btnAdd, &QAbstractButton::clicked,
            this, &ArchiveCreationHandler::onAddFiles, Qt::UniqueConnection);
    connect(ui->btnClear, &QAbstractButton::clicked,
            this, &ArchiveCreationHandler::onClearList, Qt::UniqueConnection);
    connect(ui->btnRemove, &QAbstractButton::clicked,
            this, &ArchiveCreationHandler::onRemoveSelected, Qt::UniqueConnection);
    connect(ui->btnStart, &QAbstractButton::clicked,
            this, &ArchiveCreationHandler::onStart, Qt::UniqueConnection);
    connect(ui->btnCancel, &QAbstractButton::clicked,
            this, &ArchiveCreationHandler::onCancel, Qt::UniqueConnection);
    connect(ui->checkIncludeSettings, &QCheckBox::toggled,
            this, &ArchiveCreationHandler::onArchiveOptionsChanged, Qt::UniqueConnection);
    connect(ui->checkIncludeCrypto, &QCheckBox::toggled,
            this, &ArchiveCreationHandler::onArchiveOptionsChanged, Qt::UniqueConnection);
    // clicked – doar acțiunea utilizatorului, nu și setChecked() din constructor
    connect(ui->checkIncludeCrypto, &QCheckBox::clicked,
            this, &ArchiveCreationHandler::onCryptoOptionClicked, Qt::UniqueConnection);
    connect(ui->checkEncrypt, &QCheckBox::toggled,
            this, &ArchiveCreationHandler::onEncryptToggled, Qt::UniqueConnection);
    connect(ui->checkShowPassword, &QCheckBox::toggled,
            this, &ArchiveCreationHandler::onShowPasswordToggled, Qt::UniqueConnection);
}

void ArchiveCreationHandler::onEncryptToggled(bool checked)
{
    Q_UNUSED(checked)
    updateEncryptionWidgets();
    onArchiveOptionsChanged();
}

void ArchiveCreationHandler::onShowPasswordToggled(bool checked)
{
    const QLineEdit::EchoMode mode = checked ? QLineEdit::Normal : QLineEdit::Password;
    ui->editPassword->setEchoMode(mode);
    ui->editPasswordConfirm->setEchoMode(mode);
}

QString ArchiveCreationHandler::archiveKeySettingsPath() const
{
    // Aceeași cheie ca parola profilului: crypto/profile.key de lângă profil.
    return ApplicationPathsContext::instance().data().settingsFilePath;
}

void ArchiveCreationHandler::updateEncryptionWidgets()
{
    const bool encrypt = ui->checkEncrypt->isChecked();
    ui->labelPassword->setEnabled(encrypt);
    ui->editPassword->setEnabled(encrypt);
    ui->labelPasswordConfirm->setEnabled(encrypt);
    ui->editPasswordConfirm->setEnabled(encrypt);
    ui->checkShowPassword->setEnabled(encrypt);

    QString stored;
    const bool hasStored = AppSettingsStore::readArchivePassword(
        ApplicationPathsContext::instance().startupSettingsFilePath(),
        archiveKeySettingsPath(), &stored);
    ui->editPassword->setPlaceholderText(
        hasStored ? tr("parola salvată (introduceți una nouă pentru a o schimba)")
                  : tr("minimum 8 caractere, fără diacritice"));
}

bool ArchiveCreationHandler::resolveArchivePassword(QString *password, QString *error)
{
    const QString startupPath = ApplicationPathsContext::instance().startupSettingsFilePath();
    const QString entered = ui->editPassword->text();
    const QString confirmed = ui->editPasswordConfirm->text();

    // Câmpuri goale: se folosește parola salvată (și la arhivarea automată).
    if (m_automatic || (entered.isEmpty() && confirmed.isEmpty())) {
        QString readError;
        if (AppSettingsStore::readArchivePassword(startupPath, archiveKeySettingsPath(),
                                                  password, &readError))
            return true;
        *error = tr("Parola arhivei nu este disponibilă (%1). "
                    "Introduceți parola în dialogul de arhivare.").arg(readError);
        return false;
    }

    if (entered.size() < 8) {
        *error = tr("Parola arhivei trebuie să conțină cel puțin 8 caractere.");
        return false;
    }
    if (entered != confirmed) {
        *error = tr("Parola și confirmarea nu coincid.");
        return false;
    }

    const auto answer = QMessageBox::warning(
        this,
        tr("Parola arhivei"),
        tr("Fără această parolă arhiva <b>nu poate fi restaurată</b>.<br>"
           "Notați parola într-un loc sigur, separat de calculator.<br><br>"
           "Parola va fi salvată pentru arhivarea automată la închidere.<br><br>"
           "Continuați?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (answer != QMessageBox::Yes) {
        error->clear();             // anulat de utilizator, fără mesaj
        return false;
    }

    QString writeError;
    if (!AppSettingsStore::writeArchivePassword(startupPath, archiveKeySettingsPath(),
                                                entered, &writeError)) {
        *error = tr("Parola arhivei nu a putut fi salvată: %1").arg(writeError);
        return false;
    }

    ui->editPassword->clear();
    ui->editPasswordConfirm->clear();
    updateEncryptionWidgets();
    *password = entered;
    return true;
}

void ArchiveCreationHandler::onCryptoOptionClicked(bool checked)
{
    // Într-o arhivă criptată cheile sunt protejate de parola arhivei.
    if (! checked || ui->checkEncrypt->isChecked())
        return;

    const auto answer = QMessageBox::warning(
        this,
        tr("Cheile de criptare"),
        tr("Cu cheile de criptare, oricine are arhiva poate decripta "
           "parolele salvate (cloud, e-mail, profil).<br><br>"
           "Păstrați arhiva într-un loc sigur.<br><br>"
           "Adăugați cheile de criptare în arhivă?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (answer != QMessageBox::Yes)
        ui->checkIncludeCrypto->setChecked(false);
}

ArchiveCreationHandler::~ArchiveCreationHandler()
{
    if (m_proc) {
        m_proc->disconnect(this);
        m_proc->kill();
        m_proc->waitForFinished(1000);
        delete m_proc;
    }
    delete ui;
}

bool ArchiveCreationHandler::execAutomatic()
{
    m_automatic = true;
    setWindowTitle(tr("Arhivarea automată a bazei de date"));
    setRunning(false);

    if (sevenZipPath.isEmpty()) {
        qCritical(logCritical())
            << tr("Arhivarea automată nu poate fi efectuată: 7-Zip nu este instalat.");
        return false;
    }

    QTimer::singleShot(0, this, &ArchiveCreationHandler::onStart);
    return exec() == QDialog::Accepted;
}

QString ArchiveCreationHandler::find7z()
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

#if defined(Q_OS_WIN)
    // Instalatorul 7-Zip nu adaugă directorul în PATH.
    for (const char *variable : {"ProgramW6432", "ProgramFiles", "ProgramFiles(x86)"}) {
        const QString root = qEnvironmentVariable(variable);
        if (root.isEmpty())
            continue;
        const QString path = QDir(root).filePath(QStringLiteral("7-Zip/7z.exe"));
        if (QFileInfo(path).isExecutable())
            return QDir::toNativeSeparators(path);
    }
#endif
    return QString();
}

void ArchiveCreationHandler::reject()
{
    if (m_proc && m_proc->state() != QProcess::NotRunning)
        onCancel();
    QDialog::reject();
}

void ArchiveCreationHandler::onAddFiles()
{
    const QStringList sel =
        QFileDialog::getOpenFileNames(this,
                                      tr("Alege baze SQLite"),
                                      QString(),
                                      tr("Fișiere SQLite/SQLCipher (*.sqlite3 *.sqlite *.db *.sqlcipher);;Toate fișierele (*)"));
    if (sel.isEmpty())
        return;

    // evităm dublurile
    QSet<QString> existing;
    for (int i = 0; i < ui->list_files->count(); ++i)
        existing.insert(ui->list_files->item(i)->text());

    for (const QString &f : sel) {
        const QString path = QDir::toNativeSeparators(f);
        if (! existing.contains(path)) {
            ui->list_files->addItem(path);
            existing.insert(path);
        }
    }

    if (ui->archivePath->text().trimmed().isEmpty())
        ui->archivePath->setText(defaultArchivePath());
}

void ArchiveCreationHandler::onRemoveSelected()
{
    // Un rând de configurare se elimină prin debifarea opțiunii lui
    // (toggled → refreshConfigItems), ca bifa și lista să rămână sincronizate.
    bool removeSettings = false;
    bool removeCrypto = false;

    const auto selected = ui->list_files->selectedItems();
    for (QListWidgetItem *it : selected) {
        switch (it->data(ConfigKindRole).toInt()) {
        case SettingsConfig: removeSettings = true; break;
        case CryptoConfig:   removeCrypto = true;   break;
        default:             delete it;             break;
        }
    }

    if (removeSettings)
        ui->checkIncludeSettings->setChecked(false);
    if (removeCrypto)
        ui->checkIncludeCrypto->setChecked(false);
}

void ArchiveCreationHandler::onClearList()
{
    ui->list_files->clear();
    ui->checkIncludeSettings->setChecked(false);
    ui->checkIncludeCrypto->setChecked(false);
}

void ArchiveCreationHandler::onBrowseArchive()
{
    QString f = QFileDialog::getSaveFileName(
        this,
        tr("Alege arhiva 7z"),
        ui->archivePath->text().trimmed().isEmpty()
            ? defaultArchivePath()
            : ui->archivePath->text(),
        tr("Arhive 7z (*.7z)"));

    if (! f.isEmpty()) {
        if (! f.endsWith(".7z", Qt::CaseInsensitive))
            f += ".7z";
        ui->archivePath->setText(QDir::toNativeSeparators(f));
    }
}

void ArchiveCreationHandler::onStart()
{
    if (m_proc && m_proc->state() != QProcess::NotRunning)
        return;

    const auto failStart = [this](const QString &message) {
        if (m_automatic) {
            qCritical(logCritical()) << message;
            finishAutomatic(false);
        } else {
            QMessageBox::warning(this, tr("Eroare"), message);
        }
    };

    if (sevenZipPath.isEmpty()) {
        failStart(tr("7-Zip nu este disponibil în PATH."));
        return;
    }

    QStringList files;
    for (const QString &f : currentFileList()) {
        if (QFileInfo::exists(f))
            files << f;
        else
            appendLog(tr("Fișierul lipsește și nu va fi arhivat: %1").arg(f));
    }
    if (files.isEmpty()) {
        failStart(tr("Adaugă cel puțin un fișier SQLite."));
        return;
    }

    QString outArchive = ui->archivePath->text().trimmed();
    if (outArchive.isEmpty())
        outArchive = defaultArchivePath();
    if (! outArchive.endsWith(".7z", Qt::CaseInsensitive))
        outArchive += ".7z";
    outArchive = QDir::toNativeSeparators(QDir::cleanPath(outArchive));
    ui->archivePath->setText(outArchive);

    const QString archiveDirectory = QFileInfo(outArchive).absolutePath();
    if (! QDir().mkpath(archiveDirectory)) {
        failStart(tr("Directorul arhivei nu poate fi creat: %1")
                      .arg(QDir::toNativeSeparators(archiveDirectory)));
        return;
    }

    const QStringList configFiles = configFileList();

    m_password.clear();
    if (ui->checkEncrypt->isChecked()) {
        QString passwordError;
        if (!resolveArchivePassword(&m_password, &passwordError)) {
            // Nu se creează în tăcere o arhivă necriptată.
            if (!passwordError.isEmpty())
                failStart(passwordError);
            return;
        }
    }

    // Curățare UI
    ui->progress->setVisible(true);
    ui->progress->setValue(0);
    ui->txt_status->setVisible(true);
    ui->txt_status->setText(tr("Rulează compresia…"));
    ui->txt_logs->setVisible(true);
    appendLog(QString("\n=== Start → %1 ===").arg(outArchive));
    appendLog(tr("Fișiere:"));
    for (const auto &f : files + configFiles)
        appendLog("  - " + f);

    if (!m_password.isEmpty())
        appendLog(tr("Arhiva va fi criptată (AES-256)."));

    setRunning(true);

    m_outArchive = outArchive;
    m_archiveExisted = QFileInfo::exists(outArchive);
    m_phase = Phase::Compress;
    m_cancelRequested = false;

    // Opțiuni 7z:
    //  a      – add to archive
    //  -t7z   – format 7z
    //  -mx=N  – nivelul compresiei (1..9)
    //  -mmt=on– multi-thread
    //  -ms=on – solid archive (mai bun pentru multe fișiere similare)
    //  -ssw   – citește și fișierele deschise de alt proces (baza curentă pe Windows)
    //  -bsp1  – progresul (%) în stdout; nu se folosește -bd, care îl dezactivează
    //  -bso0  – fără alte mesaje în stdout
    //  -p     – parola citită din stdin (parola + confirmarea), nu din linia de comandă
    //  -mhe=on– criptează și lista fișierelor
    //  --     – sfârșitul opțiunilor (fișiere care încep cu '-')
    QStringList args;
    args << "a" << "-t7z"
         << outArchive
         << QStringLiteral("-mx=%1").arg(m_automatic ? 9 : ui->compression->value())
         << "-mmt=on"
         << "-ms=on"
         << "-ssw"
         << "-bsp1"
         << "-bso0";
    QByteArray input;
    if (!m_password.isEmpty()) {
        args << "-p" << "-mhe=on";
        const QByteArray password = m_password.toLatin1();
        input = password + '\n' + password + '\n';
    }
    args << "--";

    // Adaugăm toate fișierele selectate și directoarele de configurare
    args << files << configFiles;

    startProcess(args, input);
}

void ArchiveCreationHandler::startProcess(const QStringList &args, const QByteArray &input)
{
    if (m_proc) {
        m_proc->disconnect(this);
        m_proc->deleteLater();
        m_proc = nullptr;
    }
    m_proc = new QProcess(this);

    m_proc->setProcessChannelMode(QProcess::SeparateChannels);

    connect(m_proc, &QProcess::readyReadStandardOutput,
            this, &ArchiveCreationHandler::onProcReadyStdout);
    connect(m_proc, &QProcess::readyReadStandardError, this, [this](){
        const QByteArray chunk = m_proc->readAllStandardError();
        if (!chunk.isEmpty())
            appendLog(QString::fromLocal8Bit(chunk));
    });
    connect(m_proc, &QProcess::finished,
            this, &ArchiveCreationHandler::onProcFinished);
    connect(m_proc, &QProcess::errorOccurred,
            this, &ArchiveCreationHandler::onProcError);

    m_stdoutBuf.clear();

    m_proc->start(sevenZipPath, args);
    // stdin se închide mereu: 7-Zip nu trebuie să aștepte o parolă la consolă.
    if (!input.isEmpty())
        m_proc->write(input);
    m_proc->closeWriteChannel();
}

void ArchiveCreationHandler::startVerification()
{
    m_phase = Phase::Verify;
    ui->progress->setValue(0);
    ui->txt_status->setText(tr("Verificarea arhivei…"));
    appendLog(tr("Verificarea arhivei (7z t)…"));

    // La verificare 7-Zip nu citește parola din stdin, deci se transmite ca
    // argument (vizibilă scurt timp în lista proceselor locale).
    QStringList args;
    args << "t" << "-bsp1" << "-bso0";
    if (!m_password.isEmpty())
        args << QStringLiteral("-p%1").arg(m_password);
    args << "--" << m_outArchive;

    startProcess(args);
}

void ArchiveCreationHandler::removeIncompleteArchive()
{
    // Nu lăsăm o arhivă invalidă care ar putea fi luată drept copie validă.
    if (! m_archiveExisted && QFileInfo::exists(m_outArchive)
        && QFile::remove(m_outArchive)) {
        appendLog(tr("Arhiva incompletă a fost ștearsă."));
    }
}

void ArchiveCreationHandler::onCancel()
{
    if (! m_proc || m_proc->state() == QProcess::NotRunning)
        return;

    m_cancelRequested = true;
    appendLog(m_phase == Phase::Verify ? tr("Se oprește verificarea…")
                                       : tr("Se oprește compresia…"));
    m_proc->terminate();

    if (! m_proc->waitForFinished(2000))
        m_proc->kill();
}

void ArchiveCreationHandler::onArchiveOptionsChanged()
{
    refreshConfigItems();

    AppSettingsStore::ArchiveOptions options;
    options.includeSettings = ui->checkIncludeSettings->isChecked();
    options.includeCrypto   = ui->checkIncludeCrypto->isChecked();
    options.encrypt         = ui->checkEncrypt->isChecked();
    if (! AppSettingsStore::writeArchiveOptions(
            ApplicationPathsContext::instance().startupSettingsFilePath(), options)) {
        qWarning(logWarning()) << "Opțiunile arhivei nu au putut fi salvate.";
    }
}

void ArchiveCreationHandler::onProcReadyStdout()
{
    // Cu -bsp1 7-Zip scrie progresul ca „ 12% + fisier”, rescris cu '\b',
    // fără '\n'. Păstrăm doar coada buffer-ului (procentul poate fi
    // împărțit între două citiri).
    m_stdoutBuf += m_proc->readAllStandardOutput();

    static const QRegularExpression rx(R"((\d{1,3})%)");
    QRegularExpressionMatchIterator it = rx.globalMatch(QString::fromLatin1(m_stdoutBuf));
    int lastPct = -1;
    while (it.hasNext())
        lastPct = it.next().captured(1).toInt();

    if (m_stdoutBuf.size() > 64)
        m_stdoutBuf = m_stdoutBuf.right(64);

    if (lastPct >= 0) {
        lastPct = qBound(0, lastPct, 100);
        if (lastPct > ui->progress->value()) {
            ui->progress->setValue(lastPct);
            ui->txt_status->setText(tr("Progres: %1%").arg(lastPct));
        }
    }
}

void ArchiveCreationHandler::onProcFinished(int exitCode, QProcess::ExitStatus status)
{
    if (m_phase == Phase::Compress) {
        // 7-Zip: 0 – succes, 1 – avertismente (ex. fișier blocat), arhiva există.
        const bool success = status == QProcess::NormalExit && (exitCode == 0 || exitCode == 1);
        if (success && ! m_cancelRequested) {
            appendLog(tr("Compresie finalizată cu succes."));
            if (exitCode == 1)
                qWarning(logWarning()) << "7-Zip a terminat cu avertismente:" << m_outArchive;
            startVerification();
            return;
        }

        setRunning(false);
        m_password.clear();
        ui->txt_status->setText(tr("Eșec (exit=%1).").arg(exitCode));
        appendLog(tr("Compresie oprită/eronată (exit=%1).").arg(exitCode));
        if (m_automatic)
            qCritical(logCritical())
                << tr("Arhivarea automată a eșuat. Cod proces:") << exitCode;
        removeIncompleteArchive();
        finishAutomatic(false);
        return;
    }

    // Phase::Verify
    setRunning(false);
    m_password.clear();

    const bool verified = status == QProcess::NormalExit && exitCode == 0;
    if (verified) {
        ui->progress->setValue(100);
        ui->txt_status->setText(tr("Complet."));
        appendLog(tr("Arhiva a fost verificată cu succes."));
        if (m_automatic)
            qInfo(logInfo()) << tr("Arhiva automată SQLite a fost creată:") << m_outArchive;
        else
            qInfo(logInfo()) << "Arhiva 7z a fost creată și verificată:" << m_outArchive;
    } else if (m_cancelRequested) {
        // Arhiva este completă, doar verificarea a fost întreruptă.
        ui->txt_status->setText(tr("Verificarea a fost anulată."));
        appendLog(tr("Verificarea a fost anulată; arhiva a fost păstrată, dar nu este verificată."));
        qWarning(logWarning()) << "Verificarea arhivei 7z a fost anulată:" << m_outArchive;
    } else {
        ui->txt_status->setText(tr("Verificarea a eșuat (exit=%1).").arg(exitCode));
        appendLog(tr("Arhiva nu a trecut verificarea (exit=%1).").arg(exitCode));
        qCritical(logCritical()) << "Verificarea arhivei 7z a eșuat:" << m_outArchive
                                 << "cod:" << exitCode;
        removeIncompleteArchive();
    }

    finishAutomatic(verified);
}

void ArchiveCreationHandler::onProcError(QProcess::ProcessError e)
{
    appendLog(tr("Eroare proces: %1").arg(static_cast<int>(e)));

    // Doar la FailedToStart nu urmează semnalul finished().
    if (e != QProcess::FailedToStart)
        return;

    setRunning(false);
    m_password.clear();
    if (m_automatic) {
        qCritical(logCritical()) << "Nu pot porni 7-Zip:" << sevenZipPath;
        finishAutomatic(false);
        return;
    }
    QMessageBox::critical(this,
                          tr("Eroare 7-Zip"),
                          tr("Nu pot rula 7-Zip (cod=%1).")
                              .arg(static_cast<int>(e)));
}

void ArchiveCreationHandler::appendLog(const QString &str)
{
    ui->txt_logs->append(str.trimmed().isEmpty()
                             ? QStringLiteral(" ")
                             : str.trimmed());
}

QStringList ArchiveCreationHandler::currentFileList() const
{
    QStringList files;
    files.reserve(ui->list_files->count());

    for (int i = 0; i < ui->list_files->count(); ++i) {
        const QListWidgetItem *item = ui->list_files->item(i);
        if (item->data(ConfigKindRole).toInt() == NotConfig)
            files << item->text();
    }

    return files;
}

void ArchiveCreationHandler::refreshConfigItems()
{
    // Rândurile de configurare se reconstruiesc din bife (sursa de adevăr).
    for (int i = ui->list_files->count() - 1; i >= 0; --i) {
        if (ui->list_files->item(i)->data(ConfigKindRole).toInt() != NotConfig)
            delete ui->list_files->takeItem(i);
    }

    const auto addItems = [this](const QStringList &paths, ConfigKind kind, const QString &toolTip) {
        for (const QString &path : paths) {
            auto *item = new QListWidgetItem(path, ui->list_files);
            item->setData(ConfigKindRole, kind);
            item->setToolTip(toolTip);
            QFont font = item->font();
            font.setItalic(true);
            item->setFont(font);
        }
    };

    addItems(configFileList(SettingsConfig), SettingsConfig, ui->checkIncludeSettings->text());
    addItems(configFileList(CryptoConfig), CryptoConfig, ui->checkIncludeCrypto->text());
}

QStringList ArchiveCreationHandler::configFileList() const
{
    return configFileList(SettingsConfig) + configFileList(CryptoConfig);
}

QStringList ArchiveCreationHandler::configFileList(ConfigKind kind) const
{
    // 7-Zip păstrează numele directorului: crypto/..., settings/...
    const ApplicationPathsContext &paths = ApplicationPathsContext::instance();
    QStringList candidates;

    if (kind == SettingsConfig && ui->checkIncludeSettings->isChecked()) {
        candidates << paths.uiSettingsDirectory()
                   << paths.data().settingsFilePath;
    }
    if (kind == CryptoConfig && ui->checkIncludeCrypto->isChecked())
        candidates << QDir(paths.configDirectory()).filePath(QStringLiteral("crypto"));

    QStringList files;
    for (const QString &path : std::as_const(candidates)) {
        if (!path.trimmed().isEmpty() && QFileInfo::exists(path))
            files << QDir::toNativeSeparators(QDir::cleanPath(path));
    }
    return files;
}

QString ArchiveCreationHandler::defaultArchivePath() const
{
    const MainDatabaseConnectionData connection =
        MainDatabaseConnectionContext::instance().data();

    QString databaseName = connection.sqliteDatabaseName.trimmed();
    if (databaseName.isEmpty())
        databaseName = QFileInfo(connection.sqliteDatabasePath).completeBaseName();
    if (databaseName.isEmpty())
        databaseName = OrganizationContext::instance().data().name;
    databaseName.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9._-]+")),
                         QStringLiteral("_"));

    const QString fileName =
        QStringLiteral("%1_%2.7z")
            .arg(databaseName,
                 QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss")));
    return QDir::toNativeSeparators(
        QDir(QDir::homePath()).filePath(QStringLiteral("Database_usg/") + fileName));
}

void ArchiveCreationHandler::setRunning(bool running)
{
    const bool editable = !running && !m_automatic;

    ui->archivePath->setEnabled(editable);
    ui->compression->setEnabled(editable);
    ui->list_files->setEnabled(editable);
    ui->btnAdd->setEnabled(editable);
    ui->btnRemove->setEnabled(editable);
    ui->btnClear->setEnabled(editable);
    ui->groupConfig->setEnabled(editable);
    ui->groupEncryption->setEnabled(editable);
    ui->btnStart->setEnabled(editable && !sevenZipPath.isEmpty());
    ui->btnCancel->setEnabled(running);
}

void ArchiveCreationHandler::finishAutomatic(bool success)
{
    if (! m_automatic)
        return;

    // La succes lăsăm 100% vizibil o clipă.
    QTimer::singleShot(success ? 500 : 0, this, [this, success]() {
        done(success ? QDialog::Accepted : QDialog::Rejected);
    });
}
