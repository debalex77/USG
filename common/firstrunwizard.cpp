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

#include "firstrunwizard.h"
#include "ui_firstrunwizard.h"

FirstRunWizard::FirstRunWizard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FirstRunWizard)
{
    ui->setupUi(this);

    // setam titlul ferestrei
    setWindowTitle(tr("Asistent de configurare inițială"));

    // alocam memoria pentru modelul listei
    db = new DataBase(this);
    popUp = new PopUp(this);
    model = new QStringListModel(this);
    model_investigations = new BaseSqlTableModel(this);
    model_typePrices     = new BaseSqlTableModel(this);

    QString str_users = R"(SELECT id, deletionMark, name FROM users ORDER BY name)";
    model_users = new BaseSqlQueryModel(str_users, this);
    model_users->setProperty("modelParent", BaseSqlQueryModel::ModelParent::GeneralListForm);

    QString str_doctors = R"(
        SELECT
            doctors.id,
            doctors.deletionMark,
            fullNameDoctors.name AS FullName,
            doctors.telephone,
            doctors.email,
            doctors.comment
        FROM
            doctors
        INNER JOIN
            fullNameDoctors ON doctors.id = fullNameDoctors.id_doctors
        ORDER BY
            fullNameDoctors.name;
    )";
    model_doctors = new BaseSqlQueryModel(str_doctors, this);
    model_doctors->setProperty("modelParent", BaseSqlQueryModel::ModelParent::GeneralListForm);

    QString str_nurses = R"(
        SELECT
            nurses.id,
            nurses.deletionMark,
            fullNameNurses.name AS FullName,
            nurses.telephone,
            nurses.email,
            nurses.comment
        FROM
            nurses
        INNER JOIN
            fullNameNurses ON nurses.id = fullNameNurses.id_nurses
        ORDER BY
            fullNameNurses.name;
    )";
    model_nurses = new BaseSqlQueryModel(str_nurses, this);
    model_nurses->setProperty("modelParent", BaseSqlQueryModel::ModelParent::GeneralListForm);

    QString str_organizations = R"(
        SELECT
            id,
            deletionMark,
            name,
            IDNP,
            address,
            telephone,
            email,
            comment
        FROM
            organizations
        ORDER BY
            name;
    )";
    model_organizations = new BaseSqlQueryModel(str_organizations, this);
    model_organizations->setProperty("modelParent", BaseSqlQueryModel::ModelParent::GeneralListForm);

    // setam lista de pași
    setListStep();

    // setam pagina inițială
    currentStepIndex = 0;
    ui->stackedWidget->setCurrentIndex(currentStepIndex);

    // conectăm semnalele la sloturi
    initBtnToolBar();
    initConnections();

    // actualizăm aspectul listei (pasul activ)
    updateTextListStep();

    ui->splitter->setSizes({240, 9999});

    ui->frame->setStyleSheet(R"(
        QFrame {
            background-color: #2b2b2b;
            border: 1px solid #444444;
            border-radius: 4px;
        }
        QFrame > * {
            background-color: transparent;
            color: white;
        }
    )");
    // ui->frame_2->setStyleSheet(R"(
    //     QFrame {
    //         background-color: #2b2b2b;
    //         border: 1px solid #444444;
    //         border-radius: 4px;
    //     }
    //     QFrame > * {
    //         background-color: transparent;
    //         color: white;
    //     }
    // )");

}

FirstRunWizard::~FirstRunWizard()
{
    delete popUp;
    delete model;
    delete model_doctors;
    delete model_nurses;
    delete model_investigations;
    delete model_typePrices;
    delete model_users;
    delete model_organizations;
    delete db;
    delete ui;
}

// *******************************************************************
// **************** PROCESSINGS INIT *********************************

void FirstRunWizard::initBtnToolBar()
{
    QString style_btnToolBar = db->getStyleForButtonToolBar();
    ui->btnAdd_users->setStyleSheet(style_btnToolBar);
    ui->btnEdit_users->setStyleSheet(style_btnToolBar);
    ui->btnRemove_users->setStyleSheet(style_btnToolBar);

    connect(ui->btnAdd_users, &QToolButton::clicked, this, &FirstRunWizard::addCatUser);
    connect(ui->btnEdit_users, &QToolButton::clicked, this, &FirstRunWizard::editCatUser);
    connect(ui->btnRemove_users, &QToolButton::clicked, this, &FirstRunWizard::removeCatUser);

    ui->btnAdd_doctors->setStyleSheet(style_btnToolBar);
    ui->btnEdit_doctors->setStyleSheet(style_btnToolBar);
    ui->btnRemove_doctors->setStyleSheet(style_btnToolBar);

    connect(ui->btnAdd_doctors, &QToolButton::clicked, this, &FirstRunWizard::addCatDoctor);
    connect(ui->btnEdit_doctors, &QToolButton::clicked, this, &FirstRunWizard::editCatDoctor);
    connect(ui->btnRemove_doctors, &QToolButton::clicked, this, &FirstRunWizard::removeCatDoctor);

    ui->btnAdd_nurses->setStyleSheet(style_btnToolBar);
    ui->btnEdit_nurses->setStyleSheet(style_btnToolBar);
    ui->btnRemove_nurses->setStyleSheet(style_btnToolBar);

    connect(ui->btnAdd_nurses, &QToolButton::clicked, this, &FirstRunWizard::addCatNurse);
    connect(ui->btnEdit_nurses, &QToolButton::clicked, this, &FirstRunWizard::editCatNurse);
    connect(ui->btnRemove_nurses, &QToolButton::clicked, this, &FirstRunWizard::removeCatNurse);

    ui->btnAdd_organizations->setStyleSheet(style_btnToolBar);
    ui->btnEdit_organizations->setStyleSheet(style_btnToolBar);
    ui->btnRemove_organizations->setStyleSheet(style_btnToolBar);

    connect(ui->btnAdd_organizations, &QToolButton::clicked, this, &FirstRunWizard::addCatOrganization);
    connect(ui->btnEdit_organizations, &QToolButton::clicked, this, &FirstRunWizard::editCatOrganization);
    connect(ui->btnRemove_organizations, &QToolButton::clicked, this, &FirstRunWizard::removeCatOrganization);

}

