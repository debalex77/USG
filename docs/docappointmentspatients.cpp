#include "docappointmentspatients.h"
#include "ui_docappointmentspatients.h"

DocAppointmentsPatients::DocAppointmentsPatients(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DocAppointmentsPatients)
{
    ui->setupUi(this);

    // ---------------------------------------------------------------
    setWindowTitle(tr("Programarea pacienților %1").arg("[*]"));    // setam titlul ferestrei
    setWindowIcon(QIcon(":/img/registration_patients.png"));        // setam icon ferestrei

    // ---------------------------------------------------------------
    db    = new DataBase(this); // alocam memoria
    popUp = new PopUp(this);

    // ---------------------------------------------------------------
    setStyleSheetButtoms();

    // ---------------------------------------------------------------
    // setam data curenta si textul datei
    ui->docDate->setDate(QDate::currentDate());

    // ---------------------------------------------------------------
    model_main = new RegistrationPatientsModel(m_rows, m_column, this); // modelam tabela
    update_model();                                                     // actualizam tabela

    // ---------------------------------------------------------------
    // initiem solicitarea pu delegate combobox
    QString qry_organizations = "SELECT id,name FROM organizations WHERE deletionMark = 0;";
    delegate_organizations = new ComboDelegate(qry_organizations, ui->tableView);
    ui->tableView->setItemDelegateForColumn(section_organization - 1, delegate_organizations);

    QString qry_doctors;
    if (globals::thisMySQL)
        qry_doctors = "SELECT doctors.id, CONCAT(doctors.name,' ', SUBSTRING(doctors.fName, 1, 1) ,'.') AS fullName FROM doctors WHERE deletionMark = 0 ORDER BY doctors.name;";
    else
        qry_doctors = "SELECT doctors.id, doctors.name ||' '|| substr(doctors.fName, 1, 1) ||'.' AS fullName FROM doctors WHERE deletionMark = 0 ORDER BY doctors.name;";
    delegate_doctors = new ComboDelegate(qry_doctors, ui->tableView);
    ui->tableView->setItemDelegateForColumn(section_doctor - 1, delegate_doctors);

    delegate_execute = new CheckBoxDelegate(ui->tableView);
    ui->tableView->setItemDelegateForColumn(section_execute - 1, delegate_execute);

    // ---------------------------------------------------------------
    ui->btnOrderEcho->setMouseTracking(true);
    ui->btnOrderEcho->installEventFilter(this);
    ui->btnWrite->setMouseTracking(true);
    ui->btnWrite->installEventFilter(this);
    ui->btnPrint->setMouseTracking(true);
    ui->btnPrint->installEventFilter(this);
    ui->btnRemove->setMouseTracking(true);
    ui->btnRemove->installEventFilter(this);
    ui->btnClose->setMouseTracking(true);
    ui->btnClose->installEventFilter(this);

    connect(ui->btnWrite, &QToolButton::clicked, this, &DocAppointmentsPatients::writeRegistration);
    connect(ui->btnPrint, &QToolButton::clicked, this, &DocAppointmentsPatients::onClickPrint);
    connect(ui->btnRemove, &QToolButton::clicked, this, &DocAppointmentsPatients::removeData_Doc);
    connect(ui->btnOrderEcho, &QToolButton::clicked, this, &DocAppointmentsPatients::onClickOrderEcho);
    connect(ui->btnClose, &QToolButton::clicked, this, &DocAppointmentsPatients::onClose);
    connect(ui->btnBackDay, &QToolButton::clicked, this, &DocAppointmentsPatients::onClickBackDay);
    connect(ui->btnNextDay, &QToolButton::clicked, this, &DocAppointmentsPatients::onClickNextDay);
    connect(ui->docDate, &QDateEdit::dateChanged, this, &DocAppointmentsPatients::update_model);
    connect(model_main, &RegistrationTableModel::m_data_changed, this, &DocAppointmentsPatients::dataWasModified);

    connect(delegate_organizations, QOverload<QWidget*>::of(&ComboDelegate::commitData),
            this, QOverload<QWidget*>::of(&DocAppointmentsPatients::GetChangedValueComboDelegate));
    connect(delegate_doctors, QOverload<QWidget*>::of(&ComboDelegate::commitData),
            this, QOverload<QWidget*>::of(&DocAppointmentsPatients::GetChangedValueComboDelegate));
}

