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

#include "catorganizations.h"
#include "ui_catorganizations.h"

#include <QBuffer>
#include <QImageReader>
#include <QImageWriter>

#include <customs/custommessage.h>

static const int max_length_comment = 255; // lungimea maxima a cometariului

CatOrganizations::CatOrganizations(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CatOrganizations)
{
    ui->setupUi(this);

    // setam titlul feresteri
    setWindowTitle(tr("Organizația %1").arg("[*]"));

    // alocarea memoriei
    db    = new DataBase(this);  // alocam memoria pu conectarea cu BD
    popUp = new PopUp(this);     // alocam memoria p-u mesaje

    // controlam lungimea maxima permisa a textului
    ui->editName->setMaxLength(100);
    ui->editIDNP->setMaxLength(15);
    ui->editTVA->setMaxLength(10);
    ui->editAddress->setMaxLength(255);
    ui->editTelephone->setMaxLength(100);
    ui->editEmail->setMaxLength(100);

    // initierea modelului de contracte
    QString strQuery = "";
    modelCantract = new BaseSqlQueryModel(strQuery, this); // initierea modelului pu contracte
    updateTableContracts();                                // completarea tabelei cu contracte

    // initializam imaginea stampilei
    ui->img_stamp->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
    ui->img_stamp->setTextFormat(Qt::RichText);
    ui->img_stamp->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

    // conectarile
    initConnections();     // initializam conectarile si procesarea lor
    connectionsModified(); // initializam conectarile de modificare a formei
    initBtnToolBar();      // initializam butoanele tool barului tabelei contracte

    // comenzi rapide la tastatura
    ui->btnOK->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));
    ui->btnWrite->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    ui->btnCancel->setShortcut(QKeySequence(Qt::Key_Escape));
}

CatOrganizations::~CatOrganizations()
{
    delete modelCantract;
    delete db;
    delete popUp;
    delete ui;
}

// *******************************************************************
// **************** PROCESAREA SLOT-URILOR SI ALTELE *****************

void CatOrganizations::controlLengthComment()
{
    if (ui->editComment->toPlainText().length() > max_length_comment)
        ui->editComment->textCursor().deletePreviousChar();
}

void CatOrganizations::dataWasModified()
{
    setWindowModified(true);
}

void CatOrganizations::slot_ItNewChanged()
{
    if (m_itNew){
        setWindowTitle(tr("Organizația (crearea) %1").arg("[*]"));
    } else {
        setWindowTitle(tr("Organizația (salvată) %1").arg("[*]"));
    }
}