void FirstRunWizard::initConnections()
{
    connect(ui->btnLoadInvestigations, &QPushButton::clicked, this, &FirstRunWizard::loadInvestigations);
    connect(ui->btnLoadTypePrices, &QPushButton::clicked, this, &FirstRunWizard::loadTypePrices);

    connect(ui->btnBack, &QPushButton::clicked, this, &FirstRunWizard::clickedBackPage);
    connect(ui->btnNext, &QPushButton::clicked, this, &FirstRunWizard::clickedNextPage);
    connect(ui->btnClose, &QPushButton::clicked, this, &FirstRunWizard::close);
}

void FirstRunWizard::setListStep()
{
    QStringList listStep {
        tr("1. Inițierea"),
        tr("2. Clasificator \"Investigații\"."),
        tr("3. Tipul prețurilor"),
        tr("4. Utilizatori"),
        tr("5. Doctori"),
        tr("6. As.medicale"),
        tr("7. Organizații/centre medicale")
    };

    model->setStringList(listStep);
    ui->listView->setModel(model);
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->listView->setFocusPolicy(Qt::NoFocus);
    ui->listView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->listView->setStyleSheet(R"(
        QListView {
            background-color: transparent;
            border: none;
        }
        QListView::item {
            color: white;
            padding: 6px;
            font-size: 14px;
        }
        QListView::item:selected {
            font-weight: bold;
            text-decoration: underline;
            background-color: transparent;
            color: #00BFFF;
        }
    )");
}

// *******************************************************************
// **************** PROCESSINGS UPDATE TABLES ************************

void FirstRunWizard::updateTextListStep()
{
    QModelIndex index = model->index(currentStepIndex);
    ui->listView->setCurrentIndex(index);
    ui->listView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
}

void FirstRunWizard::updateTableInvestigations()
{
    if (ui->stackedWidget->currentIndex() != page_classifiers)
        return;

    model_investigations->setTable("investigations");
    model_investigations->setSort(0, Qt::AscendingOrder);

    model_investigations->setHeaderData(0, Qt::Horizontal, "ID");
    model_investigations->setHeaderData(1, Qt::Horizontal, "");
    model_investigations->setHeaderData(2, Qt::Horizontal, "Cod MS");
    model_investigations->setHeaderData(3, Qt::Horizontal, "Denumirea investigatiei");

    ui->tableView->setModel(model_investigations);

    ui->tableView->setColumnWidth(1, 25);
    ui->tableView->setColumnHidden(0, true); // ascundem id
    ui->tableView->setColumnWidth(2, 70);
    ui->tableView->setColumnWidth(3, 400); // denuimirea investigatiei

    model_investigations->select();

    ui->tableView->setColumnHidden(4, true);
    ui->tableView->setColumnHidden(5, true);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    if (globals().firstLaunch)
        ui->progressBar->setValue(0);
    else
        ui->progressBar->setValue(100);

    if (model_investigations->rowCount() > 0) {
        ui->tableView->setFocus();   // focusam la tabela
        ui->tableView->selectRow(0); // selectam 1 rand
    }
}

void FirstRunWizard::updateTableTypePrices()
{
    if (ui->stackedWidget->currentIndex() != page_typePrices)
        return;

    model_typePrices->setTable("typesPrices");
    model_typePrices->setSort(0, Qt::AscendingOrder);

    model_typePrices->setHeaderData(0, Qt::Horizontal, "ID");
    model_typePrices->setHeaderData(1, Qt::Horizontal, "");
    model_typePrices->setHeaderData(2, Qt::Horizontal, "Denumirea pretului");
    model_typePrices->setHeaderData(3, Qt::Horizontal, "Reducere (%)");
    model_typePrices->setHeaderData(4, Qt::Horizontal, "Noncomercial");

    ui->tableView_typePrices->setModel(model_typePrices);

    ui->tableView_typePrices->setColumnWidth(1, 25);
    ui->tableView_typePrices->setColumnHidden(0, true); // ascundem id
    ui->tableView_typePrices->setColumnWidth(2, 450);
    ui->tableView_typePrices->setColumnHidden(3, true);
    ui->tableView_typePrices->setColumnHidden(4, true);
    ui->tableView_typePrices->horizontalHeader()->setStretchLastSection(true);

    model_typePrices->select();

    if (model_typePrices->rowCount() > 0) {
        ui->tableView_typePrices->setFocus();   // focusam la tabela
        ui->tableView_typePrices->selectRow(0); // selectam primul rand
    }
}