DocAppointmentsPatients::~DocAppointmentsPatients()
{
    delete delegate_organizations;
    delete delegate_doctors;
    delete delegate_execute;
    delete model_main;
    delete db;
    delete popUp;
    delete ui;
}

void DocAppointmentsPatients::dataWasModified()
{
    setWindowModified(true);
}

void DocAppointmentsPatients::onClickBackDay()
{
    QDate m_current_day(ui->docDate->date());
    ui->docDate->setDate(m_current_day.addDays(-1));
}

void DocAppointmentsPatients::onClickNextDay()
{
    QDate m_current_day(ui->docDate->date());
    ui->docDate->setDate(m_current_day.addDays(1));
}

void DocAppointmentsPatients::update_model()
{
    delete model_main;
    model_main = new RegistrationPatientsModel(m_rows, m_column, this);

    QSqlQuery qry;
    QString str = QString("SELECT id,"
                          "  time_registration, "
                          "  execute, "
                          "  dataPatient, "
                          "  name_organizations, "
                          "  id_organizations, "
                          "  name_doctors, "
                          "  id_doctors,"
                          "  investigations, "
                          "  comment "
                          "FROM registrationPatients WHERE dateDoc = '%1';").arg(ui->docDate->date().toString("yyyy-MM-dd"));

    if (qry.exec(str)){
        while (qry.next()){
            model_main->setDataPatient(qry.value(1).toInt(), section_id, qry.value(0).toString());
            model_main->setDataPatient(qry.value(1).toInt(), section_data_patient, qry.value(3).toString());
            model_main->setDataPatient(qry.value(1).toInt(), section_investigation, qry.value(8).toString());
            model_main->setDataPatient(qry.value(1).toInt(), section_organization, qry.value(4).toString());
            model_main->setDataPatient(qry.value(1).toInt(), section_doctor, qry.value(6).toString());
            model_main->setDataPatient(qry.value(1).toInt(), section_comment, qry.value(9).toString());
            model_main->setDataPatient(qry.value(1).toInt(), section_execute, qry.value(2).toString());
        }
    }

    ui->tableView->setModel(model_main);
    ui->tableView->hideColumn(section_id - 1);
    ui->tableView->setColumnWidth(section_id - 1, 50);
    ui->tableView->setColumnWidth(section_execute - 1, 70);
    ui->tableView->setColumnWidth(section_data_patient - 1, 250);
    ui->tableView->setColumnWidth(section_investigation - 1, 300);
    ui->tableView->setColumnWidth(section_organization - 1, 200);
    ui->tableView->setColumnWidth(section_doctor - 1, 200);
    ui->tableView->setColumnWidth(section_comment - 1, 400);
    ui->tableView->verticalHeader()->setStretchLastSection(true);
    ui->tableView->verticalHeader()->setDefaultSectionSize(30);
}

bool DocAppointmentsPatients::removeData_Doc()
{
    bool action_remove = true;

    int rows = ui->tableView->model()->rowCount();
    for (int n = 0; n < rows; n++){

        QString _str_id = ui->tableView->model()->data(ui->tableView->model()->index(n, section_id - 1)).toString();
        QString _data_patient  = ui->tableView->model()->data(ui->tableView->model()->index(n, section_data_patient - 1)).toString();

        if (_str_id.isEmpty())
            continue;

        int _id = _str_id.toInt();

        QString str_qry = QString("DELETE FROM registrationPatients WHERE id = '%1';").arg(_id);
        if (! db->execQuery(str_qry)){
            QMessageBox::warning(this, tr("Eliminarea programării"),
                                 tr("Datele pacientului <b>'%1'</b> nu au fost eliminate în baza de date.")
                                 .arg(_data_patient), QMessageBox::Ok);
            action_remove = false;
            return action_remove;
        } else {
            qInfo(logInfo()) << tr("Programarea pacientului '%1' din data '%2' eliminata cu succes din baza de date.")
                                .arg(_data_patient, ui->docDate->date().toString("dd.MM.yyyy"));
        }
    }

    if (action_remove){
        popUp->setPopupText(tr("Datele programării din data '%1'<br> au fost eliminate cu succes în baza de date.").arg(ui->docDate->date().toString("dd.MM.yyyy")));
        popUp->show();
        update_model();
    }
    return true;
}

