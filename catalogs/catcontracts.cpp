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

#include "catcontracts.h"
#include "ui_catcontracts.h"

#include <customs/custommessage.h>

CatContracts::CatContracts(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CatContracts)
{
    ui->setupUi(this);

    db = new DataBase(this);

    ui->editName->setMaxLength(50);

    ui->dateInit->setDate(QDate::currentDate()); // setam data

    QString strQueryOrganization = "SELECT id,name FROM organizations WHERE deletionMark = 0;";
    model = new BaseSqlQueryModel(strQueryOrganization, ui->comboOrganizations);
    model->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    ui->comboOrganizations->setModel(model);

    QString strQueryTypesPrices = "SELECT id, name FROM typesPrices WHERE deletionMark = 0;";
    modelTypesPrices = new BaseSqlQueryModel(strQueryTypesPrices, ui->comboTypesPrices);
    modelTypesPrices->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    ui->comboTypesPrices->setModel(modelTypesPrices);

    initConnections();
}

CatContracts::~CatContracts()
{
    delete model;
    delete modelTypesPrices;
    delete db;
    delete ui;
}

QString CatContracts::getNameParentContract()
{
    return ui->comboOrganizations->currentText();
}

void CatContracts::initConnections()
{
    ui->btnOK->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));  // comenzi rapide la tastatura
    ui->btnWrite->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    ui->btnClose->setShortcut(QKeySequence(Qt::Key_Escape));

    connect(ui->comboOrganizations, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CatContracts::changedIndexComboOrganization);
    connect(ui->comboTypesPrices, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CatContracts::currentIndexTypesPricesChanged);

    connect(ui->editName, &QLineEdit::textChanged, this, &CatContracts::dataWasModified);
    connect(ui->dateInit, &QDateEdit::dateChanged, this, &CatContracts::dataWasModified);
    connect(ui->checkBoxNotValid, &QCheckBox::checkStateChanged, this, &CatContracts::dataWasModified);
    connect(ui->editComment, &QPlainTextEdit::textChanged, this, &CatContracts::dataWasModified);

    connect(ui->btnOK, &QAbstractButton::clicked, this, &CatContracts::onWritingDataClose);
    connect(ui->btnWrite, &QAbstractButton::clicked, this, &CatContracts::onWritingData);
    connect(ui->btnClose, &QAbstractButton::clicked, this, &CatContracts::onClose);

    connect(this, &CatContracts::IdChanged, this, &CatContracts::slot_IdChanged);
    connect(this, &CatContracts::ItNewChanged, this, &CatContracts::slot_ItNewChanged);
    connect(this, &CatContracts::IdChangedOrganization, this, &CatContracts::slot_IdChangedOrganization);
    connect(this, &CatContracts::IdTypesPricesChanged, this, &CatContracts::slot_IdTypesPricesChanged);
}

void CatContracts::connectionModified()
{
    connect(ui->comboOrganizations, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CatContracts::changedIndexComboOrganization);
    connect(ui->comboTypesPrices, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CatContracts::currentIndexTypesPricesChanged);

    connect(ui->comboOrganizations, &QComboBox::currentIndexChanged, this, &CatContracts::dataWasModified);
    connect(ui->comboTypesPrices,&QComboBox::currentIndexChanged, this, &CatContracts::dataWasModified);

    connect(ui->editName, &QLineEdit::textChanged, this, &CatContracts::dataWasModified);
    connect(ui->dateInit, &QDateEdit::dateChanged, this, &CatContracts::dataWasModified);
    connect(ui->checkBoxNotValid, &QCheckBox::checkStateChanged, this, &CatContracts::dataWasModified);
    connect(ui->editComment, &QPlainTextEdit::textChanged, this, &CatContracts::dataWasModified);
}