void FirstRunWizard::updateTableUsers()
{
    if (ui->stackedWidget->currentIndex() != page_users)
        return;

    model_users->clear();
    model_users->setQuery("SELECT id, deletionMark, name FROM users ORDER BY name");
    model_users->setHeaderData(1, Qt::Horizontal, "");
    model_users->setHeaderData(2, Qt::Horizontal, "Nume utilizatorilor");

    ui->tableView_users->setModel(model_users);
    ui->tableView_users->hideColumn(0);                                                      // id-ascundem
    ui->tableView_users->setColumnWidth(1, 25);
    ui->tableView_users->setSelectionBehavior(QAbstractItemView::SelectRows);                // setam alegerea randului
    ui->tableView_users->setSelectionMode(QAbstractItemView::SingleSelection);               // setam singura alegerea(nu multipla)
    ui->tableView_users->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // permitem schimbarea size sectiilor
    ui->tableView_users->horizontalHeader()->setStretchLastSection(true);                    // extinderea ultimei sectiei

    if (model_users->rowCount() > 0) {
        ui->tableView_users->setFocus();   // focusam la tabela
        ui->tableView_users->selectRow(0); // selectam primul rand
    }
}

void FirstRunWizard::updateTableDoctors()
{
    if (ui->stackedWidget->currentIndex() != page_doctors)
        return;

    model_doctors->clear();
    model_doctors->setQuery(R"(
        SELECT
            doctors.id,
            doctors.deletionMark,
            fullNameDoctors.name AS FullName,
            doctors.telephone,
            doctors.email,
            doctors.comment
        FROM
            doctors
        INNER JOIN
            fullNameDoctors ON doctors.id = fullNameDoctors.id_doctors
        ORDER BY
            fullNameDoctors.name
    )");
    model_doctors->setHeaderData(1, Qt::Horizontal, "");
    model_doctors->setHeaderData(2, Qt::Horizontal, tr("NPP doctorului"));
    model_doctors->setHeaderData(3, Qt::Horizontal, tr("Telefoane"));
    model_doctors->setHeaderData(4, Qt::Horizontal, tr("E-mail"));
    model_doctors->setHeaderData(5, Qt::Horizontal, tr("Comentariu"));

    ui->tableView_doctors->setModel(model_doctors);
    ui->tableView_doctors->hideColumn(0);                                                      // id-ascundem
    ui->tableView_doctors->setColumnWidth(1, 25);
    ui->tableView_doctors->setColumnWidth(2, 160);
    ui->tableView_doctors->setColumnWidth(3, 90);
    ui->tableView_doctors->setColumnWidth(4, 160);
    ui->tableView_doctors->setSelectionBehavior(QAbstractItemView::SelectRows);                // setam alegerea randului
    ui->tableView_doctors->setSelectionMode(QAbstractItemView::SingleSelection);               // setam singura alegerea(nu multipla)
    ui->tableView_doctors->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // permitem schimbarea size sectiilor
    ui->tableView_doctors->horizontalHeader()->setStretchLastSection(true);                    // extinderea ultimei sectiei

    if (model_doctors->rowCount() > 0) {
        ui->tableView_doctors->setFocus();   // focusam la tabela
        ui->tableView_doctors->selectRow(0); // selectam primul rand
    }
}

void FirstRunWizard::updateTableNurses()
{
    if (ui->stackedWidget->currentIndex() != page_nurses)
        return;

    model_nurses->clear();
    model_nurses->setQuery(R"(
        SELECT
            nurses.id,
            nurses.deletionMark,
            fullNameNurses.name AS FullName,
            nurses.telephone,
            nurses.email,
            nurses.comment
        FROM
            nurses
        INNER JOIN
            fullNameNurses ON nurses.id = fullNameNurses.id_nurses
        ORDER BY
            fullNameNurses.name;
    )");
    model_nurses->setHeaderData(1, Qt::Horizontal, "");
    model_nurses->setHeaderData(2, Qt::Horizontal, tr("NPP as.medicale"));
    model_nurses->setHeaderData(3, Qt::Horizontal, tr("Telefoane"));
    model_nurses->setHeaderData(4, Qt::Horizontal, tr("E-mail"));
    model_nurses->setHeaderData(5, Qt::Horizontal, tr("Comentariu"));

    ui->tableView_nurses->setModel(model_nurses);
    ui->tableView_nurses->hideColumn(0);                                                      // id-ascundem
    ui->tableView_nurses->setColumnWidth(1, 25);
    ui->tableView_nurses->setColumnWidth(2, 160);
    ui->tableView_nurses->setColumnWidth(3, 90);
    ui->tableView_nurses->setColumnWidth(4, 160);
    ui->tableView_nurses->setSelectionBehavior(QAbstractItemView::SelectRows);                // setam alegerea randului
    ui->tableView_nurses->setSelectionMode(QAbstractItemView::SingleSelection);               // setam singura alegerea(nu multipla)
    ui->tableView_nurses->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // permitem schimbarea size sectiilor
    ui->tableView_nurses->horizontalHeader()->setStretchLastSection(true);                    // extinderea ultimei sectiei

    if (model_nurses->rowCount() > 0) {
        ui->tableView_nurses->setFocus();   // focusam la tabela
        ui->tableView_nurses->selectRow(0); // selectam primul rand
    }
}