void DocAppointmentsPatients::writeRegistration()
{
    int rowCount = ui->tableView->model()->rowCount();
    for (int row = 0; row < rowCount; row++){

        QString row_id;
        QString str_execute;
        QString str_data_patient;
        QString str_investigation;
        QString str_organization;
        QString str_doctor;
        QString str_comment;

        row_id            = ui->tableView->model()->data(ui->tableView->model()->index(row, section_id - 1)).toString();
        str_execute       = ui->tableView->model()->data(ui->tableView->model()->index(row, section_execute - 1)).toString();
        str_data_patient  = ui->tableView->model()->data(ui->tableView->model()->index(row, section_data_patient - 1)).toString();
        str_investigation = ui->tableView->model()->data(ui->tableView->model()->index(row, section_investigation - 1)).toString();
        str_organization  = ui->tableView->model()->data(ui->tableView->model()->index(row, section_organization - 1)).toString();
        str_doctor        = ui->tableView->model()->data(ui->tableView->model()->index(row, section_doctor - 1)).toString();
        str_comment       = ui->tableView->model()->data(ui->tableView->model()->index(row, section_comment - 1)).toString();

        if (str_data_patient.isEmpty())
            continue;

        int id_organization = 0;
        QMapIterator<int, QString> it(items_organizations);
        while (it.findNext(str_organization)) {
            id_organization = it.key();
        }

        int id_doctor = 0;
        QMapIterator<int, QString> it_doc(items_doctors);
        while (it_doc.findNext(str_doctor)) {
            id_doctor = it_doc.key();
        }

        if (row_id.isEmpty()){ // inserarea datelor

            QString str_qry = "INSERT INTO registrationPatients ("
                              "  id,"
                              "  dateDoc,"
                              "  time_registration,"
                              "  execute,"
                              "  dataPatient,"
                              "  name_organizations,"
                              "  id_organizations,"
                              "  name_doctors,"
                              "  id_doctors,"
                              "  investigations,"
                              "  comment) "
                              "VALUES (:id, :dateDoc, :time_registration, :execute, :dataPatient, :name_organizations, :id_organizations, :name_doctors, :id_doctors, :investigations, :comment);";
            QSqlQuery qry;
            qry.prepare(str_qry);
            qry.bindValue(":id",                 QString::number(db->getLastIdForTable("registrationPatients") + 1));
            qry.bindValue(":dateDoc",            ui->docDate->date().toString("yyyy-MM-dd"));
            qry.bindValue(":time_registration",  QString::number(row + 1));
            qry.bindValue(":execute",            str_execute);
            qry.bindValue(":dataPatient",        str_data_patient);
            qry.bindValue(":name_organizations", str_organization);
            qry.bindValue(":id_organizations",   QString::number(id_organization));
            qry.bindValue(":name_doctors",       str_doctor);
            qry.bindValue(":id_doctors",         QString::number(id_doctor));
            qry.bindValue(":investigations",     str_investigation);
            qry.bindValue(":comment",            str_comment);
            if (qry.exec()){
                QString txt_msg = tr("Datele programării din data '%1'<br> au fost salvate cu succes în baza de date.")
                        .arg(ui->docDate->date().toString("dd.MM.yyyy"));
                qInfo(logInfo()) << txt_msg;
                popUp->setPopupText(txt_msg);
                popUp->show();
                setWindowModified(false);
            } else {
                QString txt_msg = tr("Eroarea la validarea documentului de programare");
                qWarning(logWarning()) << txt_msg + ": " + qry.lastError().text();
                QMessageBox::warning(this, tr("Validarea programării"), txt_msg + tr(". Adresați-vă administratorului aplicației."), QMessageBox::Ok);
                return;
            }

        } else { // actualizarea datelor

            QString str_qry = "UPDATE registrationPatients SET "
                              "  dateDoc = :dateDoc,"
                              "  time_registration = :time_registration,"
                              "  execute = :execute,"
                              "  dataPatient = :dataPatient,"
                              "  name_organizations = :name_organizations,"
                              "  id_organizations = :id_organizations,"
                              "  name_doctors = :name_doctors,"
                              "  id_doctors = :id_doctors,"
                              "  investigations = :investigations,"
                              "  comment = :comment "
                              "WHERE id = :id;";
            QSqlQuery qry;
            qry.prepare(str_qry);
            qry.bindValue(":id",                 row_id);
            qry.bindValue(":dateDoc",            ui->docDate->date().toString("yyyy-MM-dd"));
            qry.bindValue(":time_registration",  QString::number(row + 1));
            qry.bindValue(":execute",            str_execute);
            qry.bindValue(":dataPatient",        str_data_patient);
            qry.bindValue(":name_organizations", str_organization);
            qry.bindValue(":id_organizations",   QString::number(id_organization));
            qry.bindValue(":name_doctors",       str_doctor);
            qry.bindValue(":id_doctors",         QString::number(id_doctor));
            qry.bindValue(":investigations",     str_investigation);
            qry.bindValue(":comment",            str_comment);
            if (qry.exec()){
                QString txt_msg = tr("Datele programării din data '%1'<br> au fost actualizate cu succes în baza de date.")
                        .arg(ui->docDate->date().toString("dd.MM.yyyy"));
                qInfo(logInfo()) << txt_msg;
                popUp->setPopupText(txt_msg);
                popUp->show();
                setWindowModified(false);
            } else {
                QString txt_msg = tr("Eroarea la revalidarea documentului de programare");
                qWarning(logWarning()) << txt_msg + ": " + qry.lastError().text();
                QMessageBox::warning(this, tr("Revalidarea programării"), txt_msg + tr(". Adresați-vă administratorului aplicației."), QMessageBox::Ok);
                return;
            }
        }
    }
    update_model();
}

