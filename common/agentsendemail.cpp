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

#include "agentsendemail.h"
#include "ui_agentsendemail.h"

#include <QMessageBox>
#include <QProcess>
#include <QThread>

AgentSendEmail::AgentSendEmail(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AgentSendEmail)
{
    ui->setupUi(this);

    setWindowTitle(tr("Agentul e-mail"));

    db = new DataBase(this);

    filesAttachments.clear();

    connect(this, &AgentSendEmail::NrOrderChanged, this, &AgentSendEmail::slot_NrOrderChanged);
    connect(this, &AgentSendEmail::NrReportChanged, this, &AgentSendEmail::slot_NrReportChanged);
    connect(this, &AgentSendEmail::EmailFromChanged, this, &AgentSendEmail::slot_EmailFromChanged);
    connect(this, &AgentSendEmail::EmailToChanged, this, &AgentSendEmail::slot_EmailToChanged);
    connect(this, &AgentSendEmail::DateInvestigationChanged, this, &AgentSendEmail::slot_DateInvestigationChanged);
    connect(this, &AgentSendEmail::NameReportChanged, this, &AgentSendEmail::slot_NameReportChanged);

    fileInputs["file1"] = ui->attached_file1;
    fileInputs["file2"] = ui->attached_file2;
    fileInputs["file3"] = ui->attached_file3;
    fileInputs["file4"] = ui->attached_file4;
    fileInputs["file5"] = ui->attached_file5;

    for (auto it = fileInputs.begin(); it != fileInputs.end(); ++it) {
        connect(it.value(), &LineEditOpen::onClickedButton, this, [this, key = it.key()]() {
            onOpenFile(key);
        });
    }

    imgInputs["img1"] = ui->attached_img1;
    imgInputs["img2"] = ui->attached_img2;
    imgInputs["img3"] = ui->attached_img3;
    imgInputs["img4"] = ui->attached_img4;
    imgInputs["img5"] = ui->attached_img5;

    for (auto it = imgInputs.begin(); it != imgInputs.end(); ++it) {
        connect(it.value(), &LineEditOpen::onClickedButton, this, [this, key = it.key()]() {
            onOpenFile(key);
        });
    }

    connect(ui->btnSend, &QPushButton::clicked, this, &AgentSendEmail::onSend);
    connect(ui->btnClose, &QPushButton::clicked, this, &AgentSendEmail::onClose);
}

AgentSendEmail::~AgentSendEmail()
{
    delete db;
    delete ui;
}

// **********************************************************************************
// --- procesarea proprietatilor

QString AgentSendEmail::getNrOrder()
{
    return m_nrOrder;
}

void AgentSendEmail::setNrOrder(QString NrOrder)
{
    m_nrOrder = NrOrder;
    emit NrOrderChanged();
}

QString AgentSendEmail::getNrReport()
{
    return m_nrReport;
}

void AgentSendEmail::setNrReport(QString NrReport)
{
    m_nrReport = NrReport;
    emit NrReportChanged();
}

QString AgentSendEmail::getEmailFrom()
{
    return m_emailFrom;
}

void AgentSendEmail::setEmailFrom(QString EmailFrom)
{
    m_emailFrom = EmailFrom;
    emit EmailFromChanged();
}

QString AgentSendEmail::getEmailTo()
{
    return m_emailTo;
}

void AgentSendEmail::setEmailTo(QString EmailTo)
{
    m_emailTo = EmailTo;
    emit EmailToChanged();
}

QString AgentSendEmail::getNamePatient()
{
    return m_namePatient;
}

void AgentSendEmail::setNamePatient(QString NamePatient)
{
    m_namePatient = NamePatient;
    emit NamePatientChanged();
}

QString AgentSendEmail::getNameDoctor()
{
    return m_nameDoctor;
}

void AgentSendEmail::setNameDoctor(QString NameDoctor)
{
    m_nameDoctor = NameDoctor;
    emit NameDoctorChanged();
}

QDate AgentSendEmail::getDateInvestigation()
{
    return m_dateInvestigation;
}

