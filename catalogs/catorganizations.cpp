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

#include <QImageReader>
#include <QImageWriter>

static const int max_length_comment = 255; // lungimea maxima a cometariului

CatOrganizations::CatOrganizations(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CatOrganizations)
{
    ui->setupUi(this);

    setWindowTitle(tr("Organizația %1").arg("[*]"));

    popUp = new PopUp(this);     // alocam memoria p-u mesaje
    db    = new DataBase(this);  // alocam memoria pu conectarea cu BD

    ui->editName->setMaxLength(100);
    ui->editIDNP->setMaxLength(15);
    ui->editTVA->setMaxLength(10);
    ui->editAddress->setMaxLength(255);
    ui->editTelephone->setMaxLength(100);
    ui->editEmail->setMaxLength(100);

    initBtnToolBar();        // initializam butoanele tool barului tabelei contracte

    QString strQuery = "";
    modelCantract = new BaseSqlQueryModel(strQuery, this); // initierea modelului pu contracte
    updateTableContracts();                                // completarea tabelei cu contracte

    connect(ui->btnClearStamp, &QToolButton::clicked, this, &CatOrganizations::clearImageStamp);
    ui->img_stamp->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
    ui->img_stamp->setTextFormat(Qt::RichText);
    ui->img_stamp->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    connect(ui->img_stamp, QOverload<const QString &>::of(&QLabel::linkActivated), this,
            QOverload<const QString &>::of(&CatOrganizations::onLinkActivatedForOpenImage));

    connect(this, &CatOrganizations::IdChanged, this, &CatOrganizations::slot_IdChanged);
    connect(this, &CatOrganizations::ItNewChanged, this, &CatOrganizations::slot_ItNewChanged);

    connect(ui->editName, &QLineEdit::textChanged, this, &CatOrganizations::textChangedNameOrganization);

    QList<QLineEdit*> list = this->findChildren<QLineEdit*>();
    for (int n = 0; n < list.count(); n++) {
        connect(list[n], &QLineEdit::textChanged, this, &CatOrganizations::dataWasModified);
    }

    connect(ui->editComment, &QTextEdit::textChanged, this, &CatOrganizations::controlLengthComment);
    connect(ui->editComment, &QTextEdit::textChanged, this, &CatOrganizations::dataWasModified);

    ui->btnOK->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));  // comenzi rapide la tastatura
    ui->btnWrite->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    ui->btnCancel->setShortcut(QKeySequence(Qt::Key_Escape));

    connect(ui->btnOK, &QAbstractButton::clicked, this, &CatOrganizations::onWritingDataClose);
    connect(ui->btnWrite, &QAbstractButton::clicked, this, &CatOrganizations::onWritingData);
    connect(ui->btnCancel, &QAbstractButton::clicked, this, &CatOrganizations::close);

    connect(ui->tableViewContracts, QOverload<const QModelIndex&>::of(&QTableView::doubleClicked), this, &CatOrganizations::onDoubleClickedTableContracts);
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

    QList<QLineEdit*> list = this->findChildren<QLineEdit*>();
    for (int n = 0; n < list.count(); n++) {
        disconnect(list[n], &QLineEdit::textChanged, this, &CatOrganizations::dataWasModified);
    }
    disconnect(ui->editComment, &QTextEdit::textChanged, this, &CatOrganizations::dataWasModified);

    QMap<QString, QString> _items;
    if (db->getObjectDataById("organizations", m_Id, _items)){
        ui->editName->setText(_items.constFind("name").value());
        ui->editIDNP->setText(_items.constFind("IDNP").value());
        ui->editTVA->setText(_items.constFind("TVA").value());
        ui->editAddress->setText(_items.constFind("address").value());
        ui->editTelephone->setText(_items.constFind("telephone").value());
        ui->editEmail->setText(_items.constFind("email").value());
        ui->editComment->setText(_items.constFind("comment").value());

        int id_contract = _items.constFind("id_contracts").value().toInt();
        if (id_contract != 0)                // contractul cu id=0 nu este
            setIdMainContract(id_contract);  // setam proprietatea m_IdMainContract
    }

    loadImageOpeningCatalog();
    updateTableContracts();
    for (int n = 0; n < list.count(); n++) {
        connect(list[n], &QLineEdit::textChanged, this, &CatOrganizations::dataWasModified);
    }
    connect(ui->editComment, &QTextEdit::textChanged, this, &CatOrganizations::dataWasModified);
}