void DocAppointmentsPatients::onClickPrint()
{
    QMessageBox::warning(this, tr("Printarea programării"),
                         tr("Forma de tipar se află în procesul dezvoltării.<br>"
                            "Urmăriți actualizările aplicației.<br><br>"
                            "Pentru informație suplimentară contactați administratorul aplicației."),
                         QMessageBox::Ok);
}

void DocAppointmentsPatients::onClickOrderEcho()
{
    auto row  = ui->tableView->currentIndex().row();
    if (row == -1){
        QMessageBox::warning(this, tr("Marcarea rândului"), tr("Nu este marcat rândul"), QMessageBox::Ok);
        return;
    }

    QString name_organization = model_main->data(model_main->index(row, section_organization - 1), Qt::DisplayRole).toString();
    QString name_doctor       = model_main->data(model_main->index(row, section_doctor - 1), Qt::DisplayRole).toString();
    QString name_patient      = model_main->data(model_main->index(row, section_data_patient - 1), Qt::DisplayRole).toString();

    DocOrderEcho* doc_order = new DocOrderEcho(this);
    doc_order->setAttribute(Qt::WA_DeleteOnClose);
    doc_order->setProperty("ItNew", true);
    doc_order->setProperty("IdOrganization", getIdOrganizationByName(name_organization));
    doc_order->setProperty("IdDoctor", getIdDoctorByFullName(name_doctor));
    doc_order->setProperty("NamePatient", name_patient);
    doc_order->setGeometry(250, 150, 1400, 800);
    doc_order->show();
}

void DocAppointmentsPatients::onClose()
{
    this->close();
    emit mCloseThisForm();
}

void DocAppointmentsPatients::GetChangedValueComboDelegate(QWidget *combo)
{
    QComboBox *cb = qobject_cast<QComboBox *>(combo);

    if (ui->tableView->currentIndex().column() == 4)
        items_organizations.insert(cb->currentData().toInt(), cb->currentText());
    else if (ui->tableView->currentIndex().column() == 5)
        items_doctors.insert(cb->currentData().toInt(), cb->currentText());

//    qDebug(logDebug()) << tr("Documentul - 'Programarea pacienților' - combodelegate = '%1'(id = %2).").arg(cb->currentText(), cb->currentData().toString());
    dataWasModified();
}