void AgentSendEmail::setDateInvestigation(QDate DateInvestigation)
{
    m_dateInvestigation = DateInvestigation;
    emit DateInvestigationChanged();
}

bool AgentSendEmail::getThisReports()
{
    return m_thisReports;
}

void AgentSendEmail::setThisReports(bool ThisReports)
{
    m_thisReports = ThisReports;
    emit ThisReportsChanged();
}

QString AgentSendEmail::getNameReport()
{
    return m_nameReport;
}

void AgentSendEmail::setNameReport(QString NameReport)
{
    m_nameReport = NameReport;
    emit NameReportChanged();
}

// **********************************************************************************
// --- procesarea slot-lor

void AgentSendEmail::slot_NrOrderChanged()
{
    if (m_nrOrder.isEmpty())
        return;
    QString file_order = globals().main_path_save_documents + "/Comanda_ecografica_nr_" + m_nrOrder + ".pdf";
    if (QFile(file_order).exists()) {
        ui->attached_file1->setText(file_order);
        filesAttachments << file_order;
    }
}

void AgentSendEmail::slot_NrReportChanged()
{
    if (m_nrReport.isEmpty())
        return;

    // cautam fisierele - Raport_ecografic_nr
    // cautam fisierele - Comanda_ecografica_nr
    QDir dir(globals().main_path_save_documents);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QFileInfoList listFiles = dir.entryInfoList();

    QList<QFileInfo> matchingFiles;
    QList<QFileInfo> matchingFilesImg;

    // Iterăm folosind un iterator explicit pentru a evita copia accidentală
    for (auto it = listFiles.constBegin(); it != listFiles.constEnd(); ++it) {
        if (it->fileName().contains("Raport_ecografic_nr_" + m_nrReport)) {
            matchingFiles.append(*it);
        }
    }

    for (auto it = listFiles.constBegin(); it != listFiles.constEnd(); ++it) {
        if (it->fileName().contains("Image_report_" + m_nrReport)) {
            matchingFilesImg.append(*it);
        }
    }

    QVector<QLineEdit*> attachedFiles = {
        ui->attached_file1,
        ui->attached_file2,
        ui->attached_file3,
        ui->attached_file4,
        ui->attached_file5
    };

    QVector<QLineEdit*> attachedFilesImage = {
        ui->attached_img1,
        ui->attached_img2,
        ui->attached_img3,
        ui->attached_img4,
        ui->attached_img5
    };

    int n = 0;
    // Dacă avem fișiere care îndeplinesc condiția, le procesăm
    if (! matchingFiles.isEmpty()) {
        for (const QFileInfo &fileInfo : matchingFiles) {
            if (n < attachedFiles.size()) {
                attachedFiles[n + 1]->setText(fileInfo.absoluteFilePath());
                filesAttachments << fileInfo.absoluteFilePath();
                ++n;
            } else {
                break; // Evităm accesul invalid dacă sunt mai multe fișiere decât lineedit
            }
        }
    }

    n = 0;
    // Dacă avem fișiere care îndeplinesc condiția, le procesăm
    if (! matchingFilesImg.isEmpty()) {
        for (const QFileInfo &fileInfo : matchingFilesImg) {
            if (n < attachedFilesImage.size()) {
                attachedFilesImage[n]->setText(fileInfo.absoluteFilePath());
                filesAttachments << fileInfo.absoluteFilePath();
                ++n;
            } else {
                break; // Evităm accesul invalid dacă sunt mai multe fișiere decât lineedit
            }
        }
    }
}

void AgentSendEmail::slot_EmailFromChanged()
{
    if (m_emailFrom.isEmpty())
        return;
    ui->txt_from->setText(m_emailFrom);
}

void AgentSendEmail::slot_EmailToChanged()
{
    if (m_emailTo.isEmpty())
        return;
    ui->txt_to->setText(m_emailTo);
}

