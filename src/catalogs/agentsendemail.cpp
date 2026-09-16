#include "agentsendemail.h"
#include "ui_agentsendemail.h"
#include <data/database_common.h>

AgentSendEmail::AgentSendEmail(DataBase &db, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AgentSendEmail)
    , m_db(db)
{
    ui->setupUi(this);

    const QString schema = globals().thisMySQL
        ? QStringLiteral(":/sql/mariadb/tables/online_account.sql")
        : QStringLiteral(":/sql/sqlite/tables/online_account.sql");
    DataBaseCommon::execFileBatch(m_db.getDatabase(), schema, "onlineAccount");

    setWindowTitle(tr("Agentul e-mail"));

    initModelAccount();
    initEditorsMap();
    initConnections();
}

AgentSendEmail::~AgentSendEmail()
{
    delete ui;
}

void AgentSendEmail::setContext(const MailContext &context)
{
    m_ctx = context;

    if (!loadOnlineAccountSettings(false))
        selectFirstAvailableAccount();

    buildMessage();
    collectAttachments();
    fillUiFromContext();
}

const AgentSendEmail::MailContext &AgentSendEmail::context() const
{
    return m_ctx;
}

void AgentSendEmail::initEditorsMap()
{
    fileInputs["file1"] = ui->attached_file1;
    fileInputs["file2"] = ui->attached_file2;
    fileInputs["file3"] = ui->attached_file3;
    fileInputs["file4"] = ui->attached_file4;
    fileInputs["file5"] = ui->attached_file5;

    imgInputs["img1"] = ui->attached_img1;
    imgInputs["img2"] = ui->attached_img2;
    imgInputs["img3"] = ui->attached_img3;
    imgInputs["img4"] = ui->attached_img4;
    imgInputs["img5"] = ui->attached_img5;
}

void AgentSendEmail::initConnections()
{
    for (auto it = fileInputs.begin(); it != fileInputs.end(); ++it) {
        connect(it.value(), &LineEditOpen::onClickedButton,
                this, [this, key = it.key()]() { onOpenFile(key); });
    }

    for (auto it = imgInputs.begin(); it != imgInputs.end(); ++it) {
        connect(it.value(), &LineEditOpen::onClickedButton,
                this, [this, key = it.key()]() { onOpenFile(key); });
    }

    connect(ui->btnSend, &QPushButton::clicked,
            this, &AgentSendEmail::onSend, Qt::UniqueConnection);
    connect(ui->btnClose, &QPushButton::clicked,
            this, &AgentSendEmail::onClose, Qt::UniqueConnection);
}

bool AgentSendEmail::loadOnlineAccountSettings(bool logFailure)
{
    QString error;
    QByteArray realKey;

    QSqlDatabase base = m_db.getDatabase();
    if (!CryptoManager::loadOrCreateSplitKey(base,
                                             globals().c_id_organizations,
                                             &realKey,
                                             &error)) {
        qWarning(logWarning()) << "Nu s-a putut incarca cheia split:" << error;
        return false;
    }

    QSqlQuery qry(base);
    qry.prepare(R"(
        SELECT
            onlineAccount.email,
            onlineAccount.smtp_server,
            onlineAccount.port,
            onlineAccount.username,
            onlineAccount.password,
            onlineAccount.iv,
            onlineAccount.tag
        FROM
            onlineAccount
        WHERE
            id_organizations = :id_organizations AND
            id_users = :id_users AND
            email = :email
        LIMIT 1
    )");
    qry.bindValue(":id_organizations", globals().c_id_organizations);
    qry.bindValue(":id_users", globals().idUserApp);
    qry.bindValue(":email", m_ctx.emailFrom.trimmed());

    if (!qry.exec()) {
        if (logFailure)
            qWarning(logWarning()) << "SQL Error:" << qry.lastError().text();
        return false;
    }

    if (!qry.next()) {
        if (logFailure)
            qWarning(logWarning()) << "Nu a fost gasit onlineAccount pentru email:"
                                   << m_ctx.emailFrom;
        return false;
    }

    const QSqlRecord rec = qry.record();

    m_ctx.emailFrom  = qry.value(rec.indexOf("email")).toString();
    m_ctx.smtpServer = qry.value(rec.indexOf("smtp_server")).toString();
    m_ctx.port       = qry.value(rec.indexOf("port")).toInt();
    m_ctx.username   = qry.value(rec.indexOf("username")).toString();

    CryptoManager::EncryptedData data;
    data.cipherText = CryptoManager::fromBase64(qry.value(rec.indexOf("password")).toString());
    data.iv         = CryptoManager::fromBase64(qry.value(rec.indexOf("iv")).toString());
    data.tag        = CryptoManager::fromBase64(qry.value(rec.indexOf("tag")).toString());

    bool okDecrypt = false;
    const QByteArray decryptedPassword =
        CryptoManager::decryptText(data, realKey, &okDecrypt);

    if (!okDecrypt) {
        qWarning(logWarning()) << "Nu s-a putut decripta parola onlineAccount";
        return false;
    }

    m_ctx.password = QString::fromUtf8(decryptedPassword);
    return true;
}