void CatOrganizations::textChangedNameOrganization()
{
    setNameOrganization(ui->editName->text());
}

// *******************************************************************
// **************** INSERAREA IMAGINEI SI ALTELE *********************

void CatOrganizations::loadImageOpeningCatalog()
{
    QPixmap outPixmap;
    if (! globals().main_stamp_organization.isEmpty() && outPixmap.loadFromData(globals().main_stamp_organization))
        ui->img_stamp->setPixmap(outPixmap.scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void CatOrganizations::clearImageStamp()
{
    QSqlQuery qry;
    qry.prepare("UPDATE organizations SET stamp = :value WHERE id = :id;");
    qry.bindValue(":id",    m_Id);
    qry.bindValue(":value", QVariant());
    if (qry.exec()){
        ui->img_stamp->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
        ui->img_stamp->setTextFormat(Qt::RichText);
        ui->img_stamp->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        popUp->setPopupText(tr("Imaginea este eliminată din baza de date."));
        popUp->show();
        globals().main_stamp_organization = nullptr;
    } else {
        qCritical(logCritical()) << tr("Eroare la eliminarea imaginei din baza de date:\n") << qry.lastError();
    }
}

bool CatOrganizations::loadFile(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Nu este setată imaginea %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }
    ui->img_stamp->setPixmap(QPixmap::fromImage(newImage).scaled(200,200));

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return true;
    QByteArray inByteArray = file.readAll();

    QSqlQuery qry;
    qry.prepare("UPDATE organizations SET stamp = :value WHERE id = :id;");
    qry.bindValue(":id",    m_Id);
    qry.bindValue(":value", inByteArray.toBase64());
    if (qry.exec()){
        popUp->setPopupText(tr("Imaginea este salvat cu succes în baza de date."));
        popUp->show();
        globals().main_stamp_organization = QByteArray::fromBase64(inByteArray.toBase64());
    } else {
        qCritical(logCritical()) << tr("Eroare de inserare a imaginei in baza de date:\n") << qry.lastError();
    }
    file.close();

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
            m_write_new_organziation = 1;
            if (! onWritingData())
                return;
        } else if (messange_box.clickedButton() == noButton) {
            return;
        }
    }
    if (m_Id == -1){
        qWarning(logWarning()) << this->metaObject()->className()
                               << tr(": crearea contractului pentru organizatia '%1' - nu este determinat 'id' organizatiei.")
                                  .arg(ui->editName->text());
    }
    catContracts = new CatContracts(this);
    catContracts->setAttribute(Qt::WA_DeleteOnClose);
    catContracts->setProperty("itNew", true);
    catContracts->setProperty("IdOrganization", getId());
    catContracts->show();
    connect(catContracts, &CatContracts::createNewContract, this, &CatOrganizations::slot_updateTableContracts);
}