void CatContracts::disconnectionModified()
{
    disconnect(ui->comboOrganizations, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CatContracts::changedIndexComboOrganization);
    disconnect(ui->comboTypesPrices, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CatContracts::currentIndexTypesPricesChanged);

    disconnect(ui->comboOrganizations, &QComboBox::currentIndexChanged, this, &CatContracts::dataWasModified);
    disconnect(ui->comboTypesPrices,&QComboBox::currentIndexChanged, this, &CatContracts::dataWasModified);

    disconnect(ui->editName, &QLineEdit::textChanged, this, &CatContracts::dataWasModified);
    disconnect(ui->dateInit, &QDateEdit::dateChanged, this, &CatContracts::dataWasModified);
    disconnect(ui->checkBoxNotValid, &QCheckBox::checkStateChanged, this, &CatContracts::dataWasModified);
    disconnect(ui->editComment, &QPlainTextEdit::textChanged, this, &CatContracts::dataWasModified);
}

bool CatContracts::controlRequiredObjects()
{
    if (ui->comboOrganizations->currentIndex() == -1 || ui->comboOrganizations->currentIndex() == 0){
        QMessageBox::warning(this,
                             tr("Verificarea datelor."),
                             tr("Nu este indicat\304\203 \"<b>Organiza\310\233ia</b>\" !!!"),
                             QMessageBox::Ok);
        return false;
    }

    if (ui->comboTypesPrices->currentIndex() == -1 || ui->comboTypesPrices->currentIndex() == 0){
        QMessageBox::warning(this,
                             tr("Verificarea datelor."),
                             tr("Nu este indicat \"<b>Tipul pre\310\233ului</b>\" !!!"),
                             QMessageBox::Ok);
        return false;
    }

    if (ui->editName->text().isEmpty()){
        QMessageBox::warning(this,
                             tr("Verificarea datelor."),
                             tr("Nu este indicat\304\203 \"<b>Denumirea</b>\" contractului !!!"),
                             QMessageBox::Ok);
        return false;
    }

    return true;
}

QVariantMap CatContracts::getDataObject()
{
    QVariantMap map;

    map["id"]               = m_Id;
    map["deletionMark"]     = 0;
    map["id_organizations"] = m_IdOrganization;
    map["id_typesPrices"]   = m_id_types_prices;
    map["name"]             = ui->editName->text();
    map["dateInit"]         = ui->dateInit->date().toString("yyyy-MM-dd");
    map["notValid"] = globals().thisMySQL
                          ? QVariant(ui->checkBoxNotValid->isChecked())
                          : QVariant(int(ui->checkBoxNotValid->isChecked()));
    map["comment"] = ui->editComment->toPlainText().isEmpty()
                         ? QVariant()
                         : ui->editComment->toPlainText();

    return map;
}

bool CatContracts::insertIntoTableContracts(QStringList &err)
{
    // verificam daca este corect determinat ID
    if (m_Id == -1 || m_Id == 0) {
        err << this->metaObject()->className()
            << "[insertIntoTableContracts]:"
            << "Nu este indicat ID contractului !!!";
        qWarning(logWarning()) << err;
        return false;
    }

    // pregatim datele
    QVariantMap map = getDataObject();

    // inseram datele in bd
    return db->insertIntoTable(this->metaObject()->className(), "contracts", map, err);
}

bool CatContracts::updateDataTableContracts(QStringList &err)
{
    // verificam daca este corect determinat ID
    if (m_Id == -1 || m_Id == 0) {
        err << this->metaObject()->className()
            << "[insertIntoTableContracts]:"
            << "Nu este indicat ID contractului !!!";
        qWarning(logWarning()) << err;
        return false;
    }

    // pregatim datele
    QVariantMap map = getDataObject();

    // pregatim conditia
    QMap<QString, QVariant> where;
    where["id"] = m_Id;
    if (m_IdOrganization > 0)
        where["id_organizations"] = m_IdOrganization;

    // actualizam datele
    return db->updateTable(this->metaObject()->className(), "contracts", map, where, err);
}

void CatContracts::dataWasModified()
{
    setWindowModified(true);
}

void CatContracts::slot_IdChanged()
{
    if (m_itNew)
        return;

    disconnectionModified();

    QSqlQuery qry;
    qry.prepare("SELECT * FROM contracts WHERE id = ?");
    qry.addBindValue(m_Id);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        // combo 'Organizatia'
        int _id_organization = qry.value(rec.indexOf("id_organizations")).toInt();
        setIdOrganization(_id_organization);

        // combo 'Tipul preturilor'
        int _id_type_price = qry.value(rec.indexOf("id_typesPrices")).toInt();
        setIdTypesPrices(_id_type_price);

        ui->editName->setText(qry.value(rec.indexOf("name")).toString());
        ui->dateInit->setDate(QDate::fromString(qry.value(rec.indexOf("dateInit")).toString(), "yyyy-MM-dd"));
        ui->checkBoxNotValid->setChecked(qry.value(rec.indexOf("notValid")).toBool());
        ui->editComment->setPlainText(qry.value(rec.indexOf("comment")).toString());
    }

    connectionModified();
}