void AgentSendEmail::selectFirstAvailableAccount()
{
    if (!modelAccount)
        return;

    const int roleEmail = modelAccount->roleForColumn("email");
    for (int row = 0; row < modelAccount->rowCount(); ++row) {
        const QString email = modelAccount->index(row, 0).data(roleEmail).toString().trimmed();
        if (email.isEmpty())
            continue;

        ui->comboAccount->setCurrentIndex(row);
        m_ctx.emailFrom = email;
        loadOnlineAccountSettings();
        return;
    }

    qWarning(logWarning()) << "Nu există un cont e-mail configurat pentru organizația și utilizatorul curent.";
}

void AgentSendEmail::buildMessage()
{
    QStringList lines;

    if (m_ctx.thisReports) {
        m_ctx.subject = tr("Rapoarte investigațiilor ecografice");

        lines << tr("Către %1.").arg(m_ctx.namePatient);
        lines << tr("Vă transmitem alăturat raportul medical %1.").arg(m_ctx.nameReport);
        lines << tr("Rapoarte atașate:");
        lines << tr(" - %1 în format PDF.").arg(m_ctx.nameReport);
        lines << "";
        lines << tr("Vă rugăm să confirmați primirea acestuia și să ne contactați pentru orice informații suplimentare.");
        lines << "";
        lines << tr("Cu stimă,");
        lines << QString("%1 / %2").arg(m_ctx.nameDoctor, globals().main_name_organization);
        lines << tr("Telefon: %1").arg(globals().main_phone_organization);
        lines << tr("E-mail: %1").arg(globals().main_email_organization);
    } else {
        m_ctx.subject = tr("Rezultatul investigației ecografice");

        lines << tr("Stimate/Stimată %1.").arg(m_ctx.namePatient);
        lines << tr("Vă transmitem raportul ecografic în urma investigației efectuate la %1 pe data de %2.")
                     .arg(globals().main_name_organization,
                          m_ctx.dateInvestigation.toString("dd.MM.yyyy"));
        lines << tr("Documente atașate:");
        lines << tr(" - Comanda ecografică în format PDF.");
        lines << tr(" - Rapoarte ecografice în format PDF.");
        lines << "";
        lines << tr("Observații importante:");
        lines << tr("Dacă aveți întrebări legate de rezultatul investigației sau doriți o consultație suplimentară,");
        lines << tr("vă rugăm să ne contactați la %1 sau să ne scrieți la %2.")
                     .arg(globals().main_phone_organization,
                          globals().main_email_organization);
        lines << "";
        lines << tr("Vă mulțumim pentru încrederea acordată!");
        lines << tr("Cu stimă,");
        lines << QString("%1 / %2").arg(m_ctx.nameDoctor, globals().main_name_organization);
        lines << tr("Telefon: %1").arg(globals().main_phone_organization);
        lines << tr("E-mail: %1").arg(globals().main_email_organization);
    }

    m_ctx.body = lines.join('\n');
}