void FirstRunWizard::updateTableOrganizations()
{
    if (ui->stackedWidget->currentIndex() != page_organizations)
        return;

    model_organizations->clear();
    model_organizations->setQuery(R"(
        SELECT
            id,
            deletionMark,
            name,
            IDNP,
            address,
            telephone,
            email,
            comment
        FROM
            organizations
        ORDER BY
            name;
    )");
    model_organizations->setHeaderData(1, Qt::Horizontal, "");
    model_organizations->setHeaderData(2, Qt::Horizontal, tr("Denumirea organizației"));
    model_organizations->setHeaderData(3, Qt::Horizontal, tr("IDNP"));
    model_organizations->setHeaderData(4, Qt::Horizontal, tr("Adresa"));
    model_organizations->setHeaderData(5, Qt::Horizontal, tr("Telefoane"));
    model_organizations->setHeaderData(6, Qt::Horizontal, tr("E-mail"));
    model_organizations->setHeaderData(7, Qt::Horizontal, tr("Comentariu"));

    ui->tableView_organizations->setModel(model_organizations);
    ui->tableView_organizations->hideColumn(0);                                                      // id-ascundem
    ui->tableView_organizations->setColumnWidth(1, 25);
    ui->tableView_organizations->setColumnWidth(2, 200);
    ui->tableView_organizations->setColumnWidth(3, 120);
    ui->tableView_organizations->setColumnWidth(4, 240);
    ui->tableView_organizations->setColumnWidth(5, 230);
    ui->tableView_organizations->setColumnWidth(6, 216);
    ui->tableView_organizations->setSelectionBehavior(QAbstractItemView::SelectRows);                // setam alegerea randului
    ui->tableView_organizations->setSelectionMode(QAbstractItemView::SingleSelection);               // setam singura alegerea(nu multipla)
    ui->tableView_organizations->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // permitem schimbarea size sectiilor
    ui->tableView_organizations->horizontalHeader()->setStretchLastSection(true);                    // extinderea ultimei sectiei

    if (model_organizations->rowCount() > 0) {
        ui->tableView_organizations->setFocus();   // focusam la tabela
        ui->tableView_organizations->selectRow(0); // selectam primul rand
    }
}

// *******************************************************************
// **************** PROCESSINGS BTN NEXT, BACK ***********************

void FirstRunWizard::clickedBackPage()
{
    if (currentStepIndex > 0) {
        currentStepIndex--;
        ui->stackedWidget->setCurrentIndex(currentStepIndex);
        updateTextListStep();
        updateTableInvestigations();
        updateTableTypePrices();
        updateTableUsers();
        updateTableDoctors();
        updateTableNurses();
        updateTableOrganizations();
    }
}

void FirstRunWizard::clickedNextPage()
{
    if (currentStepIndex < ui->stackedWidget->count() - 1) {
        currentStepIndex++;
        ui->stackedWidget->setCurrentIndex(currentStepIndex);
        updateTextListStep();
        updateTableInvestigations();
        updateTableTypePrices();
        updateTableUsers();
        updateTableDoctors();
        updateTableNurses();
        updateTableOrganizations();
    }
}

// *******************************************************************
// **************** PROCESAREA BTN LOAD ******************************

void FirstRunWizard::loadInvestigations()
{
    if (! globals().firstLaunch)
        return;

    db->updateInvestigationFromXML_2024();
    connect(db, &DataBase::updateProgress, this, &FirstRunWizard::handleUpdateProgress);
    connect(db, &DataBase::finishedProgress, this, &FirstRunWizard::handleFinishedProgress);
}

void FirstRunWizard::loadTypePrices()
{
    if (! globals().firstLaunch)
        return;

    db->insertDataForTabletypesPrices();
    updateTableTypePrices();
    emit finishLoadClassifier(tr("Completat catalogul \"Tipul prețurilor\"."));
}

void FirstRunWizard::handleUpdateProgress(int num_records, int value)
{
    if (num_records <= 0)
        return;

    // Calculează progresul
    int value_progress = (value * 100) / num_records;

    ui->progressBar->setValue(value_progress);
}

void FirstRunWizard::handleFinishedProgress(const QString txt)
{
    updateTableInvestigations();
    emit finishLoadClassifier(txt);
}

// *******************************************************************
// **************** PROCESAREA BTN USER ******************************

void FirstRunWizard::addCatUser()
{
    CatUsers* cat_user = new CatUsers(this);
    cat_user->setProperty("itNew", true);
    connect(cat_user, &CatUsers::mCreateNewUser, this, [this] (){
        updateTableUsers();
    });
    cat_user->exec();
}

