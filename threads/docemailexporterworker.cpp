#include "docemailexporterworker.h"

DocEmailExporterWorker::DocEmailExporterWorker(DatabaseProvider *provider, DocData &data, QObject *parent)
    : QObject{parent}, m_data(data), m_db(provider)
{}

// **********************************************************************************
// --- functia principala de export

void DocEmailExporterWorker::process()
{
    const QString connName = QStringLiteral("connection_%1")
                                    .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    { // Conexiunea trăieşte DOAR în acest bloc

        // 1. efectuam conectarea la bd int-un alt thread
        QSqlDatabase dbConn = m_db->getDatabaseThread(connName, m_data.thisMySQL);
        if (! dbConn.isOpen() &&
            ! dbConn.open()) {
            qCritical() << QStringLiteral("[THREAD %1] Nu pot deschide conexiunea DB:")
                            .arg(this->metaObject()->className())
                        << dbConn.lastError().text();
            emit finished(datesExportForAgentEmail);
            return;
        }

        // 2. exportam ordercho
        exportOrderEcho(dbConn);

        // 3. exportam reportEcho
        exportReportEcho(dbConn);

        // 4. exportam imaginile
        exportImagesDocument(dbConn);

        // 5. inchidem conexiunea cu BD
        dbConn.close();

    } // <- destructor QSqlDatabase

    m_db->removeDatabaseThread(connName);

    emit finished(datesExportForAgentEmail);
}

// **********************************************************************************
// --- setarea modelelor documentului 'orderEcho'