void AgentSendEmail::collectAttachments()
{
    m_ctx.attachments.clear();

    for (LineEditOpen *ed : std::as_const(fileInputs))
        ed->clear();

    for (LineEditOpen *ed : std::as_const(imgInputs))
        ed->clear();

    if (!m_ctx.nrOrder.isEmpty()) {
        QString fileNumber = m_ctx.nrOrder;
        fileNumber.replace('/', '_');
        fileNumber.replace('\\', '_');
        fileNumber.replace(':', '_');
        const QString fileOrder =
            globals().main_path_save_documents + "/Comanda_ecografica_nr_" + fileNumber + ".pdf";

        if (QFile::exists(fileOrder)) {
            ui->attached_file1->setText(fileOrder);
            m_ctx.attachments << fileOrder;
        }
    }

    if (!m_ctx.nameReport.isEmpty()) {
        const QString fileReport =
            globals().main_path_save_documents + "/" + m_ctx.nameReport + ".pdf";

        if (QFile::exists(fileReport)) {
            if (ui->attached_file1->text().isEmpty())
                ui->attached_file1->setText(fileReport);
            else if (ui->attached_file2->text().isEmpty())
                ui->attached_file2->setText(fileReport);

            if (!m_ctx.attachments.contains(fileReport))
                m_ctx.attachments << fileReport;
        }
    }

    if (m_ctx.nrReport.isEmpty())
        return;

    QString reportFileNumber = m_ctx.nrReport;
    reportFileNumber.replace('/', '_');
    reportFileNumber.replace('\\', '_');
    reportFileNumber.replace(':', '_');

    QDir dir(globals().main_path_save_documents);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    const QFileInfoList listFiles = dir.entryInfoList();

    QList<QFileInfo> matchingFiles;
    QList<QFileInfo> matchingImages;

    for (auto it = listFiles.constBegin(); it != listFiles.constEnd(); ++it) {
        if (it->fileName().contains("Raport_ecografic_nr_" + reportFileNumber))
            matchingFiles.append(*it);

        if (it->fileName().contains("Image_report_" + reportFileNumber))
            matchingImages.append(*it);
    }

    QVector<LineEditOpen*> attachedFiles = {
        ui->attached_file1,
        ui->attached_file2,
        ui->attached_file3,
        ui->attached_file4,
        ui->attached_file5
    };

    QVector<LineEditOpen*> attachedImages = {
        ui->attached_img1,
        ui->attached_img2,
        ui->attached_img3,
        ui->attached_img4,
        ui->attached_img5
    };

    int n = 0;
    for (const QFileInfo &fi : matchingFiles) {
        while (n < attachedFiles.size() && !attachedFiles[n]->text().isEmpty())
            ++n;

        if (n >= attachedFiles.size())
            break;

        attachedFiles[n]->setText(fi.absoluteFilePath());
        if (!m_ctx.attachments.contains(fi.absoluteFilePath()))
            m_ctx.attachments << fi.absoluteFilePath();
        ++n;
    }

    n = 0;
    for (const QFileInfo &fi : matchingImages) {
        if (n >= attachedImages.size())
            break;

        attachedImages[n]->setText(fi.absoluteFilePath());
        if (!m_ctx.attachments.contains(fi.absoluteFilePath()))
            m_ctx.attachments << fi.absoluteFilePath();
        ++n;
    }
}

void AgentSendEmail::refreshAttachmentsFromEditors()
{
    m_ctx.attachments.clear();

    auto appendFile = [this](LineEditOpen *editor) {
        const QString filePath = editor->text().trimmed();
        if (!filePath.isEmpty() && QFileInfo::exists(filePath)
            && !m_ctx.attachments.contains(filePath))
            m_ctx.attachments.append(filePath);
    };

    for (LineEditOpen *editor : std::as_const(fileInputs))
        appendFile(editor);
    for (LineEditOpen *editor : std::as_const(imgInputs))
        appendFile(editor);
}