void CatOrganizations::slot_IdChanged()
{
    if (m_itNew)
        return;

    // deconectarea la modificarea formei
    disconnectionsModified();

    // indicam toate sectiile
    QVariantMap map;
    map["*"] = QVariant();

    // pregatim conditia
    QMap<QString, QVariant> where;
    where["id"] = m_Id;

    // extragem datele si completam campurile
    QVariantMap map_result = db->selectSingleRow(this->metaObject()->className(), "organizations", map, where, err);
    if (map_result.count() > 0) {
        ui->editName->setText(map_result["name"].toString());
        ui->editIDNP->setText(map_result["IDNP"].toString());
        ui->editTVA->setText(map_result["TVA"].toString());
        ui->editAddress->setText(map_result["address"].toString());
        ui->editTelephone->setText(map_result["telephone"].toString());
        ui->editEmail->setText(map_result["email"].toString());
        ui->editComment->setText(map_result["comment"].toString());

        // contract implicit
        int _id_contract = map_result["id_contracts"].toInt();
        if (_id_contract > 0)
            setIdMainContract(_id_contract); // setam proprietatea m_IdMainContract

        // stampila
        QByteArray arr_stamp = QByteArray::fromBase64(map_result["stamp"].toByteArray());
        QPixmap pix_stamp;
        if (! arr_stamp.isEmpty() && pix_stamp.loadFromData(arr_stamp)){
            ui->img_stamp->setPixmap(pix_stamp.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }

    // extragem datele tabelei de contracte
    updateTableContracts();

    // conectarea la modificarea formei
    connectionsModified();
}

void CatOrganizations::textChangedNameOrganization()
{
    setNameOrganization(ui->editName->text());
}

// *******************************************************************
// **************** INSERAREA IMAGINEI SI ALTELE *********************

void CatOrganizations::clearImageStamp()
{
    QSqlQuery qry;
    qry.prepare("UPDATE organizations SET stamp = ? WHERE id = ?;");
    qry.addBindValue(QVariant());
    qry.addBindValue(m_Id);
    if (qry.exec()){
        ui->img_stamp->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
        ui->img_stamp->setTextFormat(Qt::RichText);
        ui->img_stamp->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        popUp->setPopupText(tr("Imaginea este eliminată din baza de date."));
        popUp->show();
        globals().main_stamp_organization = nullptr;
    } else {
        // formam textul erorii
        err.clear();
        err << this->metaObject()->className()
            << "[clearImageStamp]"
            << tr("Eroare la eliminarea imaginei din baza de date %1")
                   .arg(qry.lastError().text().isEmpty()
                            ? ": eroarea indisponibila"
                            : ": " + qry.lastError().text());
        // logarea
        qCritical(logCritical()) << err;
        // prezentam textul erorii
        CustomMessage *message = new CustomMessage(this);
        message->setWindowTitle(QGuiApplication::applicationDisplayName());
        message->setTextTitle(tr("Eliminarea imaginei nu s-a efectuat !!!"));
        message->setDetailedText(err.join("\n"));
        message->exec();
        message->deleteLater();
    }
}

bool CatOrganizations::loadFile(const QString &fileName)
{
    // 1. Citim imaginea cu auto-transformare
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();

    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Nu este setată imaginea %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }

    // 2. Convertim imaginea într-un QPixmap scalat o singură dată
    QPixmap scaledPixmap = QPixmap::fromImage(newImage)
                               .scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 3. Setăm imaginea în QLabel potrivit
    ui->img_stamp->setPixmap(scaledPixmap);

    // 4. Citim fișierul binar pentru stocare în baza de date
    QFile file(fileName);
    if (! file.open(QIODevice::ReadOnly)) {
        qWarning(logWarning()) << tr("Nu s-a putut deschide fișierul pentru citire: ") << fileName;
        return true; // Nu blocăm operația chiar dacă nu-l putem salva
    }

    // 5. pregatim byteArray
    QByteArray inByteArray = file.readAll();
    file.close();

    // 6. actualizam datele organizatiei
    QSqlQuery qry;
    qry.prepare("UPDATE organizations SET stamp = ? WHERE id = ?");
    qry.addBindValue(inByteArray.toBase64());
    qry.addBindValue(m_Id);
    if (qry.exec()){
        popUp->setPopupText(tr("Imaginea este salvat cu succes în baza de date."));
        popUp->show();
        globals().main_stamp_organization = QByteArray::fromBase64(inByteArray.toBase64());
        qInfo(logInfo()) << QStringLiteral("A fost inserata imaginea organizatiei %1").arg(ui->editName->text());
    } else {

        err.clear();
        err << this->metaObject()->className()
            << "[loadFile]"
            << tr("Eroare la salvarea imaginii în baza de date %1")
                   .arg(qry.lastError().text().isEmpty()
                            ? ": eroarea indisponibila"
                            : ": " + qry.lastError().text());

        qWarning(logWarning()) << err;

        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Inserarea imaginei nu s-a efectuat !!!"));
        msg->setDetailedText(err.join("\n"));
        msg->exec();
        msg->deleteLater();
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

void CatOrganizations::onLinkActivatedForOpenImage(const QString &link)
{
    if (link != "#LoadImage")
        return;

    if (m_Id == -1){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea valid\304\203rii"),
                                 tr("Pentru a \303\256nc\304\203rca imagine este necesar de salvat datele.<br>"
                                    "Dori\310\233i s\304\203 salva\310\233i datele ?"),
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
            loadFile(dialog.selectedFiles().constFirst());
    dialog.close();
}

// *******************************************************************
// **************** PROCESAREA SLOT-URILOR ***************************

void CatOrganizations::createNewContract()
{
    // verificam daca este nou sau modificat
    if (m_itNew || isWindowModified()){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Datele obiectului <b>%1</b> nu sunt salvate.<br>"
                                    "Dori\310\233i s\304\203 salva\310\233i datele ?").arg(ui->editName->text()),
                                 QMessageBox::NoButton, this);
        QPushButton *yesButton    = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton     = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
        noButton->setStyleSheet(db->getStyleForButtonMessageBox());
        messange_box.exec();

        if (messange_box.clickedButton() == yesButton) {
            if (! onWritingData())
                return;
        } else if (messange_box.clickedButton() == noButton) {
            return;
        }
    }

    // verificam ID
    if (m_Id == -1){

        err.clear();
        err << this->metaObject()->className()
            << "[createNewContract]"
            << tr("Nu este determinat 'id' organizatiei.");

        qWarning(logWarning()) << err;

        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Crearea contractului nu este posibila !!!"));
        msg->setDetailedText(err.join("\n"));
        msg->exec();
        msg->deleteLater();

    }

    // deschidem forma contractului
    catContracts = new CatContracts(this);
    catContracts->setAttribute(Qt::WA_DeleteOnClose);
    catContracts->setProperty("itNew", true);
    catContracts->setProperty("IdOrganization", getId());
    catContracts->show();
    connect(catContracts, &CatContracts::createNewContract, this, &CatOrganizations::slot_updateTableContracts);
}

void CatOrganizations::editContract()
{
    // verificam daca sunt modificari a formei
    if (isWindowModified()){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Datele obiectului <b>%1</b> nu sunt salvate.<br>"
                                    "Dori\310\233i s\304\203 salva\310\233i datele ?").arg(ui->editName->text()),
                                 QMessageBox::NoButton, this);
        QPushButton *yesButton    = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton     = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
        noButton->setStyleSheet(db->getStyleForButtonMessageBox());
        messange_box.exec();

        if (messange_box.clickedButton() == yesButton) {
            if (!onWritingData())
                return;
        } else if (messange_box.clickedButton() == noButton) {
            return;
        }
    }

    // verificam daca este ales randul
    if (ui->tableViewContracts->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Atenție"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    // anuntam variabile necesare
    int current_row = ui->tableViewContracts->currentIndex().row();
    int _id = modelCantract->data(modelCantract->index(current_row, sectionsContract_id), Qt::DisplayRole).toInt();

    // deschidem forma
    catContracts = new CatContracts(this);
    catContracts->setAttribute(Qt::WA_DeleteOnClose);
    catContracts->setProperty("itNew", false);
    catContracts->setProperty("Id", _id);
    connect(catContracts, &CatContracts::changedCatContract, this, [this]()
    {
        slot_updateTableContracts();
        popUp->setPopupText(tr("Datele contractului obiectului <b>'%1'</b><br>"
                               "au fost modificate cu succes.")
                                .arg(catContracts->getNameParentContract()));
        popUp->show();
    });
    catContracts->show();
}

void CatOrganizations::markDeletionContract()
{
    // 1. determinam randul curent
    int currentRow = ui->tableViewContracts->currentIndex().row();
    if (currentRow == -1){
        QMessageBox::warning(this, tr("Atenție"),
                             tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    // 2. anuntam variabile si le completam din tabela
    int _id = modelCantract->data(modelCantract->index(currentRow, sectionsQry_id), Qt::DisplayRole).toInt();
    QString _nameContract = modelCantract->data(modelCantract->index(currentRow, sectionsQry_name), Qt::DisplayRole).toString();

    // 3. pregatim solicitarea
    QString strQry;
    strQry = QStringLiteral(R"(
        SELECT
            numberDoc,
            dateDoc,
            'Comanda ecografica' AS typeDoc
        FROM
            orderEcho
        WHERE
            id_organizations = ? AND
            id_contracts = ?
        UNION ALL
        SELECT
            numberDoc,
            dateDoc,
            'Formarea preturilor' AS typeDoc
        FROM
            pricings
        WHERE
            id_organizations = ? AND
            id_contracts = ?
        ORDER BY
            dateDoc DESC
    )");

    // 4. executam
    QSqlQuery qry;
    qry.prepare(strQry);
    qry.addBindValue(m_Id); // ID organizatiei pu orderEcho
    qry.addBindValue(_id);  // ID contractului pu orderEcho
    qry.addBindValue(m_Id); // ID organizatiei pu pricings
    qry.addBindValue(_id);  // ID contractului pu pricings
    if (qry.exec()) {

        QStringList list_doc;
        list_doc << tr("Contractul '%1' organizatiei '%2' figureaza in urmatoarele documente:")
                        .arg(_nameContract, ui->editName->text());
        while (qry.next()) {
            QString doc_type = qry.value(2).toString();
            QString nr_doc   = qry.value(0).toString();

            QString rawDate  = qry.value(1).toString();
            QString date_doc = db->formatDatabaseDate(rawDate);

            list_doc << tr(" - %1 nr.%2 din %3").arg(doc_type, nr_doc, date_doc);
        }

        if (list_doc.size() > 1) { // > 1 din cauza initierea textului vezi mai sus
            CustomMessage *message = new CustomMessage(this);
            message->setWindowTitle(QGuiApplication::applicationDisplayName());
            message->setTextTitle(tr("Contractul '%1' nu poate fi eliminat !!!").arg(_nameContract));
            message->setDetailedText(list_doc.join("\n"));
            message->exec();
            message->deleteLater();

            return;
        }

    } else {

        err.clear();
        err << this->metaObject()->className()
            << "[markDeletionContract]"
            << tr("Eroarea solicitarii de selectare a contractului %1")
                   .arg(qry.lastError().text().isEmpty()
                            ? ": eroarea indisponibila"
                            : ": " + qry.lastError().text());

        qCritical(logCritical()) << err;

        CustomMessage *message = new CustomMessage(this);
        message->setWindowTitle(QGuiApplication::applicationDisplayName());
        message->setTextTitle(tr("Contractul '%1' nu poate fi eliminat !!!").arg(_nameContract));
        message->setDetailedText(err.join("\n"));
        message->exec();
        message->deleteLater();

        return;
    }

    // 5. daca ID contractului ales din tabel = cu ID contractului implicit
    // actualizam datele organizatiei - modificam id_contracts = QVariant()
    if (_id == m_IdMainContract) {

        QVariantMap map;
        map["id_contracts"] = QVariant();

        QMap<QString, QVariant> where;
        where["id"] = m_Id;

        if (db->updateTable(this->metaObject()->className(), "organizations", map, where, err)) {
            // daca a fost eliminat contractul -> setam contractul implicit -1
            setIdMainContract(-1);
        } else {
            CustomMessage *message = new CustomMessage(this);
            message->setWindowTitle(QGuiApplication::applicationDisplayName());
            message->setTextTitle(tr("Contractul '%1' nu poate fi eliminat !!!").arg(_nameContract));
            message->setDetailedText(err.join("\n"));
            message->exec();
            message->deleteLater();

            return;
        }

    }

    // 6. eliminam contractul
    if (db->deleteDataFromTable("contracts", "id", QString::number(_id))) {
        popUp->setPopupText(tr("Contractul '%1' organizatiei %2 <br>"
                               "eliminat cu cucces din baza de date.")
                            .arg(_nameContract, ui->editName->text()));
        popUp->show();

        qInfo(logInfo()) << "Eliminat contractul " + _nameContract + " din baza de date a organizatiei " + ui->editName->text();

        updateTableContracts();
    }
}

void CatOrganizations::setDefaultContract()
{
    // verificam daca este ales contractul
    int currentRow = ui->tableViewContracts->currentIndex().row();
    if (currentRow == -1){
        QMessageBox::warning(this, tr("Atenție"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    // determinam ID si numele contractului
    int _id               = modelCantract->data(modelCantract->index(currentRow, sectionsQry_id), Qt::DisplayRole).toInt();
    QString _nameContract = modelCantract->data(modelCantract->index(currentRow, sectionsQry_name), Qt::DisplayRole).toString();

    err.clear(); // curatim erorile

    // verificam daca este determinat corect ID organizatiei
    if (m_Id <= 0) {
        err << this->metaObject()->className()
            << "[setDefaultContract]"
            << "Nu este determinat ID organizatiei - ID organizatiei nu poate fi <=0 !!!";
        CustomMessage *message = new CustomMessage(this);
        message->setWindowTitle(QGuiApplication::applicationDisplayName());
        message->setTextTitle(tr("Contractul '%1' nu poate fi setat ca implicit !!!").arg(_nameContract));
        message->setDetailedText(err.join("\n"));
        message->exec();
        message->deleteLater();
        return;
    }

    // verificam daca este determinat corect ID contractului
    if (_id <= 0) {
        err << this->metaObject()->className()
            << "[setDefaultContract]"
            << "Nu este determinat corect ID contractului - ID contractului nu poate fi <=0 !!!";
        CustomMessage *message = new CustomMessage(this);
        message->setWindowTitle(QGuiApplication::applicationDisplayName());
        message->setTextTitle(tr("Contractul '%1' nu poate fi setat ca implicit !!!").arg(_nameContract));
        message->setDetailedText(err.join("\n"));
        message->exec();
        message->deleteLater();
        return;
    }

    // pregatim valorile ce trebuie sa fie modificate
    QVariantMap map;
    map["id_contracts"] = _id;

    // pregatim conditia
    QMap<QString, QVariant> where;
    where["id"] = m_Id;

    // actualizam datele organizatiei
    if (db->updateTable(this->metaObject()->className(), "organizations", map, where, err)) {
        setIdMainContract(_id); // setam contract implicit
        updateTableContracts(); // actualizam tabela cu contracte
        // prezentam mesaj
        popUp->setPopupText(tr("Contractul <b>'%1'</b><br> a fost setat implicit.").arg(_nameContract));
        popUp->show();
    } else {
        if (err.size() > 0){
            CustomMessage *message = new CustomMessage(this);
            message->setWindowTitle(QGuiApplication::applicationDisplayName());
            message->setTextTitle(tr("Contractul '%1' nu poate fi setat ca implicit !!!").arg(_nameContract));
            message->setDetailedText(err.join("\n"));
            message->exec();
            message->deleteLater();
        }
    }
}

void CatOrganizations::slot_updateTableContracts()
{
    updateTableContracts();
}

void CatOrganizations::onDoubleClickedTableContracts(const QModelIndex &index)
{
    Q_UNUSED(index)
    editContract();
}

// *******************************************************************
// **************** VALIDAREA DATELOR ********************************

bool CatOrganizations::onWritingData()
{
    // verificam campurile obligatorii
    if (! controlRequiredObjects())
        return false;

    // anuntam variabila returnarii
    bool returnBool;

    // procesarea inserarii si actualizarii
    returnBool = m_itNew
                     ? handleInsert()
                     : handleUpdate();

    if (m_itNew)
        setItNew(false); // setam ca nu este nou

    // modificarea formei
    if (returnBool)
        setWindowModified(false);

    return returnBool;
}

void CatOrganizations::onWritingDataClose()
{
    if (onWritingData())
        QDialog::accept();
}

// *******************************************************************
// **************** CONECTARILE SI PROCESAREA LOR ********************

void CatOrganizations::initConnections()
{
    connect(ui->img_stamp, QOverload<const QString &>::of(&QLabel::linkActivated), this,
            QOverload<const QString &>::of(&CatOrganizations::onLinkActivatedForOpenImage));
    connect(ui->btnClearStamp, &QToolButton::clicked, this, &CatOrganizations::clearImageStamp);

    connect(this, &CatOrganizations::IdChanged, this, &CatOrganizations::slot_IdChanged);
    connect(this, &CatOrganizations::ItNewChanged, this, &CatOrganizations::slot_ItNewChanged);

    connect(ui->editName, &QLineEdit::textChanged, this, &CatOrganizations::textChangedNameOrganization);
    connect(ui->editComment, &QTextEdit::textChanged, this, &CatOrganizations::controlLengthComment);

    connect(ui->tableViewContracts, &QTableView::doubleClicked, this, &CatOrganizations::onDoubleClickedTableContracts);

    connect(ui->btnOK, &QAbstractButton::clicked, this, &CatOrganizations::onWritingDataClose);
    connect(ui->btnWrite, &QAbstractButton::clicked, this, &CatOrganizations::onWritingData);
    connect(ui->btnCancel, &QAbstractButton::clicked, this, &CatOrganizations::close);
}

void CatOrganizations::connectionsModified()
{
    QList<QLineEdit*> list = this->findChildren<QLineEdit*>();
    for (int n = 0; n < list.count(); n++) {
        connect(list[n], &QLineEdit::textChanged, this, &CatOrganizations::dataWasModified);
    }

    connect(ui->editComment, &QTextEdit::textChanged, this, &CatOrganizations::dataWasModified);
}

void CatOrganizations::disconnectionsModified()
{
    QList<QLineEdit*> list = this->findChildren<QLineEdit*>();
    for (int n = 0; n < list.count(); n++) {
        disconnect(list[n], &QLineEdit::textChanged, this, &CatOrganizations::dataWasModified);
    }

    disconnect(ui->editComment, &QTextEdit::textChanged, this, &CatOrganizations::dataWasModified);
}

void CatOrganizations::initBtnToolBar()
{
    connect(ui->btnAddContract, &QToolButton::clicked, this, &CatOrganizations::createNewContract);
    connect(ui->btnEditContract, &QToolButton::clicked, this, &CatOrganizations::editContract);
    connect(ui->btnDeletionContract, &QToolButton::clicked, this, &CatOrganizations::markDeletionContract);
    connect(ui->btnSetDefaultContract, &QToolButton::clicked, this, &CatOrganizations::setDefaultContract);
}

// *******************************************************************
// **************** INSERAREA, ACTUALIZAREA DATELOR IN TABELE ********

void CatOrganizations::updateTableContracts()
{
    QString strQuery = "";
    if (m_Id > 0)
        strQuery = QStringLiteral(R"(
            SELECT
                id,
                deletionMark,
                dateInit,
                name
            FROM
                contracts
            WHERE
                id_organizations = %1

        )").arg(QString::number(m_Id));

    modelCantract->setQuery(strQuery);
    if (m_IdMainContract > 0)
        modelCantract->setIdContract(m_IdMainContract);  // transmitem ID contractului implicit

    ui->tableViewContracts->setModel(modelCantract);                              // setam model in tabela
    ui->tableViewContracts->hideColumn(sectionsQry_id);                           // id-ascundem
    ui->tableViewContracts->horizontalHeader()->setStretchLastSection(true);      // extinderea ultimei sectiei
    ui->tableViewContracts->setSelectionBehavior(QAbstractItemView::SelectRows);  // setam alegerea randului
    ui->tableViewContracts->setSelectionMode(QAbstractItemView::SingleSelection); // setam singura alegerea(nu multipla)
    ui->tableViewContracts->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // permitem schimbarea size sectiilor
    ui->tableViewContracts->verticalHeader()->setDefaultSectionSize(12); // modificam inaltimea randului
    ui->tableViewContracts->setColumnWidth(sectionsQry_deletionMark, 5); // modificam latimea primei sectiei cu imaginea
    ui->tableViewContracts->setFocus();    // focusam la tabela contracte
    ui->tableViewContracts->selectRow(0);  // selectam 1 rand

    updateHeaderTableContracts();
}

void CatOrganizations::updateHeaderTableContracts()
{
    modelCantract->setHeaderData(sectionsQry_deletionMark, Qt::Horizontal, tr(""));
    modelCantract->setHeaderData(sectionsQry_dateInit,     Qt::Horizontal, tr("Data încep."));
    modelCantract->setHeaderData(sectionsQry_name,         Qt::Horizontal, tr("Denumirea contractului"));
}

// *******************************************************************
// **************** VALIDAREA, ACTUALIZAREA DATELOR ******************

bool CatOrganizations::controlRequiredObjects()
{
    if (ui->editName->text().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Verificarea datelor."),
                             tr("Nu este indicat\304\203 \"<b>Denumirea organiza\310\233iei</b>\" !!!"),
                             QMessageBox::Ok);
        return false;
    }

    if (ui->editIDNP->text().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Verificarea datelor."),
                             tr("Nu este indicat \"<b>IDNO</b>\" !!!"),
                             QMessageBox::Ok);
        return false;
    }

    return true;
}

bool CatOrganizations::confirmIfDuplcateExist()
{
    QSqlQuery qry;
    qry.prepare(R"(
        SELECT
            count(name)
        FROM
            organizations
        WHERE
            name = ?
            AND deletionMark = 0
    )");
    qry.addBindValue(ui->editName->text());
    if (qry.exec() && qry.next()){
        int _bool = qry.value(0).toInt();
        return (_bool > 0) ? true : false;
    } else {
        return false;
    }
}

bool CatOrganizations::handleInsert()
{
    // verificam daca este organizatia cu acelasi nume
    if (confirmIfDuplcateExist()) {
        QString text = tr("Organiza\310\233ia cu denumirea <b>%1</b><br>"
                          "exist\304\203 \303\256n baza de date !!!<br>"
                          "Dori\310\233i s\304\203 continua\310\233i ?")
            .arg(ui->editName->text());
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 text, QMessageBox::NoButton, this);
        QPushButton *yesButton = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton  = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
        noButton->setStyleSheet(db->getStyleForButtonMessageBox());
        messange_box.exec();
        if (messange_box.clickedButton() == noButton)
            return false;
    }

    // verificam ID
    if (m_Id <= 0)
        setId(db->getLastIdForTable("organizations") + 1);

    // inseram datele
    if (insertIntoTableOrganizations()) {
        // !!! logarea si prezentarea erorii se efectuiaza in [insertIntoTableOrganizations]

        // prezentam mesaj
        popUp->setPopupText(tr("Datele organizatiei <b>%1</b> <br>"
                               "au fost inserate cu succes.")
                            .arg(ui->editName->text()));
        popUp->show();
        // emitem signal ca a fost inscris organizatia noua
        emit mWriteNewOrganization();
        return true;
    } else {
        setId(-1);
        return false;
    }
}

bool CatOrganizations::handleUpdate()
{
    if (updateDataTableOrganizations()) {
        // prezentam mesaj
        popUp->setPopupText(tr("Datele organizatiei <b>%1</b> <br>"
                               "au fost modificate cu succes.")
                                .arg(ui->editName->text()));
        popUp->show();
        // emitem signal ca dateleorganizatiei au fost modificate
        emit mChangedDateOrganization();
        return true;
    } else {
        return false;
    }
}

bool CatOrganizations::insertIntoTableOrganizations()
{   
    QSqlQuery qry;
    qry.prepare(R"(
        INSERT INTO organizations (
            id,
            deletionMark,
            IDNP,
            TVA,
            name,
            address,
            telephone,
            email,
            comment,
            id_contracts,
            stamp )
        VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    qry.addBindValue(m_Id);
    qry.addBindValue(0);
    qry.addBindValue(ui->editIDNP->text());
    qry.addBindValue(ui->editTVA->text().isEmpty()
                         ? QVariant()
                         : ui->editTVA->text());
    qry.addBindValue(ui->editName->text());
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
    qry.addBindValue((m_IdMainContract <= 0)
                         ? QVariant()
                         : m_IdMainContract);

    QByteArray image_data;
    QPixmap pix = ui->img_stamp->pixmap();
    if (! pix.isNull()) {
        QBuffer buffer(&image_data);
        buffer.open(QIODevice::WriteOnly);
        pix.save(&buffer, "PNG");
    }
    qry.addBindValue(image_data.isEmpty()
                         ? QVariant(QMetaType(QMetaType::QByteArray))
                         : image_data.toBase64());

    if (qry.exec()) {
        qInfo(logInfo()) << tr("Au fost inserate datele organizatiei '%1' cu ID='%2' in baza de date.")
                                .arg(ui->editName->text(), QString::number(m_Id));
        return true;
    } else {
        // formam eroarea
        err.clear();
        err << this->metaObject()->className()
            << "[insertIntoTableOrganizations]"
            << QStringLiteral("Eroare la inserarea datelor organizatiei %1 %2")
                   .arg(ui->editName->text(),
                        qry.lastError().text().isEmpty()
                            ? ": eroarea indisponibila"
                            : ": " + qry.lastError().text());

        // logarea
        qCritical(logCritical()) << err;

        // prezentarea mesajului cu eroare
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Inserarea datelor organizatiei '%1' nu s-a efectuat !!!")
                              .arg(ui->editName->text()));
        msg->setDetailedText(err.join("\n"));
        msg->exec();
        msg->deleteLater();

        return false;
    }
}

bool CatOrganizations::updateDataTableOrganizations()
{
    QSqlQuery qry;
    qry.prepare(R"(
        UPDATE organizations SET
            deletionMark = ?,
            IDNP         = ?,
            TVA          = ?,
            name         = ?,
            address      = ?,
            telephone    = ?,
            email        = ?,
            comment      = ?,
            id_contracts = ?,
            stamp        = ?
        WHERE
            id = ?
    )");
    qry.addBindValue(0);
    qry.addBindValue(ui->editIDNP->text());
    qry.addBindValue(ui->editTVA->text().isEmpty()
                         ? QVariant()
                         : ui->editTVA->text());
    qry.addBindValue(ui->editName->text());
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
    qry.addBindValue((m_IdMainContract <= 0)
                         ? QVariant()
                         : m_IdMainContract);

    QByteArray image_data;
    QPixmap pix = ui->img_stamp->pixmap();
    if (! pix.isNull()) {
        QBuffer buffer(&image_data);
        buffer.open(QIODevice::WriteOnly);
        pix.save(&buffer, "PNG");
    }
    qry.addBindValue(image_data.isEmpty()
                         ? QVariant(QMetaType(QMetaType::QByteArray))
                         : image_data.toBase64());

    qry.addBindValue(m_Id);

    if (qry.exec()) {
        qInfo(logInfo()) << tr("Datele organizatiei '%1' cu ID='%2' au fost modificate cu succes.")
                                .arg(ui->editName->text(), QString::number(m_Id));
        return true;
    } else {
        // formam eroarea
        err.clear();
        err << this->metaObject()->className()
            << "[updateDataTableOrganizations]"
            << QStringLiteral("Eroare la modificarea datelor organizatiei %1 %2")
                   .arg(ui->editName->text(),
                        qry.lastError().text().isEmpty()
                            ? ": eroarea indisponibila"
                            : ": " + qry.lastError().text());

        // logarea
        qCritical(logCritical()) << err;

        // prezentarea mesajului cu eroare
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Modificarea datelor organizatiei '%1' nu s-a efectuat !!!")
                              .arg(ui->editName->text()));
        msg->setDetailedText(err.join("\n"));
        msg->exec();
        msg->deleteLater();
        return false;
    }
}

// *******************************************************************
// **************** EVENIMENTELE FORMEI ******************************

void CatOrganizations::closeEvent(QCloseEvent *event)
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
            onWritingData();
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

void CatOrganizations::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        // traducem  titlu
        if (m_itNew){
            setWindowTitle(tr("Organizația (crearea) %1").arg("[*]"));
        } else {
            setWindowTitle(tr("Organizația (salvată) %1").arg("[*]"));
        }
        updateHeaderTableContracts();
    }
}

void CatOrganizations::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Return || event->key() == Qt::Key_Enter){
      this->focusNextChild();
    }
}