void AgentSendEmail::slot_DateInvestigationChanged()
{
    if (m_thisReports){
        ui->txt_subiect->setText("Rapoarte investigațiilor ecografice");
        ui->txt_body->append("Către " + m_namePatient + ".");
        ui->txt_body->append("Vă transmitem alăturat raportul medical " + m_nameReport + ".");
        ui->txt_body->append("Rapoarte atașate:");
        ui->txt_body->append(" - " + m_nameReport + " în format PDF.");
        ui->txt_body->append("");
        ui->txt_body->append("Vă rugăm să confirmați primirea acestuia și să ne contactați pentru orice informații suplimentare.");
        ui->txt_body->append("");
        ui->txt_body->append("Cu stimă,");
        ui->txt_body->append("" + m_nameDoctor + " / " + globals().main_name_organization + "");
        ui->txt_body->append("Telefon: " + globals().main_phone_organization + "");
        ui->txt_body->append("E-mail: " + globals().main_email_organization + "");
    } else {
        m_subiect = "Rezultatul investigației ecografice";
        ui->txt_subiect->setText(m_subiect);

        ui->txt_body->append("Stimate/Stimată " + m_namePatient + ".");
        ui->txt_body->append("Vă transmitem raportul ecografic în urma investigației efectuate la " +
                             globals().main_name_organization + " pe data de " + m_dateInvestigation.toString("dd.MM.yyyy") + ".");
        ui->txt_body->append("Documente atașate:");
        ui->txt_body->append(" - Comanda ecografică în format PDF.");
        ui->txt_body->append(" - Rapoarte ecografice în format PDF");
        ui->txt_body->append("");
        ui->txt_body->append("Observații importante:");
        ui->txt_body->append("Dacă aveți întrebări legate de rezultatul investigației sau doriți o consultație suplimentară, ");
        ui->txt_body->append("vă rugăm să ne contactați la " + globals().main_phone_organization + " sau să ne scrieți la " + globals().main_email_organization + ".");
        ui->txt_body->append("");
        ui->txt_body->append("Vă mulțumim pentru încrederea acordată!");
        ui->txt_body->append("Cu stimă,");
        ui->txt_body->append("" + m_nameDoctor + " / " + globals().main_name_organization + "");
        ui->txt_body->append("Telefon: " + globals().main_phone_organization + "");
        ui->txt_body->append("E-mail: " + globals().main_email_organization + "");
    }

    m_body = ui->txt_body->toPlainText();

    QSqlQuery qry;
    qry.prepare(R"(
        SELECT
            contsOnline.email,
            contsOnline.smtp_server,
            contsOnline.port,
            contsOnline.username,
            contsOnline.password,
            contsOnline.iv,
            users.hash AS hashUser
        FROM
            contsOnline
        INNER JOIN
            users ON users.id = contsOnline.id_users
        WHERE
            contsOnline.id_organizations = ? AND
            contsOnline.id_users = ? AND
            contsOnline.email = ?
    )");
    qry.addBindValue(globals().c_id_organizations);
    qry.addBindValue(globals().idUserApp);
    qry.addBindValue(m_emailFrom);
    if (! qry.exec()) {
        qWarning(logWarning()) << "SQL Error:" << qry.lastError().text();
        return;
    } else {
        if (qry.next()) {
            QSqlRecord rec = qry.record();
            m_smtp_server = qry.value(rec.indexOf("smtp_server")).toString();
            m_port = qry.value(rec.indexOf("port")).toInt();

            QByteArray hash_user         = QByteArray::fromHex(qry.value(rec.indexOf("hashUser")).toString().toUtf8());
            QByteArray iv                = QByteArray::fromBase64(qry.value(rec.indexOf("iv")).toString().toUtf8());
            QByteArray encryptedPassword = QByteArray::fromBase64(qry.value(rec.indexOf("password")).toString().toUtf8());
            QByteArray decryptedPassword = crypto_manager->decryptPassword(encryptedPassword, hash_user, iv);
            m_password = QString::fromUtf8(decryptedPassword);
        }
    }
}

void AgentSendEmail::slot_NameReportChanged()
{
    if (m_nameReport.isEmpty())
        return;
    QString file_report = globals().main_path_save_documents + "/" + m_nameReport + ".pdf";
    if (QFile(file_report).exists()){
        ui->attached_file1->setText(file_report);
        filesAttachments << file_report;
    }
}

