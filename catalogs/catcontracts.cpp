﻿#include "catcontracts.h"
#include "ui_catcontracts.h"

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
    connect(ui->checkBoxNotValid, &QCheckBox::stateChanged, this, &CatContracts::dataWasModified);
    connect(ui->editComment, &QPlainTextEdit::textChanged, this, &CatContracts::dataWasModified);

    connect(ui->btnOK, &QAbstractButton::clicked, this, &CatContracts::onWritingDataClose);
    connect(ui->btnWrite, &QAbstractButton::clicked, this, &CatContracts::onWritingData);
    connect(ui->btnClose, &QAbstractButton::clicked, this, &CatContracts::onClose);

    connect(this, &CatContracts::IdChanged, this, &CatContracts::slot_IdChanged);
    connect(this, &CatContracts::ItNewChanged, this, &CatContracts::slot_ItNewChanged);
    connect(this, &CatContracts::IdChangedOrganization, this, &CatContracts::slot_IdChangedOrganization);
}

bool CatContracts::controlRequiredObjects()
{
    if (ui->comboOrganizations->currentIndex() == -1){
        QMessageBox::warning(this, tr("Verificarea datelor."),
                             tr("Nu este indicata \"<b>Organizatia</b>\" !!!"), QMessageBox::Ok);
        return false;
    }

    if (ui->editName->text().isEmpty()){
        QMessageBox::warning(this, tr("Verificarea datelor."),
                             tr("Nu este indicata \"<b>Denumirea</b>\" contractului !!!"), QMessageBox::Ok);
        return false;
    }

    return true;
}

QMap<QString, QString> CatContracts::getDataObject()
{
    int id_typesPrices = ui->comboTypesPrices->itemData(ui->comboTypesPrices->currentIndex(), Qt::UserRole).toInt();

    QMap<QString, QString> _items;
    _items.insert("id_organizations", QString::number(m_IdOrganization));
    _items.insert("id_typesPrices",   QString::number(id_typesPrices));
    _items.insert("name",             ui->editName->text());
    _items.insert("dateInit",         ui->dateInit->date().toString("yyyy-MM-dd"));
    _items.insert("checkBoxNotValid", QString::number(ui->checkBoxNotValid->checkState()));
    _items.insert("comment",          ui->editComment->toPlainText());
    return _items;
}

bool CatContracts::insertIntoTableContracts()
{
    QSqlQuery qry;
    qry.prepare("INSERT INTO contracts ("
                "id,"
                "deletionMark,"
                "id_organizations,"
                "id_typesPrices,"
                "name,"
                "dateInit,"
                "notValid,"
                "comment"
                ") VALUES (?,?,?,?,?,?,?,?);");
    qry.addBindValue(m_Id);
    qry.addBindValue(0);
    qry.addBindValue(m_IdOrganization);
    qry.addBindValue(m_id_types_prices);
    qry.addBindValue(ui->editName->text());
    qry.addBindValue(ui->dateInit->date().toString("yyyy-MM-dd"));
    if (globals::thisMySQL)
        qry.addBindValue((ui->checkBoxNotValid->isChecked()) ? true : false);
    else
        qry.addBindValue((ui->checkBoxNotValid->isChecked()) ? 1 : 0);
    qry.addBindValue(ui->editComment->toPlainText());
    if (! qry.exec()){
        qCritical(logCritical()) << tr("Eroarea la inserarea datelor contractului '%1' in tabela 'contracts' - %2").arg(ui->editName->text(), qry.lastError().text());
        return false;
    }
    qInfo(logInfo()) << tr("Crearea contractului cu nume '%1', id='%2', organizatia - '%3'.")
                            .arg(ui->editName->text(), QString::number(m_Id), ui->comboOrganizations->currentText());
    return true;
}

bool CatContracts::updateDataTableContracts()
{
    QSqlQuery qry;
    qry.prepare("UPDATE contracts SET "
                "deletionMark     = :deletionMark,"
                "id_organizations = :id_organizations,"
                "id_typesPrices   = :id_typesPrices,"
                "name             = :name,"
                "dateInit         = :dateInit,"
                "notValid         = :notValid,"
                "comment          = :comment "
                "WHERE id = :id;");
    qry.bindValue(":id",               m_Id);
    qry.bindValue(":deletionMark",     0);
    qry.bindValue(":id_organizations", m_IdOrganization);
    qry.bindValue(":id_typesPrices",   m_id_types_prices);
    qry.bindValue(":name",             ui->editName->text());
    qry.bindValue(":dateInit",         ui->dateInit->date().toString("yyyy-MM-dd"));
    if (globals::thisMySQL)
        qry.bindValue(":notValid", (ui->checkBoxNotValid->isChecked()) ? true : false);
    else
        qry.bindValue(":notValid", (ui->checkBoxNotValid->isChecked()) ? 1 : 0);
    qry.bindValue(":comment", ui->editComment->toPlainText());
    if (! qry.exec()){
        qCritical(logCritical()) << tr("Eroarea la modificarea datelor contractului '%1' din tabela 'contracts' - %2").arg(ui->editName->text(), qry.lastError().text());
        return false;
    }
    qInfo(logInfo()) << tr("Modificarea datelor contractului cu nume '%1', id='%2', organizatia - '%3'.")
                            .arg(ui->editName->text(), QString::number(m_Id), ui->comboOrganizations->currentText());
    return true;
}

void CatContracts::dataWasModified()
{
    setWindowModified(true);
}