void CatContracts::slot_ItNewChanged()
{
    if (m_itNew){
        setWindowTitle(tr("Contract (crearea) %1").arg("[*]"));
        ui->editName->setText(tr("Contract comercial"));
    } else {
        setWindowTitle(tr("Contract (salvat) %1").arg("[*]"));
    }
}

void CatContracts::slot_IdChangedOrganization()
{
    if (m_IdOrganization == -1)
        return;

    auto indexOrganization = model->match(model->index(0,0), Qt::UserRole, m_IdOrganization, 1, Qt::MatchExactly);
    if (!indexOrganization.isEmpty())
        ui->comboOrganizations->setCurrentIndex(indexOrganization.first().row());
}

void CatContracts::slot_IdTypesPricesChanged()
{
    if (m_id_types_prices == - 1)
        return;

    auto idx_type_price = modelTypesPrices->match(modelTypesPrices->index(0,0), Qt::UserRole, m_id_types_prices, 1, Qt::MatchExactly);
    if (! idx_type_price.isEmpty())
        ui->comboTypesPrices->setCurrentIndex(idx_type_price.first().row());
}

void CatContracts::changedIndexComboOrganization(const int index)
{
    int id_organization = ui->comboOrganizations->itemData(index, Qt::UserRole).toInt();
    setIdOrganization(id_organization);

    emit IdChangedOrganization();
}

void CatContracts::currentIndexTypesPricesChanged(const int index)
{
    int id_types_prices = ui->comboTypesPrices->itemData(index, Qt::UserRole).toInt();
    setIdTypesPrices(id_types_prices);

    emit IdTypesPricesChanged();
}

bool CatContracts::onWritingData()
{
    // verificam completarea campurilor obligatorii
    if (! controlRequiredObjects())
        return false;

    QStringList err; // anuntam variabila pu erori

    /* cream o functie lambda locala in interiorul pu oprimizarea codului
     * si eliminarea dublajului */
    auto showError = [&](const QString &title) {
        CustomMessage *message = new CustomMessage(this);
        message->setTextTitle(title.arg(ui->editName->text()));
        message->setDetailedText(err.join("\n"));
        message->exec();
        message->deleteLater();
    };

    if (m_itNew){

        setId(db->getLastIdForTable("contracts") + 1);

        if (! insertIntoTableContracts(err)){
            showError(tr("Salvarea datelor contractului \"<b>%1</b>\" nu s-a efectuat."));
            return false;
        }

        setItNew(false);          // setam ca obiectul nu este nou
        emit createNewContract(); // emitem signalul pu actualizarea tabela in Organizatia

    } else {

        if (! updateDataTableContracts(err)){
            showError(tr("Modificarea datelor contractului \"<b>%1</b>\" nu s-a efectuat."));
            return false;
        }

        emit changedCatContract(); // emitem signalul pu actualizarea tabela in Organizatia
    }

    setWindowModified(false);

    return true;
}

void CatContracts::onWritingDataClose()
{
    if (onWritingData())
        QDialog::accept();
}

void CatContracts::onClose()
{
    this->close();
}

void CatContracts::closeEvent(QCloseEvent *event)
{
    if (isWindowModified()){

        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Modificarea datelor"),
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

        if (messange_box.clickedButton() == yesButton){
            onWritingDataClose();
            event->accept();
        } else if (messange_box.clickedButton() == noButton){
            event->accept();
        } else if (messange_box.clickedButton() == cancelButton){
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void CatContracts::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        // traducem  titlu
        setWindowTitle(tr("Contract %1").arg("[*]"));
    }
}

void CatContracts::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Return || event->key() == Qt::Key_Enter){
      this->focusNextChild();
    }
}