void AgentSendEmail::onOpenFile(const QString type_file)
{
    if (! fileInputs.contains(type_file) && ! imgInputs.contains(type_file)) {
        qWarning() << "Unknown file type: " << type_file;
        return;
    }

    QString filePath;
    if (type_file.contains("file"))
        filePath = fileInputs[type_file]->text();
    else if (type_file.contains("img"))
        filePath = imgInputs[type_file]->text();

    if (filePath.isEmpty()) {
        qWarning() << "No file path provided for: " << type_file;
        return;
    }

    openFile(filePath);
}

void AgentSendEmail::openFile(const QString &filePath)
{
    QString osType = QSysInfo::productType();

    if (osType == "windows") {
        QProcess::startDetached("explorer", { QDir::toNativeSeparators(filePath) });
    }
    else if (osType == "macos") {
        QProcess::startDetached("open", { filePath });
    }
    else if (osType.contains("linux") || osType.contains("ubuntu") || osType.contains("debian")) {
        QProcess::startDetached("xdg-open", { filePath });
    }
    else {
        qWarning() << "Unsupported OS: " << osType;
    }
}

void AgentSendEmail::onSend()
{
    if (ui->txt_from->text().isEmpty()){
        QMessageBox::warning(this,
                             tr("Verificarea"),
                             tr("Nu este indicat e-mail beneficiarului !!!"),
                             QMessageBox::Ok);
        return;
    }

    if (ui->txt_to->text().isEmpty()){
        QMessageBox::warning(this,
                             tr("Verificarea"),
                             tr("Nu este indicat e-mail destinatarului !!!"),
                             QMessageBox::Ok);
        return;
    }

    // 1. Creăm și afișăm loaderul
    loader = new ProcessingAction(this);
    loader->setAttribute(Qt::WA_DeleteOnClose);
    loader->setProperty("txtInfo", "Se transmit documentele destinatarului ...");
    loader->show();

    // 2️. Ascundem fereastra principală
    this->hide();

    qInfo(logInfo()) << "[THREAD] Se initializeaza trimiterea datelor investigatiilor catre e-mail"
                     << ui->txt_to->text();

    // 3️. Creăm thread-ul pentru trimiterea emailului
    QThread *thread = new QThread();
    EmailCore *email_core = new EmailCore();

    // 4️. Mutăm `email_core` în thread-ul nou
    email_core->moveToThread(thread);

    // 5️. Configurăm datele pentru email
    email_core->setEmailData(m_smtp_server,
                             m_port,
                             ui->txt_from->text(),
                             globals().main_email_organization,
                             m_password,
                             ui->txt_to->text(),
                             m_subiect,
                             m_body,
                             filesAttachments);

    // 6️. Conectăm semnalele și sloturile
    connect(thread, &QThread::started, email_core, &EmailCore::sendEmail);
    connect(email_core, &EmailCore::emailSent, this, &AgentSendEmail::onEmailSent);
    connect(email_core, &EmailCore::emailSent, thread, &QThread::quit);
    connect(email_core, &EmailCore::emailSent, email_core, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    // 7️. Pornim thread-ul
    thread->start();
}

void AgentSendEmail::onEmailSent(bool success)
{
    if (success) {
        qInfo(logInfo()) << "[THREAD] E-mail trimis cu succes !";
    } else {
        qCritical(logCritical()) << "[THREAD] Eroare la trimiterea e-mail-lui !";
    }

    // Închidem loader-ul
    if (loader) {
        loader->close();
    }

    // Afișăm din nou fereastra principală
    onClose();
}

void AgentSendEmail::onClose()
{
    QDir dir(globals().main_path_save_documents);
    if (dir.exists()) {
        if (dir.removeRecursively()) {
            qInfo(logInfo()) << "[THREAD] Directorul" << globals().main_path_save_documents << " a fost șters cu succes !";
        } else {
            qWarning(logWarning()) << "[THREAD] Eroare: Nu s-a putut șterge directorul - " << globals().main_path_save_documents;
        }
    } else {
        qWarning(logWarning()) << "[THREAD] Directorul nu există - " << globals().main_path_save_documents;
    }
    this->close();
}
