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

#include "catgeneral.h"
#include "ui_catgeneral.h"

#include <QFileDialog>
#include <QImageReader>
#include <QImageWriter>
#include <QStandardPaths>

#include <customs/custommessage.h>

static const int max_length_comment = 255; // lungimea maxima a cometariului

CatGeneral::CatGeneral(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CatGeneral)
{
    ui->setupUi(this);

    db    = new DataBase(this); // alocam memoria p-u salvarea datelor in BD
    popUp = new PopUp(this);    // alocam memoria p-u mesaje

    connect(this, &CatGeneral::typeCatalogChanged, this, &CatGeneral::slot_typeCatalogChanged);
    connect(this, &CatGeneral::IdChanged, this, &CatGeneral::slot_IdChanged);
    connect(this, &CatGeneral::ItNewChanged, this, &CatGeneral::slot_ItNewChanged);

    connect(ui->editFullName, &QLineEdit::textChanged, this, &CatGeneral::fullNameChanged);
    connect(ui->editFullName, &QLineEdit::editingFinished, this, &CatGeneral::fullNameSplit);
    connect(ui->editComment, &QTextEdit::textChanged, this, &CatGeneral::controlLengthComment); // limitarea caracterilor la comentariu

    ui->tabWidget->setTabVisible(1, false);

    ui->editName->setMaxLength(80);        // limitarea caracterilor
    ui->editPrenume->setMaxLength(50);
    ui->editPatronimic->setMaxLength(50);
    ui->editIDNP->setMaxLength(20);
    ui->editPoliceMed->setMaxLength(20);
    ui->editAddress->setMaxLength(255);
    ui->editTelephone->setMaxLength(100);
    ui->editEmail->setMaxLength(100);

    QList<QLineEdit*> list = this->findChildren<QLineEdit*>();
    for (int n = 0; n < list.count(); n++) {
        connect(list[n], &QLineEdit::textChanged, this, &CatGeneral::dataWasModified);
    }
    connect(ui->dateEdit, &QDateEdit::dateChanged, this, &CatGeneral::dataWasModified);
    connect(ui->editComment, &QTextEdit::textChanged, this, &CatGeneral::dataWasModified);

    ui->btnOK->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));  // comenzi rapide la tastatura
    ui->btnWrite->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    ui->btnCancel->setShortcut(QKeySequence(Qt::Key_Escape));

    connect(ui->btnOK, &QAbstractButton::clicked, this, &CatGeneral::onWritingDataClose);
    connect(ui->btnWrite, &QAbstractButton::clicked, this, &CatGeneral::onWritingData);
    connect(ui->btnCancel, &QAbstractButton::clicked, this, &CatGeneral::close);
}

CatGeneral::~CatGeneral()
{
    delete db;
    delete popUp;
    delete ui;
}

void CatGeneral::connectionModified()
{
    QList<QLineEdit*> list = this->findChildren<QLineEdit*>();
    for (int n = 0; n < list.count(); n++) {
        connect(list[n], &QLineEdit::textChanged, this, &CatGeneral::dataWasModified);
    }
    connect(ui->dateEdit, &QDateEdit::dateChanged, this, &CatGeneral::dataWasModified);
    connect(ui->editComment, &QTextEdit::textChanged, this, &CatGeneral::dataWasModified);

    connect(ui->editFullName, &QLineEdit::textChanged, this, &CatGeneral::fullNameChanged);
    connect(ui->editFullName, &QLineEdit::editingFinished, this, &CatGeneral::fullNameSplit);
}

void CatGeneral::disconnectionModified()
{
    QList<QLineEdit*> list = this->findChildren<QLineEdit*>();
    for (int n = 0; n < list.count(); n++) {
        disconnect(list[n], &QLineEdit::textChanged, this, &CatGeneral::dataWasModified);
    }
    disconnect(ui->dateEdit, &QDateEdit::dateChanged, this, &CatGeneral::dataWasModified);
    disconnect(ui->editComment, &QTextEdit::textChanged, this, &CatGeneral::dataWasModified);

    disconnect(ui->editFullName, &QLineEdit::textChanged, this, &CatGeneral::fullNameChanged);
    disconnect(ui->editFullName, &QLineEdit::editingFinished, this, &CatGeneral::fullNameSplit);
}