void CatContracts::slot_IdChanged()
{
    if (m_itNew)
        return;

    QMap<QString, QString> _items;
    _items.clear();
    if (db->getObjectDataById("contracts", m_Id, _items)){

        if (ui->comboOrganizations->currentIndex() == 0){ // <-selecteaza->
            int id_organization = _items.constFind("id_organizations").value().toInt();
            auto indexOrganization = model->match(model->index(0,0), Qt::UserRole, QString::number(id_organization), 1, Qt::MatchExactly);
            if (!indexOrganization.isEmpty())
                ui->comboOrganizations->setCurrentIndex(indexOrganization.first().row());
        }

        int id_typesPrices = _items.constFind("id_typesPrices").value().toInt();
        if (id_typesPrices == 0){ // 0 - nu este
            ui->comboTypesPrices->setCurrentIndex(modelTypesPrices->index(0, 0).row());
        } else {
            auto indexTypesPrices = modelTypesPrices->match(modelTypesPrices->index(0, 0), Qt::UserRole, QString::number(id_typesPrices), 1, Qt::MatchExactly);
            if (!indexTypesPrices.isEmpty())
                ui->comboTypesPrices->setCurrentIndex(indexTypesPrices.first().row());
        }

        ui->editName->setText(_items.constFind("name").value());
        ui->dateInit->setDate(QDate::fromString(_items.constFind("dateInit").value(), "yyyy-MM-dd"));
        ui->checkBoxNotValid->setCheckState((_items.constFind("notValid").value().toInt() == 1) ? Qt::Checked : Qt::Unchecked);
        ui->editComment->setPlainText(_items.constFind("comment").value());
    }

    if (!m_itNew)                  // daca nu este nou
        setWindowModified(false);  // schimbam modificarea
}

void CatContracts::slot_ItNewChanged()
{
    if (m_itNew){
        setWindowTitle(tr("Contract (crearea) %1").arg("[*]"));
    } else {
        setWindowTitle(tr("Contract (salvat) %1").arg("[*]"));
    }
}

void CatContracts::slot_IdChangedOrganization()
{
    if (m_IdOrganization == -1)
        return;

    disconnect(ui->comboOrganizations, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CatContracts::changedIndexComboOrganization);

    auto indexOrganization = model->match(model->index(0,0), Qt::UserRole, m_IdOrganization, 1, Qt::MatchExactly);
    if (!indexOrganization.isEmpty())
        ui->comboOrganizations->setCurrentIndex(indexOrganization.first().row());

    connect(ui->comboOrganizations, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CatContracts::changedIndexComboOrganization);

    if (m_itNew)
        ui->editName->setText(tr("Contract comercial"));
}

void CatContracts::changedIndexComboOrganization(const int arg1)
{
    int id_organization = ui->comboOrganizations->itemData(ui->comboOrganizations->currentIndex(), Qt::UserRole).toInt();

    qInfo(logInfo()) << tr("Selectat randul: %1").arg(QString::number(arg1))
                     << tr("A fost selectat utilizatorul cu ID:%1").arg(QString::number(id_organization));
    dataWasModified();
    setIdOrganization(id_organization);
    emit IdChangedOrganization();
}

void CatContracts::currentIndexTypesPricesChanged(const int index)
{
    Q_UNUSED(index)
    int id_types_prices = ui->comboTypesPrices->itemData(ui->comboTypesPrices->currentIndex(), Qt::UserRole).toInt();
    setIdTypesPrices(id_types_prices);
    emit IdTypesPricesChanged();
    dataWasModified();
}

bool CatContracts::onWritingData()
{
    if (!controlRequiredObjects())
        return false;

    if (m_itNew){

        setId(db->getLastIdForTable("contracts") + 1);

        if (! insertIntoTableContracts()){
            QMessageBox::warning(this, tr("Crearea obiectului."),
                                 tr("Salvarea datelor contractului \"<b>%1</b>\" nu s-a efectuat.<br>"
                                    "Adresati-va administratorului aplicatiei.")
                                     .arg(ui->comboOrganizations->currentText()), QMessageBox::Ok);
            return false;
        }

        m_itNew = false;          // setam ca obiectul nu este nou
        emit createNewContract(); // emitem signalul pu actualizarea tabelei organizatiei

    } else {

        if (! updateDataTableContracts()){
            QMessageBox::warning(this, tr("Modificarea datelor."),
                                 tr("Modificarea datelor contractului \"<b>%1</b>\" nu s-a efectuat.<br>"
                                    "Adresati-va administratorului aplicatiei.")
                                 .arg(ui->comboOrganizations->currentText()), QMessageBox::Ok);
            return false;
        }

        emit changedCatContract();
    }

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
#if defined(Q_OS_LINUX)
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Datele au fost modificate.\n"
                                    "Doriți să salvați aceste modificări ?"),
                                 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, this);
#elif defined(Q_OS_MACOS)
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Datele au fost modificate.\n"
                                    "Doriți să salvați aceste modificări ?"),
                                 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, this);
#elif defined(Q_OS_WIN)
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Datele au fost modificate.\n"
                                    "Dori\310\233i s\304\203 salva\310\233i aceste modific\304\203ri ?"),
                                 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, this);
#endif
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        messange_box.setButtonText(QMessageBox::Yes,    tr("Da"));
        messange_box.setButtonText(QMessageBox::No,     tr("Nu"));
        messange_box.setButtonText(QMessageBox::Cancel, tr("Cancel"));
//#else
//        messange_box.addButton(tr("Da"), QMessageBox::YesRole);
//        messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
//        messange_box.addButton(tr("Anuleaza"), QMessageBox::RejectRole);
#endif
        auto answer = messange_box.exec();
        if (answer == QMessageBox::Yes){
            onWritingDataClose();
            event->accept();
        } else if (answer == QMessageBox::Cancel){
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