void FirstRunWizard::editCatUser()
{
    if (ui->tableView_users->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Atenție"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    auto row = ui->tableView_users->currentIndex().row();
    int id_user = model_users->data(model_users->index(row, 0), Qt::DisplayRole).toInt();

    CatUsers* cat_user = new CatUsers(this);
    cat_user->setProperty("itNew", false);
    cat_user->setProperty("Id", id_user);
    cat_user->exec();
}

void FirstRunWizard::removeCatUser()
{
    // 1. Verificam daca este marcat randul
    if (ui->tableView_users->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Atenție"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    // 2. pregatim variabile necesare
    auto row = ui->tableView_users->currentIndex().row();
    int id_user       = model_users->data(model_users->index(row, 0), Qt::DisplayRole).toInt();
    QString name_user = model_users->data(model_users->index(row, 2), Qt::DisplayRole).toString();

    // 3. preagatim soliciatre
    QString strQry;
    strQry = globals().thisMySQL ?
        QStringLiteral(R"(
            SELECT
                CONCAT('Comanda ecografica nr.', numberDoc ,' din ', DATE_FORMAT(dateDoc, '%d.%m.%Y')) AS typeDoc
            FROM
                orderEcho
            WHERE
                id_users = ?

            UNION ALL

            SELECT
                CONCAT('Raport ecografic nr.', numberDoc ,' din ', DATE_FORMAT(dateDoc, '%d.%m.%Y')) AS typeDoc
            FROM
                reportEcho
            WHERE
                id_users = ?

            UNION ALL

            SELECT
                CONCAT('Formarea preturilor nr.', numberDoc ,' din ', DATE_FORMAT(dateDoc, '%d.%m.%Y')) AS typeDoc
            FROM
                pricings
            WHERE
                id_users = ?

            UNION ALL

            SELECT
                'Preferintele utilizatorului' AS typeDoc
            FROM
                userPreferences
            WHERE
                id_users = ?

            UNION ALL

            SELECT
                'Cont online' AS typeDoc
            FROM
                contsOnline
            WHERE
                id_users = ?

            UNION ALL

            SELECT
                'Cloud server' AS typeDoc
            FROM
                cloudServer
            WHERE
                id_users = ?
        )") :
        QStringLiteral(R"(
            SELECT
                'Comanda ecografica nr.'|| numberDoc ||' din '|| strftime('%d.%m.%Y', dateDoc) AS typeDoc
            FROM
                orderEcho
            WHERE
                id_users = ?

            UNION ALL

            SELECT
                'Raport ecografic nr.'|| numberDoc ||' din '|| strftime('%d.%m.%Y', dateDoc) AS typeDoc
            FROM
                reportEcho
            WHERE
                id_users = ?

            UNION ALL

            SELECT
                'Formarea preturilor nr.' || numberDoc || ' din ' || strftime('%d.%m.%Y', dateDoc) AS typeDoc
            FROM
                pricings
            WHERE
                id_users = ?

            UNION ALL

            SELECT
                'Preferintele utilizatorului' AS typeDoc
            FROM
                userPreferences
            WHERE
                id_users = ?

            UNION ALL

            SELECT
                'Cont online' AS typeDoc
            FROM
                contsOnline
            WHERE
                id_users = ?

            UNION ALL

            SELECT
                'Cloud server' AS typeDoc
            FROM
                cloudServer
            WHERE
                id_users = ?
        )");

    // 4. executam
    QSqlQuery qry;
    qry.prepare(strQry);
    qry.addBindValue(id_user); //pu orderEcho
    qry.addBindValue(id_user); //pu reportEcho
    qry.addBindValue(id_user); //pu pricings
    qry.addBindValue(id_user); //pu userPreferences
    qry.addBindValue(id_user); //pu contsOnline
    qry.addBindValue(id_user); //pu cloudServer
    if (qry.exec()) {
        QStringList list_doc;

        list_doc << tr("Utilizatorul '%1' figureaza in urmatoarele documente:")
                        .arg(name_user);

        while (qry.next()) {
            QString doc_type = qry.value(0).toString();
            list_doc << tr(" - %1").arg(doc_type);
        }

        if (list_doc.size() > 1) { // > 1 din cauza initierea textului vezi mai sus
            CustomMessage *message = new CustomMessage(this);
            message->setWindowTitle(QGuiApplication::applicationDisplayName());
            message->setTextTitle(tr("Utilizatorul '%1' nu poate fi eliminat !!!").arg(name_user));
            message->setDetailedText(list_doc.join("\n"));
            message->exec();
            message->deleteLater();

            return;
        }
    } else {
        err.clear();
        err << this->metaObject()->className()
            << "[removeCatUser]"
            << tr("Eroarea solicitarii de selectare a utilizatorului %1")
                   .arg(qry.lastError().text().isEmpty()
                            ? ": eroarea indisponibila"
                            : ": " + qry.lastError().text());

        qCritical(logCritical()) << err;

        CustomMessage *message = new CustomMessage(this);
        message->setWindowTitle(QGuiApplication::applicationDisplayName());
        message->setTextTitle(tr("Utilizatorul '%1' nu poate fi eliminat !!!").arg(name_user));
        message->setDetailedText(err.join("\n"));
        message->exec();
        message->deleteLater();

        return;
    }

    // 5. eliminam din baza de date
    if (db->deleteDataFromTable("users", "id", QString::number(id_user))) {
        popUp->setPopupText(tr("Utilizatorul '%1' eliminat cu cucces din baza de date.")
                                .arg(name_user));
        popUp->show();

        qInfo(logInfo()) << "Utilizatorul " + name_user + " eliminat din baza de date.";
        updateTableUsers();
    }
}

// *******************************************************************
// **************** PROCESAREA BTN DOCTOR ****************************

void FirstRunWizard::addCatDoctor()
{
    CatGeneral *cat_General = new CatGeneral(this);
    cat_General->setProperty("itNew", true);
    cat_General->setProperty("typeCatalog", CatGeneral::TypeCatalog::Doctors);
    connect(cat_General, &CatGeneral::createCatGeneral, this, [this]() {
        updateTableDoctors();
    });
    cat_General->exec();
}

void FirstRunWizard::editCatDoctor()
{
    if (ui->tableView_doctors->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Atenție"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    auto row = ui->tableView_doctors->currentIndex().row();
    int id_doctor = model_doctors->data(model_doctors->index(row, 0), Qt::DisplayRole).toInt();

    CatGeneral *cat_General = new CatGeneral(this);
    cat_General->setProperty("itNew", false);
    cat_General->setProperty("typeCatalog", CatGeneral::TypeCatalog::Doctors);
    cat_General->setProperty("Id", id_doctor);
    connect(cat_General, &CatGeneral::changedCatGeneral, this, [this]() {
        updateTableDoctors();
    });
    cat_General->exec();
}

void FirstRunWizard::removeCatDoctor()
{
    // 1. Verificam daca este marcat randul
    if (ui->tableView_doctors->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Atenție"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    // 2. pregatim variabile necesare
    auto row = ui->tableView_doctors->currentIndex().row();
    int id_doctor       = model_doctors->data(model_doctors->index(row, 0), Qt::DisplayRole).toInt();
    QString name_doctor = model_doctors->data(model_doctors->index(row, 2), Qt::DisplayRole).toString();

    // 3. preagatim soliciatre
    QString strQry;
    strQry = globals().thisMySQL ?
        QStringLiteral(R"(
            SELECT
                CONCAT('Comanda ecografica nr.', numberDoc ,' din ', DATE_FORMAT(dateDoc, '%d.%m.%Y')) AS typeDoc
            FROM
                orderEcho
            WHERE
                id_doctors = ? OR id_doctors_execute = ?

            UNION ALL

            SELECT
                'Preferintele utilizatorului' AS typeDoc
            FROM
                constants
            WHERE
                id_doctors = ?
        )") :
        QStringLiteral(R"(
            SELECT
                'Comanda ecografica nr.'|| numberDoc ||' din '|| strftime('%d.%m.%Y', dateDoc) AS typeDoc
            FROM
                orderEcho
            WHERE
                id_doctors = ? OR id_doctors_execute = ?

            UNION ALL

            SELECT
                'Preferintele utilizatorului' AS typeDoc
            FROM
                constants
            WHERE
                id_doctors = ?
        )");

    // 4. executam
    QSqlQuery qry;
    qry.prepare(strQry);
    qry.addBindValue(id_doctor); //pu orderEcho - id_doctors
    qry.addBindValue(id_doctor); //pu orderEcho - id_doctors_execute
    qry.addBindValue(id_doctor); //pu constants
    if (qry.exec()) {

        QStringList list_doc;

        list_doc << tr("Doctor '%1' figureaza in urmatoarele documente:")
                        .arg(name_doctor);

        while (qry.next()) {
            QString doc_type = qry.value(0).toString();
            list_doc << tr(" - %1").arg(doc_type);
        }

        if (list_doc.size() > 1) { // > 1 din cauza initierea textului vezi mai sus
            CustomMessage *message = new CustomMessage(this);
            message->setWindowTitle(QGuiApplication::applicationDisplayName());
            message->setTextTitle(tr("Doctor '%1' nu poate fi eliminat !!!").arg(name_doctor));
            message->setDetailedText(list_doc.join("\n"));
            message->exec();
            message->deleteLater();

            return;
        }
    } else {
        err.clear();
        err << this->metaObject()->className()
            << "[removeCatDoctor]"
            << tr("Eroarea solicitarii de selectare a doctorului %1")
                   .arg(qry.lastError().text().isEmpty()
                            ? ": eroarea indisponibila"
                            : ": " + qry.lastError().text());

        qCritical(logCritical()) << err;

        CustomMessage *message = new CustomMessage(this);
        message->setWindowTitle(QGuiApplication::applicationDisplayName());
        message->setTextTitle(tr("Doctor '%1' nu poate fi eliminat !!!").arg(name_doctor));
        message->setDetailedText(err.join("\n"));
        message->exec();
        message->deleteLater();

        return;
    }

    // 5. eliminam din baza de date
    if (db->deleteDataFromTable("doctors", "id", QString::number(id_doctor))) {
        popUp->setPopupText(tr("Doctor '%1' eliminat cu cucces din baza de date.")
                                .arg(name_doctor));
        popUp->show();

        qInfo(logInfo()) << "Doctor " + name_doctor + " eliminat din baza de date.";
        updateTableDoctors();
    }
}

// *******************************************************************
// **************** PROCESAREA BTN NURSE *****************************

void FirstRunWizard::addCatNurse()
{
    CatGeneral *cat_General = new CatGeneral(this);
    cat_General->setProperty("itNew", true);
    cat_General->setProperty("typeCatalog", CatGeneral::TypeCatalog::Nurses);
    connect(cat_General, &CatGeneral::createCatGeneral, this, [this]() {
        updateTableNurses();
    });
    cat_General->exec();
}

void FirstRunWizard::editCatNurse()
{
    if (ui->tableView_nurses->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Atenție"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    auto row = ui->tableView_nurses->currentIndex().row();
    int id_nurse = model_nurses->data(model_nurses->index(row, 0), Qt::DisplayRole).toInt();

    CatGeneral *cat_General = new CatGeneral(this);
    cat_General->setProperty("itNew", false);
    cat_General->setProperty("typeCatalog", CatGeneral::TypeCatalog::Nurses);
    cat_General->setProperty("Id", id_nurse);
    connect(cat_General, &CatGeneral::changedCatGeneral, this, [this]() {
        updateTableNurses();
    });
    cat_General->exec();
}

void FirstRunWizard::removeCatNurse()
{
    // 1. Verificam daca este marcat randul
    if (ui->tableView_nurses->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Atenție"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    // 2. pregatim variabile necesare
    auto row = ui->tableView_nurses->currentIndex().row();
    int id_nurse       = model_nurses->data(model_nurses->index(row, 0), Qt::DisplayRole).toInt();
    QString name_nurse = model_nurses->data(model_nurses->index(row, 2), Qt::DisplayRole).toString();

    // 3. preagatim soliciatre
    QString strQry;
    strQry = globals().thisMySQL ?
        QStringLiteral(R"(
            SELECT
                CONCAT('Comanda ecografica nr.', numberDoc ,' din ', DATE_FORMAT(dateDoc, '%d.%m.%Y')) AS typeDoc
            FROM
                orderEcho
            WHERE
                id_nurses = ?

            UNION ALL

            SELECT
                'Preferintele utilizatorului' AS typeDoc
            FROM
                constants
            WHERE
                id_nurses = ?
        )") :
        QStringLiteral(R"(
            SELECT
                'Comanda ecografica nr.'|| numberDoc ||' din '|| strftime('%d.%m.%Y', dateDoc) AS typeDoc
            FROM
                orderEcho
            WHERE
                id_nurses = ?

            UNION ALL

            SELECT
                'Preferintele utilizatorului' AS typeDoc
            FROM
                constants
            WHERE
                id_nurses = ?
        )");

    // 4. executam
    QSqlQuery qry;
    qry.prepare(strQry);
    qry.addBindValue(id_nurse); //pu orderEcho
    qry.addBindValue(id_nurse); //pu constants
    if (qry.exec()) {

        QStringList list_doc;

        list_doc << tr("As.medicala '%1' figureaza in urmatoarele documente:")
                        .arg(name_nurse);

        while (qry.next()) {
            QString doc_type = qry.value(0).toString();
            list_doc << tr(" - %1").arg(doc_type);
        }

        if (list_doc.size() > 1) { // > 1 din cauza initierea textului vezi mai sus
            CustomMessage *message = new CustomMessage(this);
            message->setWindowTitle(QGuiApplication::applicationDisplayName());
            message->setTextTitle(tr("As.medicala '%1' nu poate fi eliminat !!!").arg(name_nurse));
            message->setDetailedText(list_doc.join("\n"));
            message->exec();
            message->deleteLater();

            return;
        }
    } else {
        err.clear();
        err << this->metaObject()->className()
            << "[removeCatNurse]"
            << tr("Eroarea solicitarii de selectare a as.medicale %1")
                   .arg(qry.lastError().text().isEmpty()
                            ? ": eroarea indisponibila"
                            : ": " + qry.lastError().text());

        qCritical(logCritical()) << err;

        CustomMessage *message = new CustomMessage(this);
        message->setWindowTitle(QGuiApplication::applicationDisplayName());
        message->setTextTitle(tr("As.medicala '%1' nu poate fi eliminat !!!").arg(name_nurse));
        message->setDetailedText(err.join("\n"));
        message->exec();
        message->deleteLater();

        return;
    }


    // 5. eliminam din baza de date
    if (db->deleteDataFromTable("nurses", "id", QString::number(id_nurse))) {
        popUp->setPopupText(tr("As.medicala '%1' eliminat cu cucces din baza de date.")
                                .arg(name_nurse));
        popUp->show();

        qInfo(logInfo()) << "As.medicala " + name_nurse + " eliminat din baza de date.";
        updateTableNurses();
    }
}

// *******************************************************************
// **************** PROCESAREA BTN ORGANIZATION **********************

void FirstRunWizard::addCatOrganization()
{
    CatOrganizations *catOrganization = new CatOrganizations(this);
    catOrganization->setProperty("itNew", true);
    connect(catOrganization, &CatOrganizations::mWriteNewOrganization, this, [this]() {
        updateTableOrganizations();
    });
    catOrganization->exec();
}

void FirstRunWizard::editCatOrganization()
{
    if (ui->tableView_organizations->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Atenție"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    auto row = ui->tableView_organizations->currentIndex().row();
    int id_organization = model_organizations->data(model_organizations->index(row, 0), Qt::DisplayRole).toInt();

    CatOrganizations *catOrganization = new CatOrganizations(this);
    catOrganization->setProperty("itNew", false);
    catOrganization->setProperty("Id", id_organization);
    connect(catOrganization, &CatOrganizations::mChangedDateOrganization, this, [this]() {
        updateTableOrganizations();
    });
    catOrganization->exec();
}

void FirstRunWizard::removeCatOrganization()
{
    // 1. Verificam daca este marcat randul
    if (ui->tableView_organizations->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Atenție"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    // 2. pregatim variabile necesare
    auto row = ui->tableView_organizations->currentIndex().row();
    int id_organizations       = model_organizations->data(model_organizations->index(row, 0), Qt::DisplayRole).toInt();
    QString name_organizations = model_organizations->data(model_organizations->index(row, 2), Qt::DisplayRole).toString();

    // 3. preagatim soliciatre
    QString strQry;
    strQry = globals().thisMySQL ?
        QStringLiteral(R"(
            SELECT
                CONCAT('Comanda ecografica nr.', numberDoc ,' din ', DATE_FORMAT(dateDoc, '%d.%m.%Y')) AS typeDoc
            FROM
                orderEcho
            WHERE
                id_organizations = ?

            UNION ALL

            SELECT
                CONCAT('Formarea preturilor nr.', numberDoc ,' din ', DATE_FORMAT(dateDoc, '%d.%m.%Y')) AS typeDoc
            FROM
                pricings
            WHERE
                id_organizations = ?

            UNION ALL

            SELECT
                'Preferintele utilizatorului' AS typeDoc
            FROM
                constants
            WHERE
                id_organizations = ?

            UNION ALL

            SELECT
                'Cont online' AS typeDoc
            FROM
                contsOnline
            WHERE
                id_organizations = ?

            UNION ALL

            SELECT
                'Cloud server' AS typeDoc
            FROM
                cloudServer
            WHERE
                id_organizations = ?
        )") :
        QStringLiteral(R"(
            SELECT
                'Comanda ecografica nr.'|| numberDoc ||' din '|| strftime('%d.%m.%Y', dateDoc) AS typeDoc
            FROM
                orderEcho
            WHERE
                id_organizations = ?

            UNION ALL

            SELECT
                'Formarea preturilor nr.' || numberDoc || ' din ' || strftime('%d.%m.%Y', dateDoc) AS typeDoc
            FROM
                pricings
            WHERE
                id_organizations = ?

            UNION ALL

            SELECT
                'Preferintele utilizatorului' AS typeDoc
            FROM
                constants
            WHERE
                id_organizations = ?

            UNION ALL

            SELECT
                'Cont online' AS typeDoc
            FROM
                contsOnline
            WHERE
                id_organizations = ?

            UNION ALL

            SELECT
                'Cloud server' AS typeDoc
            FROM
                cloudServer
            WHERE
                id_organizations = ?
        )");

    // 4. executam
    QSqlQuery qry;
    qry.prepare(strQry);
    qry.addBindValue(id_organizations); //pu orderEcho
    qry.addBindValue(id_organizations); //pu pricings
    qry.addBindValue(id_organizations); //pu constants
    qry.addBindValue(id_organizations); //pu contsOnline
    qry.addBindValue(id_organizations); //pu cloudServer
    if (qry.exec()) {

        QStringList list_doc;

        list_doc << tr("Organizatia '%1' figureaza in urmatoarele documente:")
                        .arg(name_organizations);

        while (qry.next()) {
            QString doc_type = qry.value(0).toString();
            list_doc << tr(" - %1").arg(doc_type);
        }

        if (list_doc.size() > 1) { // > 1 din cauza initierea textului vezi mai sus
            CustomMessage *message = new CustomMessage(this);
            message->setWindowTitle(QGuiApplication::applicationDisplayName());
            message->setTextTitle(tr("Organizatia '%1' nu poate fi eliminat !!!").arg(name_organizations));
            message->setDetailedText(list_doc.join("\n"));
            message->exec();
            message->deleteLater();

            return;
        }

    } else {
        err.clear();
        err << this->metaObject()->className()
            << "[removeCatOrganization]"
            << tr("Eroarea solicitarii de selectare a organizatiei %1")
                   .arg(qry.lastError().text().isEmpty()
                            ? ": eroarea indisponibila"
                            : ": " + qry.lastError().text());

        qCritical(logCritical()) << err;

        CustomMessage *message = new CustomMessage(this);
        message->setWindowTitle(QGuiApplication::applicationDisplayName());
        message->setTextTitle(tr("Organizatia '%1' nu poate fi eliminat !!!").arg(name_organizations));
        message->setDetailedText(err.join("\n"));
        message->exec();
        message->deleteLater();

        return;
    }

    // 5. eliminam din baza de date
    if (db->deleteDataFromTable("organizations", "id", QString::number(id_organizations))) {
        popUp->setPopupText(tr("Organizatia '%1' eliminat cu cucces din baza de date.")
                                .arg(name_organizations));
        popUp->show();

        qInfo(logInfo()) << "Organizatia " + name_organizations + " eliminat din baza de date.";
        updateTableOrganizations();
    }
}