// *******************************************************************
// **************** INSERAREA, ACTUALIZAREA DATELOR IN TABELE ********

bool CatGeneral::controlRequiredObjects()
{
    if (ui->editFullName->text().isEmpty()){
        QMessageBox::warning(this,
                             tr("Verificarea datelor."),
                             tr("Nu este indicat \"<b>Nume, prenume, patronimic</b>\" obiectului !!!"),
                             QMessageBox::Ok);
        return false;
    }
    if (ui->editName->text().isEmpty()){
        QMessageBox::warning(this,
                             tr("Verificarea datelor."),
                             tr("Nu este indicat \"<b>Nume</b>\" obiectului !!!"),
                             QMessageBox::Ok);
        return false;
    }
    if (ui->editPrenume->text().isEmpty()){
        QMessageBox::warning(this,
                             tr("Verificarea datelor."),
                             tr("Nu este indicat \"<b>Prenume</b>\" obiectului !!!"),
                             QMessageBox::Ok);
        return false;
    }
    return true;
}

bool CatGeneral::insertDataIntoTableByNameTable(const QString name_table)
{
    QSqlQuery qry;
    if (name_table == "pacients"){

        qry.prepare(R"(
            INSERT INTO pacients (
                id, deletionMark, IDNP, name, fName, mName,
                medicalPolicy, birthday, address, telephone, email, comment
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
        )");
        qry.addBindValue(m_Id);
        qry.addBindValue(0);
        qry.addBindValue(ui->editIDNP->text());
        qry.addBindValue(ui->editName->text());
        qry.addBindValue(ui->editPrenume->text());
        qry.addBindValue(ui->editPatronimic->text().isEmpty()
                             ? QVariant()
                             : ui->editPatronimic->text());
        qry.addBindValue(ui->editPoliceMed->text().isEmpty()
                             ? QVariant()
                             : ui->editPoliceMed->text());
        qry.addBindValue(ui->dateEdit->date().toString("yyyy-MM-dd"));
        qry.addBindValue(ui->editAddress->text().isEmpty()
                             ? QVariant()
                             : ui->editAddress->text());
        qry.addBindValue(ui->editTelephone->text().isEmpty()
                             ? QVariant()
                             : ui->editTelephone->text());
        qry.addBindValue(ui->editEmail->text().isEmpty()
                             ? QVariant()
                             : ui->editEmail->text());
        qry.addBindValue(ui->editComment->toPlainText().isEmpty()
                             ? QVariant()
                             : ui->editComment->toPlainText());

    } else {

        qry.prepare("INSERT INTO " + name_table + R"( (
            id, deletionMark, name, fName, mName, telephone, email, comment
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?);
        )");
        qry.addBindValue(m_Id);
        qry.addBindValue(0);
        qry.addBindValue(ui->editName->text());
        qry.addBindValue(ui->editPrenume->text());
        qry.addBindValue(ui->editPatronimic->text().isEmpty()
                             ? QVariant()
                             : ui->editPatronimic->text());
        qry.addBindValue(ui->editTelephone->text().isEmpty()
                             ? QVariant()
                             : ui->editTelephone->text());
        qry.addBindValue(ui->editEmail->text().isEmpty()
                             ? QVariant()
                             : ui->editEmail->text());
        qry.addBindValue(ui->editComment->toPlainText().isEmpty()
                             ? QVariant()
                             : ui->editComment->toPlainText());

    }
    if (qry.exec()){

        qInfo(logInfo()) << tr("Datele obiectului '%1' cu id='%2' au fost salvate cu succes in tabela '%3'.")
                                .arg(ui->editFullName->text(), QString::number(m_Id), name_table);
        popUp->setPopupText(tr("Datele obiectului <b>'%1'</b> <br>"
                               "au fost salvate cu succes.")
                                .arg(ui->editFullName->text()));
        popUp->show();

        return true;
    } else {

        err.clear();
        err << QStringLiteral("Eroare la inserarea datelor obiectului '%1' cu id='%2' in tabela '%3' - %4")
                   .arg(ui->editFullName->text(),
                        QString::number(m_Id),
                        name_table,
                        qry.lastError().text());

        qCritical(logCritical()) << err;

        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Inserarea datelor"));
        msg->setDetailedText(err.join("\n"));
        msg->exec();
        msg->deleteLater();

        return false;
    }
}