void DocEmailExporterWorker::setModelImgForPrint()
{
    // 1. logotip
    QPixmap pix_logo = QPixmap();
    QStandardItem *img_item_logo = new QStandardItem();
    if (! m_data.logo_byteArray.isEmpty() && pix_logo.loadFromData(m_data.logo_byteArray)) {
        img_item_logo->setData(pix_logo.scaled(300,50, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage(),
                               Qt::DisplayRole);
        exist_logo = 1;
    }

    // 2. stampila organizatiei
    QPixmap pix_stamp_organization = QPixmap();
    QStandardItem* img_item_stamp_organization = new QStandardItem();
    if (! m_data.stamp_organization_byteArray.isEmpty() && pix_stamp_organization.loadFromData(m_data.stamp_organization_byteArray)) {
        img_item_stamp_organization->setData(pix_stamp_organization.scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage(),
                                             Qt::DisplayRole);
        exist_stamp_organization = 1;
    }

    // 3. stampila doctorului
    QPixmap pix_stamp_doctor = QPixmap();
    QStandardItem* img_item_stamp_doctor = new QStandardItem();
    if (! m_data.stamp_doctor_byteArray.isEmpty() && pix_stamp_doctor.loadFromData(m_data.stamp_doctor_byteArray)) {
        img_item_stamp_doctor->setData(pix_stamp_doctor.scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage(),
                                       Qt::DisplayRole);
        exist_stamp_doctor = 1;
    }

    // 4. semnatura doctorului
    QPixmap pix_signature = QPixmap();
    QStandardItem* img_item_signature = new QStandardItem();
    if (! m_data.signature_doctor_byteArray.isEmpty() && pix_signature.loadFromData(m_data.signature_doctor_byteArray)) {
        img_item_signature->setData(pix_signature.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage(),
                                    Qt::DisplayRole);
        exist_signature = 1;
    }

    //------------------------------------------------------------------------------------------------------
    // setam imaginile in model
    QList<QStandardItem *> items_img;
    items_img.append(img_item_logo);
    items_img.append(img_item_stamp_organization);
    items_img.append(img_item_stamp_doctor);
    items_img.append(img_item_signature);

    model_img = new QStandardItemModel(this);
    model_img->setColumnCount(4);
    model_img->appendRow(items_img);
}

void DocEmailExporterWorker::setModelDatesOrganization(QSqlDatabase &dbConn)
{
    QSqlQuery qry(dbConn);
    qry.prepare(R"(
            SELECT
                constants.id_organizations,
                organizations.IDNP,
                organizations.name,
                organizations.address,
                organizations.telephone,
                fullNameDoctors.nameAbbreviated AS doctor,
                organizations.email
            FROM
                constants
            INNER JOIN
                organizations ON organizations.id = constants.id_organizations
            INNER JOIN
                fullNameDoctors ON fullNameDoctors.id_doctors = constants.id_doctors
            WHERE
                constants.id_users = ?
        )");
    qry.addBindValue(m_data.id_user);
    if (! qry.exec()) {
        qCritical() << QStringLiteral("[THREAD %1] Eroare solicitarii la crearea 'model_organization':")
                            .arg(this->metaObject()->className())
                    << qry.lastError().text();
        return;
    } else {
        model_organization = new QSqlQueryModel(this);
        model_organization->setQuery(std::move(qry));
    }
}

void DocEmailExporterWorker::setModelDatesPatient(QSqlDatabase &dbConn)
{
    QSqlQuery qry(dbConn);
    if (m_data.thisMySQL)
        qry.prepare(R"(
                SELECT
                    CONCAT(pacients.name,' ', pacients.fName) AS FullName,
                    DATE_FORMAT(pacients.birthday, '%d.%m.%Y') AS birthday,
                    pacients.IDNP,
                    pacients.medicalPolicy,
                    pacients.address
                FROM
                    pacients
                WHERE
                    id = ?
            )");
    else
        qry.prepare(R"(
                SELECT
                    pacients.name ||' '|| pacients.fName AS FullName,
                    strftime('%d.%m.%Y', pacients.birthday) AS birthday,
                    pacients.IDNP,
                    pacients.medicalPolicy,
                    pacients.address
                FROM
                    pacients
                WHERE
                    id = ?
            )");
    qry.addBindValue(m_data.id_patient);
    if (! qry.exec()) {
        qCritical() << QStringLiteral("[THREAD %1] Eroare solicitarii la crearea 'model_patient':")
                            .arg(this->metaObject()->className())
                    << qry.lastError().text();
        return;
    } else {
        model_patient = new QSqlQueryModel(this);
        model_patient->setQuery(std::move(qry));
    }
}

void DocEmailExporterWorker::setModelDocTable(QSqlDatabase &dbConn, bool noncomercial)
{
    QSqlQuery qry(dbConn);
    QString str_price;
    if (m_data.thisMySQL)
        str_price = (noncomercial) ? "'0-00'" : "orderEchoTable.price";
    else
        str_price = (noncomercial) ? "'0-00'" : "orderEchoTable.price ||'0'";

    db = new DataBase(this);
    qry.prepare(db->getQryForTableOrderById(m_data.id_order, (noncomercial) ? "'0-00'" : str_price));
    if (! qry.exec()) {
        qCritical() << QStringLiteral("[THREAD %1] Eroare solicitarii la crearea 'model_table':")
                            .arg(this->metaObject()->className())
                    << qry.lastError().text();
        return;
    } else {
        model_table = new QSqlQueryModel(this);
        model_table->setQuery(std::move(qry));
    }
}

// **********************************************************************************
// --- eliberarea memoriei modelelor 'orderEcho' si exportul documentului

void DocEmailExporterWorker::cleaningUpModelInstances()
{
    if (model_organization) {
        model_organization->deleteLater();
        model_organization = nullptr; // pu siguranta si refolosirea
    }
    if (model_patient) {
        model_patient->deleteLater();
        model_patient = nullptr; // pu siguranta si refolosirea
    }
    if (model_table) {
        model_table->deleteLater();
        model_table = nullptr; // pu siguranta si refolosirea
    }
    if (model_img) {
        model_img->deleteLater();
        model_img = nullptr; // pu siguranta si refolosirea
    }
}

void DocEmailExporterWorker::exportOrderEcho(QSqlDatabase &dbConn)
{
    // 1. difinim variabile locale
    bool noncomercial_price = false;
    int sum_order = 0;

    // 2. setam variabile locale si structura
    QSqlQuery qry(dbConn);
    qry.prepare(R"(
            SELECT
                orderEcho.id_typesPrices,
                orderEcho.id_pacients,
                orderEcho.sum,
                orderEcho.numberDoc  AS nr_order,
                orderEcho.dateDoc    AS dateInvestigation,
                reportEcho.id        AS id_report,
                reportEcho.numberDoc AS nr_report,
                typesPrices.noncomercial,
                fullNamePacients.name AS name_patient,
                fullNameDoctors.nameAbbreviated AS doctore_execute,
                pacients.email AS emailTo
            FROM
                orderEcho
            INNER JOIN
                typesPrices ON typesPrices.id = orderEcho.id_typesPrices
            INNER JOIN
                reportEcho ON reportEcho.id_orderEcho = orderEcho.id
            INNER JOIN
                fullNamePacients ON fullNamePacients.id_pacients = orderEcho.id_pacients
            INNER JOIN
                fullNameDoctors ON fullNameDoctors.id_doctors = orderEcho.id_doctors_execute
            INNER JOIN
                pacients ON pacients.id = orderEcho.id_pacients
            WHERE
                orderEcho.id = ?
        )");
    qry.addBindValue(m_data.id_order);
    if (! qry.exec()) {
        qCritical(logCritical()) << QStringLiteral("[THREAD %1] Eroare exec SELECT(orderEcho):")
                                        .arg(this->metaObject()->className())
                                 << qry.lastError().text();
    } else {
        if (qry.next()) {
            QSqlRecord rec = qry.record();
            noncomercial_price = qry.value(rec.indexOf("noncomercial")).toBool();
            m_data.id_patient  = qry.value(rec.indexOf("id_pacients")).toInt();
            sum_order          = qry.value(rec.indexOf("sum")).toInt();
            m_data.id_report   = qry.value(rec.indexOf("id_report")).toInt();
            m_data.nr_order    = qry.value(rec.indexOf("nr_order")).toString();
            m_data.nr_report   = qry.value(rec.indexOf("nr_report")).toString();
            qInfo(logInfo()) << "[THREAD] Se initializeaza exportul documentului 'Comanda ecografica' nr." << m_data.nr_order;

            // setam datele pentru export
            m_datesExport.nr_order              = m_data.nr_order;
            m_datesExport.nr_report             = m_data.nr_report;
            m_datesExport.emailTo               = qry.value(rec.indexOf("emailTo")).toString();
            m_datesExport.name_patient          = qry.value(rec.indexOf("name_patient")).toString();
            m_datesExport.name_doctor_execute   = qry.value(rec.indexOf("doctore_execute")).toString();
            m_datesExport.str_dateInvestigation = qry.value(rec.indexOf("dateInvestigation")).toString();
        }
    }

    // 3. introducem date pu export in vector
    datesExportForAgentEmail.append(m_datesExport);

    // 4. setam modele pu transmiterea generatorului de rapoarte
    setModelImgForPrint();
    setModelDatesOrganization(dbConn);
    setModelDatesPatient(dbConn);
    setModelDocTable(dbConn, noncomercial_price);

    // 5. alocam memoria pu generatorul de rapoarte
    m_report = new LimeReport::ReportEngine(this);

    // 6. ne clarificam cu suma
    double _num = 0;
    _num = (noncomercial_price) ? 0 : sum_order; // daca 'noncomercial_price' -> nu aratam suma

    // 7. transmitem variabile si modele generatorului de rapoarte
    m_report->dataManager()->clearUserVariables();
    m_report->dataManager()->setReportVariable("sume_total", QString("%1").arg(_num, 0, 'f', 2));
    m_report->dataManager()->setReportVariable("v_exist_logo", exist_logo);
    m_report->dataManager()->setReportVariable("v_exist_stamp", exist_stamp_organization);
    m_report->dataManager()->setReportVariable("v_exist_stamp_doctor", exist_stamp_doctor);
    m_report->dataManager()->setReportVariable("v_exist_signature", exist_signature);

    m_report->dataManager()->addModel("table_img", model_img, true);
    m_report->dataManager()->addModel("main_organization", model_organization, false);
    m_report->dataManager()->addModel("table_pacient", model_patient, false);
    m_report->dataManager()->addModel("table_table", model_table, false);
    m_report->setShowProgressDialog(true);

    // 8. verificam daca este fisierul de tipar
    QDir dir;
    if (! QFile(dir.toNativeSeparators(m_data.pathTemplatesDocs + "/Order.lrxml")).exists()){
        cleaningUpModelInstances();
        m_report->deleteLater();
        qCritical() << "[THREAD %1] Nu este determinat modelul formai de tipar 'Order.lrxml' !!!"
                    << qry.lastError().text();
        return;
    }

    // 9. exportam fisierul
    m_report->loadFromFile(dir.toNativeSeparators(m_data.pathTemplatesDocs + "/Order.lrxml"));
    if (m_report->printToPDF(m_data.filePDF + "/Comanda_ecografica_nr_" + m_data.nr_order + ".pdf")) {
        qInfo(logInfo()) << "[THREAD] Exportul cu succes a documentului 'Comanda ecografica' nr."
                         << m_data.nr_order;
        emit setTextInfo("S-a descarcat documentul 'Comanda ecografica' nr." + m_data.nr_order);
    } else {
        qCritical(logCritical()) << "[THREAD] Eroare la exportul documentului 'Comanda ecografica' nr."
                                 << m_data.nr_order;
    }

    // 10. eliberam memoria medelelor si a generatorului de rapoarte
    cleaningUpModelInstances();
    m_report->deleteLater();
    m_report = nullptr;
}

// **********************************************************************************
// --- procesarea modelelor documentului 'reportEcho'

void DocEmailExporterWorker::handlerComplex(QSqlDatabase &dbConn)
{
    // organs internal
    QSqlQuery qry_organsInternal(dbConn);
    qry_organsInternal.prepare(db->getQryForTableOrgansInternalById(m_data.id_report));
    if (qry_organsInternal.exec()) {
        modelOrgansInternal = new QSqlQueryModel(this);
        modelOrgansInternal->setQuery(std::move(qry_organsInternal));
    }

    // urinary system
    QSqlQuery qry_urinarySystem(dbConn);
    qry_urinarySystem.prepare(db->getQryForTableUrinarySystemById(m_data.id_report));
    if (qry_urinarySystem.exec()){
        modelUrinarySystem = new QSqlQueryModel(this);
        modelUrinarySystem->setQuery(std::move(qry_urinarySystem));
    }
}

void DocEmailExporterWorker::handlerOrgansInternal(QSqlDatabase &dbConn)
{
    QSqlQuery qry(dbConn);
    qry.prepare(db->getQryForTableOrgansInternalById(m_data.id_report));
    if (qry.exec()) {
        modelOrgansInternal = new QSqlQueryModel(this);
        modelOrgansInternal->setQuery(std::move(qry));
    }
}

void DocEmailExporterWorker::handlerUrinarySystem(QSqlDatabase &dbConn)
{
    QSqlQuery qry(dbConn);
    qry.prepare(db->getQryForTableUrinarySystemById(m_data.id_report));
    if (qry.exec()){
        modelUrinarySystem = new QSqlQueryModel(this);
        modelUrinarySystem->setQuery(std::move(qry));
    }
}

void DocEmailExporterWorker::handlerProstate(QSqlDatabase &dbConn)
{
    QSqlQuery qry(dbConn);
    qry.prepare(db->getQryForTableProstateById(m_data.id_report));
    if (qry.exec()){
        modelProstate = new QSqlQueryModel(this);
        modelProstate->setQuery(std::move(qry));
    }
}

void DocEmailExporterWorker::handlerGynecology(QSqlDatabase &dbConn)
{
    QSqlQuery qry(dbConn);
    qry.prepare(db->getQryForTableGynecologyById(m_data.id_report));
    if (qry.exec()){
        modelGynecology = new QSqlQueryModel(this);
        modelGynecology->setQuery(std::move(qry));
    }
}

void DocEmailExporterWorker::handlerBreast(QSqlDatabase &dbConn)
{
    QSqlQuery qry(dbConn);
    qry.prepare(db->getQryForTableBreastById(m_data.id_report));
    if (qry.exec()){
        modelBreast = new QSqlQueryModel(this);
        modelBreast->setQuery(std::move(qry));
    }
}

void DocEmailExporterWorker::handlerThyroid(QSqlDatabase &dbConn)
{
    QSqlQuery qry(dbConn);
    qry.prepare(db->getQryForTableThyroidById(m_data.id_report));
    if (qry.exec()){
        modelThyroid = new QSqlQueryModel(this);
        modelThyroid->setQuery(std::move(qry));
    }
}

void DocEmailExporterWorker::handlerGestation0(QSqlDatabase &dbConn)
{
    QSqlQuery qry(dbConn);
    qry.prepare(db->getQryForTableGestation0dById(m_data.id_report));
    if (qry.exec()){
        modelGestationO = new QSqlQueryModel(this);
        modelGestationO->setQuery(std::move(qry));
    }
}

void DocEmailExporterWorker::handlerGestation1(QSqlDatabase &dbConn)
{
    QSqlQuery qry(dbConn);
    qry.prepare(db->getQryForTableGestation1dById(m_data.id_report));
    if (qry.exec()){
        modelGestation1 = new QSqlQueryModel(this);
        modelGestation1->setQuery(std::move(qry));
    }
}

void DocEmailExporterWorker::handlerGestation2(QSqlDatabase &dbConn)
{
    QSqlQuery qry(dbConn);
    qry.prepare(db->getQryForTableGestation2(m_data.id_report));
    if (qry.exec()){
        modelGestation2 = new QSqlQueryModel(this);
        modelGestation2->setQuery(std::move(qry));
    }
}

// **********************************************************************************
// --- eliberarea memoriei modelelor 'reportEcho' si exportul documentului

void DocEmailExporterWorker::cleaningUpModelInstancesReport()
{
    if (modelOrgansInternal) {
        modelOrgansInternal->deleteLater();
        modelOrgansInternal = nullptr;
    }
    if (modelUrinarySystem) {
        modelUrinarySystem->deleteLater();
        modelUrinarySystem = nullptr;
    }
    if (modelProstate) {
        modelProstate->deleteLater();
        modelProstate = nullptr;
    }
    if (modelGynecology) {
        modelGynecology->deleteLater();
        modelGynecology = nullptr;
    }
    if (modelBreast) {
        modelBreast->deleteLater();
        modelBreast = nullptr;
    }
    if (modelThyroid) {
        modelThyroid->deleteLater();
        modelThyroid = nullptr;
    }
    if (modelGestationO) {
        modelGestationO->deleteLater();
        modelGestationO = nullptr;
    }
    if (modelGestation1) {
        modelGestation1->deleteLater();
        modelGestation1 = nullptr;
    }
    if (modelGestation2) {
        modelGestation2->deleteLater();
        modelGestation2 = nullptr;
    }
}

void DocEmailExporterWorker::exportReportEcho(QSqlDatabase &dbConn)
{
    // 1. definim variabile locale
    bool organs_internal = false;
    bool urinary_system  = false;
    bool prostate        = false;
    bool gynecology      = false;
    bool breast          = false;
    bool thyroide        = false;
    bool gestation0      = false;
    bool gestation1      = false;
    bool gestation2      = false;

    // 2. completam variabile locale
    QSqlQuery qry(dbConn);
    qry.prepare(R"(
                SELECT
                    t_organs_internal,
                    t_urinary_system,
                    t_prostate,
                    t_gynecology,
                    t_breast,
                    t_thyroid,
                    t_gestation0,
                    t_gestation1,
                    t_gestation2,
                    t_gestation3
                FROM
                    reportEcho
                WHERE
                    id_orderEcho = ?
            )");
    qry.addBindValue(m_data.id_order);
    if (! qry.exec()) {
        qCritical(logCritical()) << QStringLiteral("[THREAD %1] Eroare la determinarea tag-lor documentului 'reportEcho':")
                                    .arg(this->metaObject()->className())
                                 << qry.lastError().text();
    } else {
        if (qry.next()) {
            QSqlRecord rec = qry.record();
            organs_internal = qry.value(rec.indexOf("t_organs_internal")).toBool();
            urinary_system  = qry.value(rec.indexOf("t_urinary_system")).toBool();
            prostate        = qry.value(rec.indexOf("t_prostate")).toBool();
            gynecology      = qry.value(rec.indexOf("t_gynecology")).toBool();
            breast          = qry.value(rec.indexOf("t_breast")).toBool();
            thyroide        = qry.value(rec.indexOf("t_thyroid")).toBool();
            gestation0      = qry.value(rec.indexOf("t_gestation0")).toBool();
            gestation1      = qry.value(rec.indexOf("t_gestation1")).toBool();
            gestation2      = qry.value(rec.indexOf("t_gestation2")).toBool();
            qInfo(logInfo()) << "[THREAD] Se initializeaza exportul documentului 'Raport ecografic' nr." << m_data.nr_report;
        }
    }

    m_report = new LimeReport::ReportEngine(this);
    setModelImgForPrint();
    setModelDatesOrganization(dbConn);
    setModelDatesPatient(dbConn);
    m_report->dataManager()->setReportVariable("v_exist_logo", exist_logo);
    m_report->dataManager()->setReportVariable("v_exist_stamp", exist_stamp_organization);
    m_report->dataManager()->setReportVariable("v_exist_stamp_doctor", exist_stamp_doctor);
    m_report->dataManager()->setReportVariable("v_exist_signature", exist_signature);

    m_report->dataManager()->addModel("table_img", model_img, true);
    m_report->dataManager()->addModel("main_organization", model_organization, false);
    m_report->dataManager()->addModel("table_pacient", model_patient, false);

    // complex
    if (organs_internal && urinary_system){
        // pregatim modelul si variabile pentru generatorul de rapoarte
        handlerComplex(dbConn);
        m_report->dataManager()->addModel("table_organs_internal", modelOrgansInternal, false);
        m_report->dataManager()->addModel("table_urinary_system", modelUrinarySystem, false);
        m_report->dataManager()->setReportVariable("unit_measure_volum", "ml");
        // verificam existenta sablonului si posibilitatea incarcarii
        if (! m_report->loadFromFile(m_data.pathTemplatesDocs + "/Complex.lrxml")){
            cleaningUpModelInstancesReport();
            m_report->deleteLater();
            qCritical() << "[THREAD %1] Nu este incarcat sablonul formei de tipar 'Complex.lrxml' !!!"
                        << qry.lastError().text();
            return;
        }
        // export documentului
        m_report->setShowProgressDialog(true);
        if (m_report->printToPDF(m_data.filePDF + "/Raport_ecografic_nr_" + m_data.nr_report + "_complex.pdf")) {
            qInfo(logInfo()) << "[THREAD] Exportul cu succes a documentului 'Raport ecografic' (complex) nr."
                             << m_data.nr_report;
            emit setTextInfo("S-a descarcat documentul 'Raport ecografic' (complex) nr." + m_data.nr_report);
        } else {
            qCritical(logCritical()) << "[THREAD] Eroare la exportul documentului 'Raport ecografic' (complex) nr."
                                     << m_data.nr_report;
        }
    }

    // organs internal
    if (organs_internal && ! urinary_system){
        // pregatim modelul si variabile pentru generatorul de rapoarte
        handlerOrgansInternal(dbConn);
        m_report->dataManager()->addModel("table_organs_internal", modelOrgansInternal, false);
        m_report->dataManager()->setReportVariable("unit_measure_volum", "ml");
        // verificam existenta sablonului si posibilitatea incarcarii
        if (! m_report->loadFromFile(m_data.pathTemplatesDocs + "/Organs internal.lrxml")){
            cleaningUpModelInstancesReport();
            m_report->deleteLater();
            qCritical() << "[THREAD %1] Nu este incarcat sablonul formei de tipar 'Organs internal.lrxml' !!!"
                        << qry.lastError().text();
            return;
        }
        // export documentului
        m_report->setShowProgressDialog(true);
        if (m_report->printToPDF(m_data.filePDF + "/Raport_ecografic_nr_" + m_data.nr_report + "_organe_interne.pdf")) {
            qInfo(logInfo()) << "[THREAD] Exportul cu succes a documentului 'Raport ecografic' (org.interne) nr."
                             << m_data.nr_report;
            emit setTextInfo("S-a descarcat documentul 'Raport ecografic' (org.interne) nr." + m_data.nr_report);
        } else {
            qCritical(logCritical()) << "[THREAD] Eroare la exportul documentului 'Raport ecografic' (org.interne) nr."
                                     << m_data.nr_report;
        }
    }

    // urinary system
    if (urinary_system && ! organs_internal){
        handlerUrinarySystem(dbConn);
        m_report->dataManager()->addModel("table_urinary_system", modelUrinarySystem, false);
        m_report->dataManager()->setReportVariable("unit_measure_volum", "ml");
        if (! m_report->loadFromFile(m_data.pathTemplatesDocs + "/Urinary system.lrxml")){
            cleaningUpModelInstancesReport();
            m_report->deleteLater();
            qCritical() << "[THREAD %1] Nu este incarcat sablonul formei de tipar 'Urinary system.lrxml' !!!"
                        << qry.lastError().text();
            return;
        }

        m_report->setShowProgressDialog(true);
        if (m_report->printToPDF(m_data.pathTemplatesDocs + "/Raport_ecografic_nr_" + m_data.nr_report + "_sistemul_urinar.pdf")) {
            qInfo(logInfo()) << "[THREAD] Exportul cu succes a documentului 'Raport ecografic' (s.urinar) nr."
                             << m_data.nr_report;
            emit setTextInfo("S-a descarcat documentul 'Raport ecografic' - s.urinar ...");
        } else {
            qCritical(logCritical()) << "[THREAD] Eroare la exportul documentului 'Raport ecografic' (s.urinar) nr."
                                     << m_data.nr_report;
        }
    }

    // prostate
    if (prostate){
        handlerProstate(dbConn);
        m_report->dataManager()->addModel("table_prostate", modelProstate, false);
        m_report->dataManager()->setReportVariable("unit_measure_volum", "cm3");
        QSqlQuery qry_supliment(dbConn);
        qry_supliment.prepare("SELECT transrectal FROM tableProstate WHERE id_reportEcho = ?;");
        qry_supliment.addBindValue(m_data.id_report);
        if (qry_supliment.exec() && qry_supliment.next()){
            m_report->dataManager()->setReportVariable("method_examination",
                                                       (qry_supliment.value(0).toInt() == 1)
                                                           ? "transrectal"
                                                           : "transabdominal");
        } else {
            m_report->dataManager()->setReportVariable("method_examination", "transabdominal");
        }

        if (! m_report->loadFromFile(m_data.pathTemplatesDocs + "/Prostate.lrxml")){
            cleaningUpModelInstancesReport();
            m_report->deleteLater();
            qCritical() << "[THREAD %1] Nu este incarcat sablonul formei de tipar 'Prostate.lrxml' !!!"
                        << qry.lastError().text();
            return;
        }

        m_report->setShowProgressDialog(true);
        if (m_report->printToPDF(m_data.filePDF + "/Raport_ecografic_nr_" + m_data.nr_report + "_prostata.pdf")) {
            qInfo(logInfo()) << "[THREAD] Exportul cu succes a documentului 'Raport ecografic' (prostata) nr."
                             << m_data.nr_report;
            emit setTextInfo("S-a descarcat documentul 'Raport ecografic' - prostata ...");
        } else {
            qCritical(logCritical()) << "[THREAD] Eroare la exportul documentului 'Raport ecografic' (prostata) nr."
                                     << m_data.nr_report;
        }
    }

    // gynecology
    if (gynecology){
        handlerGynecology(dbConn);
        m_report->dataManager()->addModel("table_gynecology", modelGynecology, false);
        m_report->dataManager()->setReportVariable("unit_measure_volum", "cm3");
        QSqlQuery qry_ginecSupliment(dbConn);
        qry_ginecSupliment.prepare("SELECT transvaginal FROM tableGynecology WHERE id_reportEcho = ?;");
        qry_ginecSupliment.addBindValue(m_data.id_report);
        if (qry_ginecSupliment.exec() && qry_ginecSupliment.next()){
            m_report->dataManager()->setReportVariable("method_examination",
                                                       (qry_ginecSupliment.value(0).toInt() == 1)
                                                           ? "transvaginal"
                                                           : "transabdominal");
        } else {
            m_report->dataManager()->setReportVariable("method_examination", "transabdominal");
        }

        if (! m_report->loadFromFile(m_data.pathTemplatesDocs + "/Gynecology.lrxml")){
            cleaningUpModelInstancesReport();
            m_report->deleteLater();
            qCritical() << "[THREAD %1] Nu este incarcat sablonul formei de tipar 'Gynecology.lrxml' !!!"
                        << qry.lastError().text();
            return;
        }

        m_report->setShowProgressDialog(true);
        if (m_report->printToPDF(m_data.filePDF + "/Raport_ecografic_nr_" + m_data.nr_report + "_ginecologie.pdf")) {
            qInfo(logInfo()) << "[THREAD] Exportul cu succes a documentului 'Raport ecografic' (ginecologie) nr."
                             << m_data.nr_report;
            emit setTextInfo("S-a descarcat documentul 'Raport ecografic - ginecologia'...");
        } else {
            qCritical(logCritical()) << "[THREAD] Eroare la exportul documentului 'Raport ecografic' (ginecologie) nr."
                                     << m_data.nr_report;
        }
    }

    // breast
    if (breast){
        handlerBreast(dbConn);
        m_report->dataManager()->addModel("table_breast", modelBreast, false);
        if (! m_report->loadFromFile(m_data.pathTemplatesDocs + "/Breast.lrxml")){
            cleaningUpModelInstancesReport();
            m_report->deleteLater();
            qCritical() << "[THREAD %1] Nu este incarcat sablonul formei de tipar 'Breast.lrxml' !!!"
                        << qry.lastError().text();
            return;
        }

        m_report->setShowProgressDialog(true);
        if (m_report->printToPDF(m_data.filePDF + "/Raport_ecografic_nr_" + m_data.nr_report + "_glandele_mamare.pdf")) {
            qInfo(logInfo()) << "[THREAD] Exportul cu succes a documentului 'Raport ecografic' (gl.mamare) nr."
                             << m_data.nr_report;
            emit setTextInfo("S-a descarcat documentul 'Raport ecografic' - gl.mamare ...");
        } else {
            qCritical(logCritical()) << "[THREAD] Eroare la exportul documentului 'Raport ecografic' (gl.mamare) nr."
                                     << m_data.nr_report;
        }
    }

    // thyroid
    if (thyroide){
        handlerThyroid(dbConn);
        m_report->dataManager()->addModel("table_thyroid", modelThyroid, false);
        m_report->dataManager()->setReportVariable("unit_measure_volum", "cm3");
        if (! m_report->loadFromFile(m_data.pathTemplatesDocs + "/Thyroid.lrxml")){
            cleaningUpModelInstancesReport();
            m_report->deleteLater();
            qCritical() << "[THREAD %1] Nu este incarcat sablonul formei de tipar 'Thyroid.lrxml' !!!"
                        << qry.lastError().text();
            return;
        }

        m_report->setShowProgressDialog(true);
        if (m_report->printToPDF(m_data.filePDF + "/Raport_ecografic_nr_" + m_data.nr_report + "_tiroida.pdf")) {
            qInfo(logInfo()) << "[THREAD] Exportul cu succes a documentului 'Raport ecografic' (gl.tiroida) nr."
                             << m_data.nr_report;
            emit setTextInfo("S-a descarcat documentul 'Raport ecografic' - tiroida ...");
        } else {
            qCritical(logCritical()) << "[THREAD] Eroare la exportul documentului 'Raport ecografic' (gl.tiroida) nr."
                                     << m_data.nr_report;
        }
    }

    // gestation0
    if (gestation0){
        handlerGestation0(dbConn);
        m_report->dataManager()->addModel("table_gestation0", modelGestationO, false);
        QSqlQuery qry_supGestation0(dbConn);
        qry_supGestation0.prepare("SELECT view_examination FROM tableGestation0 WHERE id_reportEcho = ?;");
        qry_supGestation0.addBindValue(m_data.id_report);
        if (qry_supGestation0.exec() && qry_supGestation0.next()){
            m_report->dataManager()->setReportVariable("ivestigation_view", (qry_supGestation0.value(0).toInt() == 1));
        } else {
            m_report->dataManager()->setReportVariable("ivestigation_view", 0);
        }

        if (! m_report->loadFromFile(m_data.pathTemplatesDocs + "/Gestation0.lrxml")){
            cleaningUpModelInstancesReport();
            m_report->deleteLater();
            qCritical() << "[THREAD %1] Nu este incarcat sablonul formei de tipar 'Gestation0.lrxml' !!!"
                        << qry.lastError().text();
            return;
        }

        m_report->setShowProgressDialog(true);
        if (m_report->printToPDF(m_data.filePDF + "/Raport_ecografic_nr_" + m_data.nr_report + "_sarcina0.pdf")) {
            qInfo(logInfo()) << "[THREAD] Exportul cu succes a documentului 'Raport ecografic' (sarcina pana la 11 sapt.) nr."
                             << m_data.nr_report;
            emit setTextInfo("S-a descarcat documentul 'Raport ecografic' - sarcina pana la 11 sapt. ...");
        } else {
            qCritical(logCritical()) << "[THREAD] Eroare la exportul documentului 'Raport ecografic' (sarcina pana la 11 sapt.) nr."
                                     << m_data.nr_report;
        }
    }

    // gestation1
    if (gestation1){
        handlerGestation1(dbConn);
        m_report->dataManager()->addModel("table_gestation1", modelGestation1, false);
        QSqlQuery qry_supGestation1(dbConn);
        qry_supGestation1.prepare("SELECT view_examination FROM tableGestation1 WHERE id_reportEcho = ?;");
        qry_supGestation1.addBindValue(m_data.id_report);
        if (qry_supGestation1.exec() && qry_supGestation1.next()){
            m_report->dataManager()->setReportVariable("ivestigation_view", (qry_supGestation1.value(0).toInt() == 1));
        } else {
            m_report->dataManager()->setReportVariable("ivestigation_view", 0);
        }

        if (! m_report->loadFromFile(m_data.pathTemplatesDocs + "/Gestation1.lrxml")){
            cleaningUpModelInstancesReport();
            m_report->deleteLater();
            qCritical() << "[THREAD %1] Nu este incarcat sablonul formei de tipar 'Gestation1.lrxml' !!!"
                        << qry.lastError().text();
            return;
        }

        m_report->setShowProgressDialog(true);
        if (m_report->printToPDF(m_data.filePDF + "/Raport_ecografic_nr_" + m_data.nr_report + "_sarcina1.pdf")) {
            qInfo(logInfo()) << "[THREAD] Exportul cu succes a documentului 'Raport ecografic' (sarcina trim.I) nr."
                             << m_data.nr_report;
            emit setTextInfo("S-a descarcat documentul 'Raport ecografic' - sarcina trim.I ...");
        } else {
            qCritical(logCritical()) << "[THREAD] Eroare la exportul documentului 'Raport ecografic' (sarcina trim.I) nr."
                                     << m_data.nr_report;
        }
    }

    // gestation2
    if (gestation2){
        handlerGestation2(dbConn);
        m_report->dataManager()->addModel("table_gestation2", modelGestation2, false);

        int trim = -1;
        QSqlQuery qry_supGestation2(dbConn);
        qry_supGestation2.prepare("SELECT trimestru FROM tableGestation2 WHERE id_reportEcho = ?;");
        qry_supGestation2.addBindValue(m_data.id_report);
        if (qry_supGestation2.exec() && qry_supGestation2.next()){
            trim = qry_supGestation2.value(0).toInt();
        } else {
            trim = 2;
        }

        QString pathToFileGestation = (trim == 2)
                                          ? m_data.pathTemplatesDocs + "/Gestation2.lrxml"
                                          : m_data.pathTemplatesDocs + "/Gestation3.lrxml";

        if (! m_report->loadFromFile(pathToFileGestation)){
            cleaningUpModelInstancesReport();
            m_report->deleteLater();
            qCritical() << "[THREAD %1] Nu este incarcat sablonul formei de tipar" << pathToFileGestation
                        << qry.lastError().text();
            return;
        }

        QString str_txt = (trim == 2) ? "trim.II" : "trim.III";
        m_report->setShowProgressDialog(true);
        if (m_report->printToPDF(m_data.filePDF + "/Raport_ecografic_nr_" + m_data.nr_report + "_sarcina2.pdf")) {

            qInfo(logInfo()) << "[THREAD] Exportul cu succes a documentului 'Raport ecografic' (" + str_txt + ") nr."
                             << m_data.nr_report;
            emit setTextInfo("S-a descarcat documentul 'Raport ecografic' - sarcina " + str_txt);
        } else {
            qCritical(logCritical()) << "[THREAD] Eroare la exportul documentului 'Raport ecografic' (" + str_txt + ") nr."
                                     << m_data.nr_report;
        }
    }

    cleaningUpModelInstancesReport();
    m_report->deleteLater();

    emit setTextInfo("Exportul documentelor s-a finisat cu succes.");

}

// **********************************************************************************
// --- exportul imaginilor

void DocEmailExporterWorker::exportImagesDocument(QSqlDatabase &dbConn)
{
    emit setTextInfo("Se pregateste exportarea imaginelor ...");

    QSqlQuery qry(dbConn);
    qry.prepare(R"(
        SELECT
            image_1,
            image_2,
            image_3,
            image_4,
            image_5
        FROM
            imagesReports
        WHERE
            id_orderEcho = ? AND
            id_reportEcho = ?
    )");
    qry.addBindValue(m_data.id_order);
    qry.addBindValue(m_data.id_report);
    if (! qry.exec()) {
        qCritical(logCritical()) << QStringLiteral("[THREAD %1] Eroare la selectare din tabela 'imagesReports':")
                                        .arg(this->metaObject()->className())
                                 << qry.lastError().text();
    } else {
        qInfo(logInfo()) << "[THREAD] Se initializeaza exportul imaginelor documentului 'Raport ecografic' nr." << m_data.nr_report;

        if (qry.next()){
            QSqlRecord rec = qry.record();
            QVector<QByteArray> imageByteArrays;
            QVector<QString> imageNames = {"image_1", "image_2", "image_3", "image_4", "image_5"};

            for (const auto& imageName : imageNames) {
                QByteArray imageData = QByteArray::fromBase64(qry.value(rec.indexOf(imageName)).toByteArray());
                imageByteArrays.append(imageData);
            }

            for (int i = 0; i < imageByteArrays.size(); ++i) {
                if (! imageByteArrays[i].isEmpty()) {
                    QPixmap pixmap;
                    if (pixmap.loadFromData(imageByteArrays[i])) {
                        QString filePath = m_data.filePDF + "/" + QString("Image_report_%1_nr_%2.jpg")
                                                                      .arg(m_data.nr_report,
                                                                           QString::number(i + 1));
                        if (!pixmap.save(filePath)) {
                            qCritical(logCritical()) << QStringLiteral("[THREAD %1] Eroare la salvarea imaginei - %2")
                                                        .arg(this->metaObject()->className(), filePath);
                            return;
                        } else {
                            qInfo(logInfo()) << "[THREAD] Exportul cu succes a imaginei -" << filePath;
                            emit setTextInfo("Imagine salvată: " + filePath);
                        }
                    } else {
                        qCritical(logCritical()) << QStringLiteral("[THREAD %1] Eroare la incarcarea imaginei '%2' din QByteArray")
                                                    .arg(this->metaObject()->className());
                        return;
                    }
                }
            }
        }
    }

    emit setTextInfo("Imaginile sunt salvate cu succes ...");
}