void AgentSendEmail::fillUiFromContext()
{
    ui->txt_from->setText(m_ctx.emailFrom);
    ui->txt_to->setText(m_ctx.emailTo);
    ui->txt_subiect->setText(m_ctx.subject);
    ui->txt_body->clear();
    ui->txt_body->setPlainText(m_ctx.body);
}

void AgentSendEmail::onOpenFile(const QString &typeFile)
{
    if (!fileInputs.contains(typeFile) && !imgInputs.contains(typeFile)) {
        qWarning(logWarning()) << "Tip fisier necunoscut:" << typeFile;
        return;
    }

    QString filePath;
    if (fileInputs.contains(typeFile))
        filePath = fileInputs.value(typeFile)->text();
    else
        filePath = imgInputs.value(typeFile)->text();

    if (filePath.isEmpty()) {
        qWarning(logWarning()) << "Nu exista cale de fisier pentru:" << typeFile;
        return;
    }

    qInfo(logInfo()) << "Deschiderea atasamentului:" << filePath;
    openFile(filePath);
}

void AgentSendEmail::openFile(const QString &filePath)
{
    const QString osType = QSysInfo::productType();

    if (osType == "windows") {
        QProcess::startDetached("explorer", {QDir::toNativeSeparators(filePath)});
    } else if (osType == "macos") {
        QProcess::startDetached("open", {filePath});
    } else if (osType.contains("linux")
               || osType.contains("ubuntu")
               || osType.contains("debian")) {
        // Vizualizatorul PDF (de ex. Okular) este tot o aplicatie Qt. Nu-i
        // transmitem variabilele Qt ale USG/Qt Creator: acestea pot forta
        // incarcarea unor pluginuri incompatibile cu Qt-ul sistemului.
        QProcess opener;
        const QProcessEnvironment systemEnvironment = QProcessEnvironment::systemEnvironment();
        QProcessEnvironment cleanEnvironment;
        const QStringList variablesToKeep = {
            QStringLiteral("HOME"),
            QStringLiteral("USER"),
            QStringLiteral("LOGNAME"),
            QStringLiteral("PATH"),
            QStringLiteral("LANG"),
            QStringLiteral("LC_ALL"),
            QStringLiteral("LC_CTYPE"),
            QStringLiteral("DISPLAY"),
            QStringLiteral("WAYLAND_DISPLAY"),
            QStringLiteral("XDG_RUNTIME_DIR"),
            QStringLiteral("XDG_CURRENT_DESKTOP"),
            QStringLiteral("DBUS_SESSION_BUS_ADDRESS")
        };
        for (const QString &variable : variablesToKeep) {
            if (systemEnvironment.contains(variable))
                cleanEnvironment.insert(variable, systemEnvironment.value(variable));
        }

        opener.setProcessEnvironment(cleanEnvironment);
        opener.setProgram(QStringLiteral("/usr/bin/xdg-open"));
        opener.setArguments({filePath});
        // Procesul este independent. Mesajele lui nu fac parte din jurnalul
        // USG si nu trebuie sa ramana atasate consolei Qt Creator.
        opener.setStandardOutputFile(QProcess::nullDevice());
        opener.setStandardErrorFile(QProcess::nullDevice());
        if (!opener.startDetached())
            qWarning(logWarning()) << "Nu s-a putut porni xdg-open pentru:" << filePath;
    } else {
        qWarning(logWarning()) << "OS nesuportat:" << osType;
    }
}