bool CatGeneral::updateDataIntoTableByNameTable(const QString name_table)
{
    QSqlQuery qry;
    if (name_table == "pacients"){

        qry.prepare(R"(
            UPDATE pacients SET
                deletionMark  = ?,
                IDNP          = ?,
                name          = ?,
                fName         = ?,
                mName         = ?,
                medicalPolicy = ?,
                birthday      = ?,
                address       = ?,
                telephone     = ?,
                email         = ?,
                comment       = ?
            WHERE id = ?;
        )");
        qry.addBindValue(0); // deletionMark
        qry.addBindValue(ui->editIDNP->text());
        qry.addBindValue(ui->editName->text());
        qry.addBindValue(ui->editPrenume->text());
        qry.addBindValue(ui->editPatronimic->text().isEmpty()
                             ? QVariant()
                             : ui->editPatronimic->text());
        qry.addBindValue(ui->editPoliceMed->text().isEmpty()
                             ? QVariant()
                             : ui->editPoliceMed->text());
        qry.addBindValue(ui->dateEdit->date().toString("yyyy-MM-dd"));
        qry.addBindValue(ui->editAddress->text().isEmpty()
                             ? QVariant()
                             : ui->editAddress->text());
        qry.addBindValue(ui->editTelephone->text().isEmpty()
                             ? QVariant()
                             : ui->editTelephone->text());
        qry.addBindValue(ui->editEmail->text().isEmpty()
                             ? QVariant()
                             : ui->editEmail->text());
        qry.addBindValue(ui->editComment->toPlainText().isEmpty()
                             ? QVariant()
                             : ui->editComment->toPlainText());
        qry.addBindValue(m_Id);

    } else {

        qry.prepare(QStringLiteral(R"(
            UPDATE %1 SET
                deletionMark = ?,
                name         = ?,
                fName        = ?,
                mName        = ?,
                telephone    = ?,
                email        = ?,
                comment      = ?
            WHERE id = ?;
        )").arg(name_table));

        qry.addBindValue(0);
        qry.addBindValue(ui->editName->text());
        qry.addBindValue(ui->editPrenume->text());
        qry.addBindValue(ui->editPatronimic->text().isEmpty()
                             ? QVariant()
                             : ui->editPatronimic->text());
        qry.addBindValue(ui->editTelephone->text().isEmpty()
                             ? QVariant()
                             : ui->editTelephone->text());
        qry.addBindValue(ui->editEmail->text().isEmpty()
                             ? QVariant()
                             : ui->editEmail->text());
        qry.addBindValue(ui->editComment->toPlainText().isEmpty()
                             ? QVariant()
                             : ui->editComment->toPlainText());
        qry.addBindValue(m_Id);

    }

    if (qry.exec()){

        qInfo(logInfo()) << tr("Datele obiectului '%1' cu id='%2' au fost modificate cu succes in tabela '%3'.")
                                .arg(ui->editFullName->text(), QString::number(m_Id), name_table);
        popUp->setPopupText(tr("Datele obiectului <b>'%1'</b> <br>"
                               "au fost modificate cu succes.").arg(ui->editFullName->text()));
        popUp->show();

        return true;

    } else {

        err.clear();
        err << QStringLiteral("Eroare la modificarea datelor obiectului '%1' cu id='%2' in tabela '%3' - %4")
                   .arg(ui->editFullName->text(),
                        QString::number(m_Id),
                        name_table,
                        qry.lastError().text());

        qCritical(logCritical()) << err;

        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Inserarea datelor"));
        msg->setDetailedText(err.join("\n"));
        msg->exec();
        msg->deleteLater();

        return false;
    }
}

bool CatGeneral::objectExistsInTableByName(const QString name_table)
{
    QSqlQuery qry;
    qry.prepare(QStringLiteral(R"(
        SELECT
            COUNT(name)
        FROM
            %1
        WHERE
            name = ? AND
            fName = ? AND
            mName = ? AND
            deletionMark = 0;
    )").arg(name_table));
    qry.addBindValue(ui->editName->text());
    qry.addBindValue(ui->editPrenume->text());
    qry.addBindValue(ui->editPatronimic->text());
    if (qry.exec() && qry.next()){
        int _bool = qry.value(0).toInt();
        return (_bool > 0) ? true : false;
    } else {
        return false;
    }
}

bool CatGeneral::confirmIfDuplicateExist(const QString &name_table, const QString &type_label, const QString &extra_info)
{
    // verificam daca este persoana
    if (! objectExistsInTableByName(name_table))
        return true;

    // formam textul
    QString text = tr("%1 exist\304\203 \303\256n baza de date:<br>"
                      " - nume: <b>%2</b><br>"
                      " - prenume: <b>%3</b><br>"
                      " - patronimic: <b>%4</b><br>"
                      "%5 <br>"
                      "Dori\310\233i s\304\203 continua\310\233i validarea ?")
        .arg(type_label,
             ui->editName->text(),
             ui->editPrenume->text(),
             ui->editPatronimic->text(),
             extra_info);

    // prezentam mesaj
    QMessageBox messange_box(QMessageBox::Question,
                             tr("Verificarea datelor"),
                             text, QMessageBox::NoButton, this);
    QPushButton *yesButton = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
    QPushButton *noButton  = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
    yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
    noButton->setStyleSheet(db->getStyleForButtonMessageBox());
    messange_box.exec();

    return messange_box.clickedButton() == yesButton;
}

bool CatGeneral::handleInsert(const QString &name_table, const QString &type_label, const QString &extra_info)
{
    // verificam daca este duplicat
    if (! confirmIfDuplicateExist(name_table, type_label, extra_info))
        return false;

    // ne determinam cu ID
    if (m_Id <= 0)
        setId(db->getLastIdForTable(name_table) + 1);

    // inseram datele
    if (insertDataIntoTableByNameTable(name_table)) {
        // !!! logarea si mesajul de inserare cu succes vezi in [insertDataIntoTableByNameTable]

        setItNew(false); // setam itNew = false

        return true; // returnam true
    } else {
        // !!! prezentarea mesajului de eroare in functia [insertDataIntoTableByNameTable]
        return false;
    }
}

bool CatGeneral::handleUpdate(const QString &name_table, const QString &type_label)
{
    Q_UNUSED(type_label);

    if (updateDataIntoTableByNameTable(name_table)) {
        // !!! logarea si mesajul de modificarea datelor vezi in [updateDataIntoTableByNameTable]
        return true;
    } else {
        // !!! prezentarea mesajului de eroare in functia [updateDataIntoTableByNameTable]
        return false;
    }
}

// *******************************************************************
// **************** INSERAREA IMAGINEI SI ALTELE *********************

void CatGeneral::controlLengthComment()
{
    if (ui->editComment->toPlainText().length() > max_length_comment)
        ui->editComment->textCursor().deletePreviousChar();
}

void CatGeneral::dataWasModified()
{
    setWindowModified(true);
}

void CatGeneral::clearImageSignature()
{
    QSqlQuery qry;
    qry.prepare("UPDATE doctors SET signature = ? WHERE id = ?;");
    qry.addBindValue(QVariant());
    qry.addBindValue(m_Id);
    if (qry.exec()){
        ui->imageSignature->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
        ui->imageSignature->setTextFormat(Qt::RichText);
        ui->imageSignature->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        popUp->setPopupText(tr("Semnatura este eliminat din baza de date."));
        popUp->show();
    } else {

        err.clear();
        err << tr("Eroare la eliminarea semnaturei din baza de date: ")
            << qry.lastError().text();

        qCritical(logCritical()) << err;

        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Eliminarea semnaturei"));
        msg->setDetailedText(err.join("\n"));
        msg->exec();
        msg->deleteLater();
    }
}

void CatGeneral::clearImageStamp()
{
    QSqlQuery qry;
    qry.prepare("UPDATE doctors SET stamp = ? WHERE id = ?;");
    qry.addBindValue(QVariant());
    qry.addBindValue(m_Id);
    if (qry.exec()){
        ui->imageSignature->setText("<a href=\"#LoadImageStamp\">Apasa pentru a alege imaginea</a>");
        ui->imageSignature->setTextFormat(Qt::RichText);
        ui->imageSignature->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        popUp->setPopupText(tr("Imaginea este eliminat din baza de date."));
        popUp->show();
    } else {

        err.clear();
        err << tr("Eroare la eliminarea imaginei din baza de date: ")
            << qry.lastError().text();

        qCritical(logCritical()) << err;

        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Eliminarea imaginei"));
        msg->setDetailedText(err.join("\n"));
        msg->exec();
        msg->deleteLater();
    }
}

bool CatGeneral::loadFile(const QString &fileName, const QString &link)
{
    // 1. Citim imaginea cu auto-transformare
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();

    if (newImage.isNull()) {
        QMessageBox::information(this,
                                 QGuiApplication::applicationDisplayName(),
                                 tr("Nu este setată imaginea %1: %2")
                                     .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }

    // 2. Convertim imaginea într-un QPixmap scalat o singură dată
    QPixmap scaledPixmap = QPixmap::fromImage(newImage)
                               .scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 3. Setăm imaginea în QLabel potrivit
    if (link == "#LoadImage")
        ui->imageSignature->setPixmap(scaledPixmap);
    else
        ui->imageStamp->setPixmap(scaledPixmap);

    // 4. Citim fișierul binar pentru stocare în baza de date
    QFile file(fileName);
    if (! file.open(QIODevice::ReadOnly)) {
        qWarning(logWarning()) << tr("Nu s-a putut deschide fișierul pentru citire: ") << fileName;
        return true; // Nu blocăm operația chiar dacă nu-l putem salva
    }

    QByteArray inByteArray = file.readAll();
    file.close();

    // 5. Pregătim interogarea SQL
    QString column = (link == "#LoadImage") ? "signature" : "stamp";

    QSqlQuery qry;
    qry.prepare(QStringLiteral("UPDATE doctors SET %1 = ? WHERE id = ?;").arg(column));
    qry.addBindValue(inByteArray.toBase64());
    qry.addBindValue(m_Id);

    if (qry.exec()) {
        popUp->setPopupText(tr("Imaginea a fost salvată cu succes în baza de date."));
        popUp->show();
    } else {

        err.clear();
        err << tr("Eroare la salvarea imaginii în baza de date: ")
            << qry.lastError().text();

        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Inserarea imaginei"));
        msg->setDetailedText(err.join("\n"));
        msg->exec();
        msg->deleteLater();

        qWarning(logWarning()) << err;
    }

    return true;
}

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    for (const QByteArray &mimeTypeName : supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/png");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("png");
}

void CatGeneral::onLinkActivatedForOpenImage(const QString &link)
{
    if (m_Id == -1){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea validării"),
                                 tr("Pentru a încărca logotipul este necesar de salvat datele.<br>"
                                    "Doriți să salvați datele ?"),
                                 QMessageBox::NoButton, this);
        QPushButton *yesButton    = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton     = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        QPushButton *cancelButton = messange_box.addButton(tr("Anulare"), QMessageBox::RejectRole);
        yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
        noButton->setStyleSheet(db->getStyleForButtonMessageBox());
        cancelButton->setStyleSheet(db->getStyleForButtonMessageBox());
        messange_box.exec();

        if (messange_box.clickedButton() == yesButton)
            onWritingData();
        else if (messange_box.clickedButton() == noButton)
            return;
        else if (messange_box.clickedButton() == cancelButton)
            return;
    }

    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    if (dialog.exec() == QDialog::Accepted)
        if (! dialog.selectedFiles().constFirst().isEmpty())
            loadFile(dialog.selectedFiles().constFirst(), link);
    dialog.close();
}

// *******************************************************************
// **************** PROCESAREA SLOT-URILOR ***************************

void CatGeneral::slot_typeCatalogChanged()
{
    switch (m_typeCatalog) {
    case Doctors:
        ui->editPoliceMed->setVisible(false);
        ui->labelPoliceMed->setVisible(false);
        ui->editIDNP->setVisible(false);
        ui->labelIDNP->setVisible(false);
        ui->editAddress->setVisible(false);
        ui->labelAddress->setVisible(false);
        ui->dateEdit->setVisible(false);
        ui->labelDateEdit->setVisible(false);

        ui->tabWidget->setTabVisible(1, true);
        ui->imageSignature->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
        ui->imageSignature->setTextFormat(Qt::RichText);
        ui->imageSignature->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

        ui->imageStamp->setText("<a href=\"#LoadImageStamp\">Apasa pentru a alege imaginea</a>");
        ui->imageStamp->setTextFormat(Qt::RichText);
        ui->imageStamp->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

        connect(ui->imageSignature, QOverload<const QString &>::of(&QLabel::linkActivated), this,
                QOverload<const QString &>::of(&CatGeneral::onLinkActivatedForOpenImage));
        connect(ui->imageStamp, QOverload<const QString &>::of(&QLabel::linkActivated), this,
                QOverload<const QString &>::of(&CatGeneral::onLinkActivatedForOpenImage));
        connect(ui->btnClearSignature, &QToolButton::clicked, this, &CatGeneral::clearImageSignature);
        connect(ui->btnClearStamp, &QToolButton::clicked, this, &CatGeneral::clearImageStamp);
        break;
    case Nurses:
        ui->editPoliceMed->setVisible(false);
        ui->labelPoliceMed->setVisible(false);
        ui->editIDNP->setVisible(false);
        ui->labelIDNP->setVisible(false);
        ui->editAddress->setVisible(false);
        ui->labelAddress->setVisible(false);
        ui->dateEdit->setVisible(false);
        ui->labelDateEdit->setVisible(false);
        break;
    case Pacients:
        break;
    default:
        qWarning(logWarning()) << this->metaObject()->className()
            << tr(": nu a fost determinata proprietatea 'typeCatalog' !!!");
        break;
    }
    slot_ItNewChanged();
}

void CatGeneral::slot_IdChanged()
{
    if (m_itNew)
        return;

    // deconectarea de modificarea formei
    disconnectionModified();

    // container cu sectiile pu solicitare
    //   - aceste sectii sunt comune
    QVariantMap map_column {
        { "name",  QVariant() },
        { "fName", QVariant() },
        { "mName", QVariant() },
        { "telephone", QVariant() },
        { "email", QVariant() },
        { "comment", QVariant() }
    };

    // container cu conditia
    QMap<QString, QVariant> where; // conditia este unica pentru
    where["id"] = m_Id;            // fiecare catalog

    QString class_name = this->metaObject()->className();
    QVariantMap map_result;
    err.clear();

    QString table;

    switch (m_typeCatalog) {
    case Doctors:
        // sectii specifice
        map_column["signature"] = QVariant();
        map_column["stamp"]     = QVariant();
        table = "doctors";
        break;
    case Nurses:
        table = "nurses";
        break;
    case Pacients:
        // sectii specifice
        map_column["IDNP"]          = QVariant();
        map_column["birthday"]      = QVariant();
        map_column["medicalPolicy"] = QVariant();
        table = "pacients";
        break;
    default:
        qWarning(logWarning()) << class_name
                               << tr(": nu a fost determinată proprietatea 'typeCatalog' !!!");
        return;
    }

    map_result = db->selectSingleRow(class_name, table, map_column, where, err);

    if (! map_result.isEmpty()) {
        if (m_typeCatalog == Doctors) {
            // signature
            QByteArray arr_signature = QByteArray::fromBase64(map_result["signature"].toByteArray());
            QPixmap pix_signature;
            if (! arr_signature.isEmpty() && pix_signature.loadFromData(arr_signature)) {
                ui->imageSignature->setPixmap(pix_signature.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }
            // stamp
            QByteArray arr_stamp = QByteArray::fromBase64(map_result["stamp"].toByteArray());
            QPixmap pix_stamp;
            if (! arr_stamp.isEmpty() && pix_stamp.loadFromData(arr_stamp)) {
                ui->imageStamp->setPixmap(pix_stamp.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }
        }

        if (m_typeCatalog == Pacients) {
            ui->editIDNP->setText(map_result["IDNP"].toString());
            ui->editPoliceMed->setText(map_result["medicalPolicy"].toString());
            ui->dateEdit->setDate(QDate::fromString(map_result["birthday"].toString(), "yyyy-MM-dd"));
        }

        // valori comune
        ui->editName->setText(map_result["name"].toString());
        ui->editPrenume->setText(map_result["fName"].toString());
        ui->editPatronimic->setText(map_result["mName"].toString());
        ui->editTelephone->setText(map_result["telephone"].toString());
        ui->editEmail->setText(map_result["email"].toString());
        ui->editComment->setText(map_result["comment"].toString());
        ui->editFullName->setText(ui->editName->text() + " " + ui->editPrenume->text() + " " + ui->editPatronimic->text());

    } else if (! err.isEmpty()) {
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Inserarea datelor"));
        msg->setDetailedText(err.join("\n"));
        msg->exec();
        msg->deleteLater();
    }

    ui->editName->setFocus(); // modifica forma - isWindowModified() = true
                              // cand este focusat initial pe fullName
    setFullName(ui->editFullName->text());

    // conectarea la modificarea formei
    connectionModified();
}

void CatGeneral::slot_ItNewChanged()
{
    if (m_itNew){
        switch (m_typeCatalog) {
        case Doctors:
            setWindowTitle(tr("Doctor (crearea) %1").arg("[*]"));
            break;
        case Nurses:
            setWindowTitle(tr("As.medicală (crearea) %1").arg("[*]"));
            break;
        case Pacients:
            setWindowTitle(tr("Pacient (crearea) %1").arg("[*]"));
            break;
        default:
            break;
        }
    } else {
        switch (m_typeCatalog) {
        case Doctors:
            setWindowTitle(tr("Doctor (salvat) %1").arg("[*]"));
            break;
        case Nurses:
            setWindowTitle(tr("As.medicală (salvată) %1").arg("[*]"));
            break;
        case Pacients:
            setWindowTitle(tr("Pacient (salvat) %1").arg("[*]"));
            break;
        default:
            break;
        }
    }
}

void CatGeneral::fullNameChanged()
{
    QString strFullName = ui->editFullName->text();
    if (firstSpace == 0){
        QString n1 = strFullName.right(1);
        if (n1 == " "){
            strName = strFullName.left(strFullName.size() - 1);
            firstSpace = 1;
        } else {
            strName = strFullName.right(strFullName.size());
        }
    } else if (firstSpace == 1){
        QString n2 = strFullName.right(1);
        if (n2 == " "){
            strPrenume = strFullName.right(strFullName.size() - strName.size() - 1);
            firstSpace = 2;
        } else {
            strPrenume = strFullName.right(strFullName.size() - (strName.size() + 1));
        }
    } else {
        strPatrimonic = strFullName.right(strFullName.size() - (strName.size() + 1) - strPrenume.size());
    }

    ui->editName->setText(strName);
    ui->editPrenume->setText(strPrenume);
    ui->editPatronimic->setText(strPatrimonic);
    setFullName(ui->editFullName->text());
}

void CatGeneral::fullNameSplit()
{
    QString strFullName = ui->editFullName->text();
    static const QRegularExpression reg_listStr("\\s+");
    QStringList listStr = strFullName.split(reg_listStr);

    int n;

    ui->editName->clear();
    ui->editPrenume->clear();
    ui->editPatronimic->clear();

    for (n = 0; n < listStr.count(); ++ n) {
        if (n == 0){
            if (ui->editName->text() != listStr[n]){
                ui->editName->setText(listStr[n]);
            }
        } else if (n == 1){
            if (ui->editPrenume->text() != listStr[n]){
                ui->editPrenume->setText(listStr[n]);
            }
        } else if (n == 2){
            if (ui->editPatronimic->text() != listStr[n]){
                ui->editPatronimic->setText(listStr[n]);
            }
        }
    }
}

// *******************************************************************
// **************** VALIDAREA DATELOR ********************************

bool CatGeneral::onWritingData()
{
    if (! controlRequiredObjects())
        return false;

    bool returnBool;

    switch (m_typeCatalog) {
    case Doctors:
        returnBool = m_itNew
                         ? handleInsert("doctors", tr("Doctor"), "")
                         : handleUpdate("doctors", tr("Doctor"));
        break;
    case Nurses:
        returnBool = m_itNew
                         ? handleInsert("nurses", tr("As.medicala"), "")
                         : handleUpdate("nurses", tr("As.medcala"));
        break;
    case Pacients:
        returnBool = m_itNew
            ? handleInsert("pacients", tr("Pacientul"), tr(" - anul na\310\231terii: %1")
                                                        .arg(ui->dateEdit->date().toString("dd.MM.yyyy")))
                         : handleUpdate("pacients", tr("Pacientul"));
        break;
    default:
        qWarning(logWarning()) << this->metaObject()->className()
                               << tr(": nu a fost determinanta proprietatea 'typeCatalog' !!!");
        returnBool = false;
        break;
    }

    // emitem signal
    if (returnBool) {
        if (m_itNew)
            emit createCatGeneral();
        else
            emit changedCatGeneral();
    }

    // modificam forma
    setWindowModified(! returnBool); // schimbam proprietatea de modificare a datelor formai

    return returnBool;
}

void CatGeneral::onWritingDataClose()
{
   if (onWritingData())
       QDialog::accept();
}

// *******************************************************************
// **************** EVENIMENTELE FORMEI ******************************

void CatGeneral::closeEvent(QCloseEvent *event)
{
    if (isWindowModified()){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Datele au fost modificate.\n"
                                    "Dori\310\233i s\304\203 salva\310\233i aceste modific\304\203ri ?"),
                                 QMessageBox::NoButton, this);
        QPushButton *yesButton    = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton     = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        QPushButton *cancelButton = messange_box.addButton(tr("Anulare"), QMessageBox::RejectRole);
        yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
        noButton->setStyleSheet(db->getStyleForButtonMessageBox());
        cancelButton->setStyleSheet(db->getStyleForButtonMessageBox());
        messange_box.exec();

        if (messange_box.clickedButton() == yesButton) {
            onWritingDataClose();
            event->accept();
        } else if (messange_box.clickedButton() == noButton) {
            event->accept();
        } else if (messange_box.clickedButton() == cancelButton) {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void CatGeneral::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        // traducem  titlu
        setWindowTitle(tr("Creiază utilizator nou %1").arg("[*]"));
    }
}

void CatGeneral::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Return || event->key() == Qt::Key_Enter){
      this->focusNextChild();
    }
}