void CatOrganizations::editContract()
{
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
    if (ui->tableViewContracts->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Atenție"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }
    int _id = modelCantract->data(modelCantract->index(ui->tableViewContracts->currentIndex().row(), sectionsContract_id), Qt::DisplayRole).toInt();
    catContracts = new CatContracts(this);
    catContracts->setAttribute(Qt::WA_DeleteOnClose);
    catContracts->setProperty("itNew", false);
    catContracts->setProperty("Id", _id);
    connect(catContracts, &CatContracts::changedCatContract, this, [this]()
    {
        slot_updateTableContracts();
        popUp->setPopupText(tr("Datele contractului obiectului <b>'%1'</b><br>"
                               "au fost modificate cu succes.").arg(catContracts->getNameParentContract()));
        popUp->show();
    });
    catContracts->show();
}

void CatOrganizations::markDeletionContract()
{
    int currentRow = ui->tableViewContracts->currentIndex().row();
    if (currentRow == -1){
        QMessageBox::warning(this, tr("Atenție"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }
    int _id               = modelCantract->data(modelCantract->index(currentRow, sectionsQry_id), Qt::DisplayRole).toInt();
    int _delMark          = modelCantract->data(modelCantract->index(currentRow, sectionsQry_deletionMark), Qt::DisplayRole).toInt();
    QString _nameContract = modelCantract->data(modelCantract->index(currentRow, sectionsQry_name), Qt::DisplayRole).toString();

    QString strMSG;
    if (_delMark == 0)
        strMSG = tr("Obiectul \"%1\" a fost marcat \n"
                    "pentru eliminarea din baza de date.").arg(_nameContract);
    else if (_delMark == 1)
        strMSG = tr("Obiectul \"%1\" a fost demarcat \n"
                    "pentru eliminarea din baza de date.").arg(_nameContract);

    if (db->deletionMarkObject("contracts", _id)){
        qInfo(logInfo()) << tr("Obiectul '%1' cu id='%2' este marcat pentru eliminarea din baza de date.").arg(ui->editName->text(), QString::number(m_Id));
        popUp->setPopupText(strMSG);
        popUp->show();
    } else {
        QMessageBox::warning(this, tr("Marcarea/demarcarea obiectului"),
                             tr("Marcarea/demarcarea obiectului \"<b>%1</b>\" nu este reusita.").arg(_nameContract), QMessageBox::Ok);
        qWarning(logWarning()) << this->metaObject()->className()
                               << tr(": eroare la marcare eliminarii din baza de date a obiectului '%1' cu id='%2'.").arg(ui->editName->text(), QString::number(m_Id));
    }
    updateTableContracts();
}

void CatOrganizations::setDefaultContract()
{
    int currentRow = ui->tableViewContracts->currentIndex().row();
    if (currentRow == -1){
        QMessageBox::warning(this, tr("Atenție"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }
    int _id               = modelCantract->data(modelCantract->index(currentRow, sectionsQry_id), Qt::DisplayRole).toInt();
    QString _nameContract = modelCantract->data(modelCantract->index(currentRow, sectionsQry_name), Qt::DisplayRole).toString();

    if (_id == 0) // contract cu ID = 0 nu poate fi
        return;

    QMap<QString, QString> items = getDataObject();             // datele formei

    if (db->updateDataObject("organizations", m_Id, items)){   // actualizam datele obiectului cu ID contractului
        qInfo(logInfo()) << tr("Organizatiei '%1' cu id='%2' a fost atasat contract de baza.").arg(ui->editName->text(), QString::number(m_Id));
    } else {
        QMessageBox::warning(this, tr("Modificarea datelor."),
                             tr("Modificarea datelor obiectului \"<b>%1</b>\" nu s-a efectuat.<br>"
                                "Adresați-vă administratorului aplicației.").arg(ui->editName->text()),
                             QMessageBox::Ok);
        qWarning(logWarning()) << this->metaObject()->className()
                               << tr(": eroare la atasarea contractului de baza a organizatiei '%1' cu id='%2'.").arg(ui->editName->text(), QString::number(m_Id));
        return;
    }

    setIdMainContract(_id); // setam proprietatea
    updateTableContracts();

    popUp->setPopupText(tr("Contractul <b>'%1'</b><br> a fost setat de bază.").arg(_nameContract));
    popUp->show();
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
    if (m_write_new_organziation == 0)
        m_write_new_organziation = 1;

    if (m_itNew){

        if (existObjectInTableOrganizations()){
            QMessageBox messange_box(QMessageBox::Question,
                                     tr("Verificarea datelor"),
                                     tr("Organiza\310\233ia cu nume \"<b>%1</b>\" exist\304\203 \303\256n baza de date.<br>"
                                        "Dori\310\233i s\304\203 continua\310\233i validarea datelor ?").arg(ui->editName->text()),
                                     QMessageBox::NoButton, this);
            QPushButton *yesButton    = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
            QPushButton *noButton     = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
            yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
            noButton->setStyleSheet(db->getStyleForButtonMessageBox());
            messange_box.exec();

            if (messange_box.clickedButton() == yesButton) {
            } else if (messange_box.clickedButton() == noButton) {
               return false;
            }
        }

        //setam id
        if (m_Id == -1)
            setId(db->getLastIdForTable("organizations") + 1);

        // crearea organizatiei
        if (! insertIntoTableOrganizations()){
            QMessageBox::warning(this, tr("Crearea obiectului."),
                                 tr("Crearea obiectului cu nume \"<b>%1</b>\" nu s-a reușit.<br>"
                                    "Adresați-vă administratorului aplicației.").arg(ui->editName->text()),
                                 QMessageBox::Ok);
            setId(-1);
            return false;
        }

        setItNew(false);
        setWindowModified(false);

        if (m_write_new_organziation == 1){
            popUp->setPopupText(tr("Organizația <b>%1</b><br> a fost creat cu succes.").arg(ui->editName->text()));
            popUp->show();
        }

        emit mWriteNewOrganization();

    } else {

        // verificam daca a fost transmis proprietatea ID utilizatorului
        if (m_Id == - 1){
            QMessageBox::warning(this, tr("Modificarea datelor."),
                                 tr("Modificarea datelor obiectului \"<b>%1</b>\" nu s-a efectuat.<br>"
                                    "Adresați-vă administratorului aplicației.").arg(ui->editName->text()),
                                 QMessageBox::Ok);
            qWarning(logWarning()) << this->metaObject()->className()
                                   << tr(": eroare la modificarea datelor organizatiei '%1': nu este determinat ID.").arg(ui->editName->text());
            return false;
        }
        // modificam datele utilizatorului
        if (! updateDataTableOrganizations()){
            QMessageBox::warning(this, tr("Modificarea datelor."),
                                 tr("Modificarea datelor obiectului \"<b>%1</b>\" nu s-a efectuat.<br>"
                                    "Adresați-vă administratorului aplicației.").arg(ui->editName->text()),
                                 QMessageBox::Ok);
            return false;
        }
        if (m_write_new_organziation == 1){
            popUp->setPopupText(tr("Datele organizației <b>%1</b><br> au fost modificate cu succes.").arg(ui->editName->text()));
            popUp->show();
        }
        emit mChangedDateOrganization();
    }
    return true;
}

void CatOrganizations::onWritingDataClose()
{
    m_write_new_organziation = 2; // validare
    if (onWritingData())
        QDialog::accept();
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

QMap<QString, QString> CatOrganizations::getDataObject()
{
    QMap<QString, QString> _items;
    _items.insert("name",      ui->editName->text());
    _items.insert("IDNP",      ui->editIDNP->text());
    _items.insert("TVA",       ui->editTVA->text());
    _items.insert("address",   ui->editAddress->text());
    _items.insert("telephone", ui->editTelephone->text());
    _items.insert("email",     ui->editEmail->text());
    _items.insert("comment",   ui->editComment->toPlainText());
    if (m_IdMainContract != -1)
        _items.insert("id_contracts", QString::number(m_IdMainContract));
    else
        _items.insert("id_contracts", "");
    return _items;
}

void CatOrganizations::updateTableContracts()
{
    QString strQuery = "";
    if (m_Id != -1)
        strQuery = QString("SELECT id,deletionMark,dateInit,name FROM contracts WHERE id_organizations = %1;").arg(QString::number(m_Id));

    modelCantract->setQuery(strQuery);
    if (m_IdMainContract != -1)
        modelCantract->setIdContract(m_IdMainContract);  // transmitem id contractului de baza

    ui->tableViewContracts->setModel(modelCantract);     // setam model in tabela
    ui->tableViewContracts->hideColumn(sectionsQry_id);                                        // id-ascundem
    ui->tableViewContracts->horizontalHeader()->setStretchLastSection(true);      // extinderea ultimei sectiei
    ui->tableViewContracts->setSelectionBehavior(QAbstractItemView::SelectRows);  // setam alegerea randului
    ui->tableViewContracts->setSelectionMode(QAbstractItemView::SingleSelection); // setam singura alegerea(nu multipla)
    ui->tableViewContracts->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // permitem schimbarea size sectiilor
    ui->tableViewContracts->verticalHeader()->setDefaultSectionSize(12); // modificam inaltimea randului
    ui->tableViewContracts->setColumnWidth(sectionsQry_deletionMark, 5);                        // modificam latimea primei sectiei cu imaginea
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

bool CatOrganizations::existObjectInTableOrganizations()
{
    QSqlQuery qry;
    qry.prepare("SELECT count(name) FROM organizations WHERE name = :name AND deletionMark = 0;");
    qry.bindValue(":name", ui->editName->text());
    if (qry.exec() && qry.next()){
        int _bool = qry.value(0).toInt();
        return (_bool > 0) ? true : false;
    } else {
        return false;
    }
}

bool CatOrganizations::insertIntoTableOrganizations()
{
    QSqlQuery qry;
    qry.prepare("INSERT INTO organizations ("
                "id,"
                "deletionMark,"
                "IDNP,"
                "TVA,"
                "name,"
                "address,"
                "telephone,"
                "email,"
                "comment,"
                "id_contracts ) VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
    qry.addBindValue(m_Id);
    qry.addBindValue(0);
    qry.addBindValue(ui->editIDNP->text());
    qry.addBindValue(ui->editTVA->text());
    qry.addBindValue(ui->editName->text());
    qry.addBindValue(ui->editAddress->text());
    qry.addBindValue(ui->editTelephone->text());
    qry.addBindValue(ui->editEmail->text());
    qry.addBindValue(ui->editComment->toPlainText());
    qry.addBindValue((m_IdMainContract == -1) ? QVariant() : m_IdMainContract);
    if (qry.exec()) {
        qInfo(logInfo()) << tr("A fost creata organizatia noua '%1' cu id='%2'.").arg(ui->editName->text(), QString::number(m_Id));
        return true;
    } else {
        qWarning(logWarning()) << this->metaObject()->className()
                               << tr(": eroare la crearea organizatiei '%1' cu id='%2'.").arg(ui->editName->text(), QString::number(m_Id));
        return false;
    }
}

bool CatOrganizations::updateDataTableOrganizations()
{
    QSqlQuery qry;
    qry.prepare("UPDATE organizations SET "
                "deletionMark = :deletionMark,"
                "IDNP         = :IDNP,"
                "TVA          = :TVA,"
                "name         = :name,"
                "address      = :address,"
                "telephone    = :telephone,"
                "email        = :email,"
                "comment      = :comment,"
                "id_contracts = :id_contracts WHERE id = :id;");
    qry.bindValue(":id",           m_Id);
    qry.bindValue(":deletionMark", 0);
    qry.bindValue(":IDNP",         ui->editIDNP->text());
    qry.bindValue(":TVA",          ui->editTVA->text());
    qry.bindValue(":name",         ui->editName->text());
    qry.bindValue(":address",      ui->editAddress->text());
    qry.bindValue(":telephone",    ui->editTelephone->text());
    qry.bindValue(":email",        ui->editEmail->text());
    qry.bindValue(":comment",      ui->editComment->toPlainText());
    qry.bindValue(":id_contracts", (m_IdMainContract == -1) ? QVariant() : m_IdMainContract);
    if (qry.exec()) {
        qInfo(logInfo()) << tr("Datele organizatiei '%1' cu id='%2' au fost modificate cu succes.").arg(ui->editName->text(), QString::number(m_Id));
        return true;
    } else {
        qWarning(logWarning()) << this->metaObject()->className()
                               << tr(": eroarea la modificarea datelor organizatiei '%1' cu id='%2' - %1.").arg(ui->editName->text(), QString::number(m_Id), qry.lastError().text());
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