void AgentSendEmail::onSend()
{
    if (ui->txt_from->text().trimmed().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Verificarea"),
                             tr("Nu este indicat e-mail beneficiarului !!!"),
                             QMessageBox::Ok);
        return;
    }

    if (ui->txt_to->text().trimmed().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Verificarea"),
                             tr("Nu este indicat e-mail destinatarului !!!"),
                             QMessageBox::Ok);
        return;
    }

    m_ctx.emailFrom = ui->txt_from->text().trimmed();
    if (!loadOnlineAccountSettings()) {
        QMessageBox::warning(this,
                             tr("Verificarea"),
                             tr("Nu au putut fi încărcate datele contului SMTP selectat."),
                             QMessageBox::Ok);
        return;
    }

    refreshAttachmentsFromEditors();
    if (m_ctx.attachments.isEmpty()) {
        QMessageBox::warning(this,
                             tr("Verificarea"),
                             tr("Nu există documente exportate pentru atașare."),
                             QMessageBox::Ok);
        return;
    }

    loader = new ProcessingAction(this);
    loader->setAttribute(Qt::WA_DeleteOnClose);
    loader->setProperty("txtInfo", tr("Se transmit documentele destinatarului ..."));
    loader->show();

    hide();

    qInfo(logInfo()) << "[THREAD] Se initializeaza trimiterea emailului catre"
                     << ui->txt_to->text();

    QThread *thread = new QThread(this);
    EmailCore *emailCore = new EmailCore();
    emailCore->moveToThread(thread);

    emailCore->setEmailData(m_ctx.smtpServer,
                            m_ctx.port,
                            ui->txt_from->text().trimmed(),
                            m_ctx.username,
                            m_ctx.password,
                            ui->txt_to->text().trimmed(),
                            ui->txt_subiect->text().trimmed(),
                            ui->txt_body->toPlainText(),
                            m_ctx.attachments);

    connect(thread, &QThread::started,
            emailCore, &EmailCore::sendEmail);
    connect(emailCore, &EmailCore::emailSent,
            this, &AgentSendEmail::onEmailSent);
    connect(emailCore, &EmailCore::emailSent,
            thread, &QThread::quit);
    connect(emailCore, &EmailCore::emailSent,
            emailCore, &QObject::deleteLater);
    connect(thread, &QThread::finished,
            thread, &QObject::deleteLater);

    thread->start();
}

void AgentSendEmail::onEmailSent(bool success)
{
    if (success)
        qInfo(logInfo()) << "[THREAD] E-mail trimis cu succes!";
    else
        qCritical(logCritical()) << "[THREAD] Eroare la trimiterea e-mail-ului!";

    if (loader)
        loader->close();

    onClose();
}

void AgentSendEmail::onClose()
{
    QDir dir(globals().main_path_save_documents);
    if (dir.exists()) {
        if (dir.removeRecursively()) {
            qInfo(logInfo()) << "[THREAD] Directorul"
                             << globals().main_path_save_documents
                             << "a fost sters cu succes!";
        } else {
            qWarning(logWarning()) << "[THREAD] Nu s-a putut sterge directorul:"
                                   << globals().main_path_save_documents;
        }
    }

    close();
}

void AgentSendEmail::initModelAccount()
{
    if (modelAccount)
        delete modelAccount;

    QSqlQuery query(m_db.getDatabase());
    query.prepare(R"(
        SELECT
            id,
            username,
            email
        FROM
            onlineAccount
        WHERE
            id_organizations = ? AND
            id_users = ?
        ORDER BY
            username
    )");
    query.addBindValue(globals().c_id_organizations);
    query.addBindValue(globals().idUserApp);

    if (!query.exec()) {
        qWarning() << "SQL Error:" << query.lastError().text();
        return;
    }

    modelAccount = new QueryRolesModel(QString(), this);
    modelAccount->setQuery(std::move(query));
    modelAccount->setEmptyRowEnabled(true);

    ui->comboAccount->setModel(modelAccount);
    ui->comboAccount->setModelColumn(modelAccount->columnIndex("username"));

    const int roleEmail = modelAccount->roleForColumn("email");

    connect(ui->comboAccount, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this, roleEmail](int index)
            {
                Q_UNUSED(index)

                const QString email = ui->comboAccount->currentData(roleEmail).toString();

                if (email.isEmpty())
                    ui->txt_from->clear();
                else {
                    ui->txt_from->setText(email);
                    m_ctx.emailFrom = email;
                    loadOnlineAccountSettings();
                }
            });
}