void DocAppointmentsPatients::getChangedDataDelegateComplet(const int id, const QString str_data)
{
    qDebug(logDebug()) << id << str_data;
}

void DocAppointmentsPatients::setStyleSheetButtoms()
{
    ui->btnWrite->setStyleSheet("padding-left: 4px;"
                                "border: 1px solid #8f8f91; "
                                "border-radius: 4px;");
    ui->btnRemove->setStyleSheet("padding-left: 4px;"
                                 "border: 1px solid #8f8f91; "
                                 "border-radius: 4px;");
    ui->btnPrint->setStyleSheet("padding-left: 4px;"
                                "border: 1px solid #8f8f91; "
                                "border-radius: 4px;");
    ui->btnOrderEcho->setStyleSheet("padding-left: 4px;"
                                    "border: 1px solid #8f8f91; "
                                    "border-radius: 4px;");
    ui->btnClose->setStyleSheet("padding-left: 4px;"
                                "border: 1px solid #8f8f91; "
                                "border-radius: 4px;");
    ui->btnBackDay->setStyleSheet("border: 1px solid #8f8f91; "
                                          "border-radius: 4px;");
    ui->btnNextDay->setStyleSheet("border: 1px solid #8f8f91; "
                                          "border-radius: 4px;");

#if defined(Q_OS_WIN)
    ui->frame->setStyle(style_fusion);
#endif

}

int DocAppointmentsPatients::getIdOrganizationByName(const QString _name) const
{
    int id_organization = -1;

    QSqlQuery qry;
    qry.prepare("SELECT id FROM organizations WHERE name = '" + _name + "';");
    if (qry.exec() && qry.next())
        id_organization = qry.value(0).toInt();

    return id_organization;
}

int DocAppointmentsPatients::getIdDoctorByFullName(const QString _name) const
{
    int id_doctor = -1;

    QSqlQuery qry;
    QString str_qry;
    if (globals::thisMySQL)
        str_qry = QString("SELECT doctors.id FROM doctors "
                          "WHERE CONCAT(doctors.name, ' ' , SUBSTRING(doctors.fName, 1, 1) ,'.') = '%1' AND deletionMark = 0 ORDER BY doctors.name;").arg(_name);
    else
        str_qry = QString("SELECT doctors.id FROM doctors "
                          "WHERE doctors.name ||' '|| substr(doctors.fName, 1, 1) ||'.' = '%1' AND deletionMark = 0 ORDER BY doctors.name;").arg(_name);

    if (qry.exec(str_qry)){
        while (qry.next()){
            id_doctor = qry.value(0).toInt();
        }
    }
    return id_doctor;
}

void DocAppointmentsPatients::closeEvent(QCloseEvent *event)
{
    if (isWindowModified()){
        const QMessageBox::StandardButton answer = QMessageBox::warning(this, tr("Modificarea datelor"),
                                                                        tr("Datele au fost modificate.\n"
                                                                           "Doriți să salvați aceste modificări ?"),
                                                                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (answer == QMessageBox::Yes){
            writeRegistration();
            event->accept();
        } else if (answer == QMessageBox::Cancel){
            event->ignore();
        }
    } else {
        event->accept();
    }
}

bool DocAppointmentsPatients::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->btnWrite){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnWrite->pos().x() - 14, ui->btnWrite->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Salvează datele<br>programării."));         // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == ui->btnPrint){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnPrint->pos().x() - 18, ui->btnPrint->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Generarea formei<br>de tipar."));         // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == ui->btnRemove){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnRemove->pos().x() - 28, ui->btnRemove->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Eliminarea datelor<br>programării."));         // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == ui->btnOrderEcho){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnOrderEcho->pos().x() - 44, ui->btnOrderEcho->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Formează documentul<br><b>'Comanda ecografică'</b><br>pe baza datelor programării."));         // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == ui->btnClose){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnClose->pos().x() - 14, ui->btnClose->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Închide forma<br>programării."));         // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    }
    return false;
}
