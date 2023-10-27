#include "reports.h"
#include "qabstractitemview.h"
#include "qprogressdialog.h"
#include "ui_reports.h"

Reports::Reports(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Reports)
{
    ui->setupUi(this);

    setWindowTitle(tr("Generatorul de rapoarte"));

    m_report = new LimeReport::ReportEngine(this);
    m_preview = m_report->createPreviewWidget();
    m_preview->setPreviewPageBackgroundColor(QColor(179, 179, 179));

    getNameReportsFromDorectory(); // setam denumirea rapoartelor

    QString strQryOrganizations = "SELECT id,name FROM organizations WHERE deletionMark = 0;";    // solicitarea
    QString strQryContracts     = "SELECT id,name FROM contracts WHERE deletionMark = 0;";
    QString strQryDoctors       = "SELECT doctors.id, "
                                  "fullNameDoctors.nameAbbreviated AS nameFull FROM doctors "
                                  "INNER JOIN fullNameDoctors ON fullNameDoctors.id_doctors = doctors.id "
                                  "WHERE deletionMark = 0 ORDER BY nameFull;";

    db = new DataBase(this);
    modelOrganizations = new BaseSqlQueryModel(strQryOrganizations, ui->comboOrganizations);
    modelContracts     = new BaseSqlQueryModel(strQryContracts, ui->comboContracts);
    modelDoctors       = new BaseSqlQueryModel(strQryDoctors, ui->comboDoctors);

    modelOrganizations->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    modelContracts->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    modelDoctors->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);

    ui->comboOrganizations->setModel(modelOrganizations);
    ui->comboContracts->setModel(modelContracts);
    ui->comboDoctors->setModel(modelDoctors);

    if (modelOrganizations->rowCount() > 20){
        ui->comboOrganizations->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->comboOrganizations->setStyleSheet("combobox-popup: 0;");
        ui->comboOrganizations->setMaxVisibleItems(15);
    }
    if (modelDoctors->rowCount() > 20){
        ui->comboDoctors->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->comboDoctors->setStyleSheet("combobox-popup: 0;");
        ui->comboDoctors->setMaxVisibleItems(15);
    }

    ui->dateStart->setDate(QDate(2022, 01, 01));
    ui->dateEnd->setDate(QDate(2022, 12, 31));

    ui->frame_filter->setVisible(false);
    ui->comboContracts->setEnabled(false);

    ui->frame_preview->layout()->addWidget(m_preview);

    ui->pageNavigator->setPrefix(tr("Pagina: "));

    initConnections();
    initPercentCombobox();
    setStyleForButtom();

    ui->frame_preview->resize(616, ui->frame_preview->height());
    ui->comboTypeReport->resize(260, ui->comboTypeReport->height());
}

Reports::~Reports()
{
    delete modelDoctors;
    delete modelContracts;
    delete modelOrganizations;
    delete m_report;
    delete db;
    delete ui;
}

// **********************************************************************************
// --- initierea datelor raportului

void Reports::loadSettingsReport()
{
    QString str_qry;
    if (m_id == m_id_onLaunch)
        str_qry = "SELECT * FROM settingsReports WHERE showOnLaunch = 1;";
    else
        str_qry = "SELECT * FROM settingsReports WHERE nameReport = :nameReport;";

    QSqlQuery qry;
    qry.prepare(str_qry);
    if (ui->comboTypeReport->currentIndex() != 0)
        qry.bindValue(":nameReport", ui->comboTypeReport->currentText());

    if (qry.exec() && qry.next()){
        m_id = qry.value(0).toInt();

        disconnect(ui->comboTypeReport, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, QOverload<int>::of(&Reports::typeReportsCurrentIndexChanged));
        ui->comboTypeReport->setCurrentText(qry.value(1).toString());
        connect(ui->comboTypeReport, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, QOverload<int>::of(&Reports::typeReportsCurrentIndexChanged));

        if (globals::thisMySQL){
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            ui->dateStart->setDateTime(QDateTime::fromString(qry.value(2).toString().replace(QRegExp("T"), " ").replace(".000",""), "yyyy-MM-dd hh:mm:ss"));
            ui->dateEnd->setDateTime(QDateTime::fromString(qry.value(3).toString().replace(QRegExp("T"), " ").replace(".000",""), "yyyy-MM-dd hh:mm:ss"));
#else
            ui->dateStart->setDateTime(QDateTime::fromString(qry.value(2).toString().replace(QRegularExpression("T"), " ").replace(".000",""), "yyyy-MM-dd hh:mm:ss"));
            ui->dateEnd->setDateTime(QDateTime::fromString(qry.value(3).toString().replace(QRegularExpression("T"), " ").replace(".000",""), "yyyy-MM-dd hh:mm:ss"));
#endif
        } else {
            ui->dateStart->setDateTime(QDateTime::fromString(qry.value(2).toString(), "yyyy-MM-dd hh:mm:ss"));
            ui->dateEnd->setDateTime(QDateTime::fromString(qry.value(3).toString(), "yyyy-MM-dd hh:mm:ss"));
        }

        auto indexOrganization = modelOrganizations->match(modelOrganizations->index(0, 0), Qt::UserRole, qry.value(4).toInt(), 1, Qt::MatchExactly);
        if (!indexOrganization.isEmpty())
            ui->comboOrganizations->setCurrentIndex(indexOrganization.first().row());

        auto indexDoctor = modelDoctors->match(modelDoctors->index(0, 0), Qt::UserRole, qry.value(6).toInt(), 1, Qt::MatchExactly);
        if (!indexDoctor.isEmpty())
            ui->comboDoctors->setCurrentIndex(indexDoctor.first().row());

        ui->showOnLaunch->setCheckState((qry.value(7).toInt() == 1) ? Qt::Checked : Qt::Unchecked);
        ui->hideLogo->setCheckState((qry.value(8).toInt() == 1) ? Qt::Checked : Qt::Unchecked);
        ui->hideDataOrganization->setCheckState((qry.value(10).toInt() == 1) ? Qt::Checked : Qt::Unchecked);
        ui->hideNameDoctor->setCheckState((qry.value(11).toInt() == 1) ? Qt::Checked : Qt::Unchecked);
        ui->hideSignatureStamped->setCheckState((qry.value(12).toInt() == 1) ? Qt::Checked : Qt::Unchecked);
        ui->hidePricesTotal->setCheckState((qry.value(13).toInt() == 1) ? Qt::Checked : Qt::Unchecked);

        generateReport();
    }
}

void Reports::getNameReportsFromDorectory()
{
    ui->comboTypeReport->addItem(tr("<<- selectează raport ->>"));

    QDir dir(globals::pathReports);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QFileInfoList listFiles = dir.entryInfoList();
    for (int n = 0; n < listFiles.size(); n++) {
        QFileInfo fileInfo = listFiles.at(n);
        ui->comboTypeReport->addItem(fileInfo.baseName());
    }
}

void Reports::initConnections()
{
#if defined(Q_OS_LINUX)

    connect(m_report, &LimeReport::ReportEngine::renderStarted, this, &Reports::renderStarted);
    connect(m_report, QOverload<int>::of(&LimeReport::ReportEngine::renderPageFinished), this,
            QOverload<int>::of(&Reports::renderPageFinished));
    connect(m_report, &LimeReport::ReportEngine::renderFinished, this, &Reports::renderFinished);

    connect(ui->comboScalePercent, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&Reports::slotScalePercentChanged));
    connect(ui->btnZoomIn, &QToolButton::clicked, m_preview, &LimeReport::PreviewReportWidget::zoomIn);
    connect(ui->btnZoomOut, &QToolButton::clicked, m_preview, &LimeReport::PreviewReportWidget::zoomOut);

    connect(m_preview, QOverload<int>::of(&LimeReport::PreviewReportWidget::scalePercentChanged),
            this, QOverload<int>::of(&Reports::slotSetTextScalePercentChanged));

    connect(ui->pageNavigator, QOverload<int>::of(&QSpinBox::valueChanged),
            this, QOverload<int>::of(&Reports::slotPageNavigatorChanged));
    connect(ui->btnBackPage, &QToolButton::clicked, m_preview, &LimeReport::PreviewReportWidget::priorPage);
    connect(ui->btnNextPage, &QToolButton::clicked, m_preview, &LimeReport::PreviewReportWidget::nextPage);

    connect(m_preview, QOverload<int>::of(&LimeReport::PreviewReportWidget::pagesSet),
            this, QOverload<int>::of(&Reports::slotPagesSet));

    connect(m_preview, QOverload<int>::of(&LimeReport::PreviewReportWidget::pageChanged),
            this, QOverload<int>::of(&Reports::slotPageChanged));

    connect(ui->btnPrintReport, &QToolButton::clicked, m_preview, &LimeReport::PreviewReportWidget::print);
    connect(ui->btnExportPdf, &QToolButton::clicked, m_preview, &LimeReport::PreviewReportWidget::printToPDF);

#elif defined(Q_OS_WIN)
    // render
    connect(m_report, SIGNAL(renderStarted()), this, SLOT(renderStarted()));
    connect(m_report, SIGNAL(renderPageFinished(int)), this, SLOT(renderPageFinished(int)));
    connect(m_report, SIGNAL(renderFinished()), this, SLOT(renderFinished()));
    // percent
    connect(ui->comboScalePercent, SIGNAL(currentIndexChanged(int)), this, SLOT(slotScalePercentChanged(int)));
    connect(m_preview, SIGNAL(scalePercentChanged(int)), this, SLOT(slotSetTextScalePercentChanged(int)));
    // zoom
    connect(ui->btnZoomIn, SIGNAL(clicked()), m_preview, SLOT(zoomIn()));
    connect(ui->btnZoomOut, SIGNAL(clicked()), m_preview, SLOT(zoomOut()));

    connect(ui->pageNavigator, SIGNAL(valueChanged(int)), this, SLOT(slotPageNavigatorChanged(int)));
    connect(ui->btnBackPage, SIGNAL(clicked()), m_preview, SLOT(priorPage()));
    connect(ui->btnNextPage, SIGNAL(clicked()), m_preview, SLOT(nextPage()));

    connect(m_preview, SIGNAL(pagesSet(int)), this, SLOT(slotPagesSet(int)));

    connect(m_preview, SIGNAL(pageChanged(int)), this, SLOT(slotPageChanged(int)));

    connect(ui->btnPrintReport, SIGNAL(clicked()), m_preview, SLOT(print()));
    connect(ui->btnExportPdf, SIGNAL(clicked()), m_preview, SLOT(printToPDF()));
#endif

    connect(ui->btnChoicePeriod, &QToolButton::clicked, this, &Reports::openCustomPeriod);
    connect(ui->comboTypeReport, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&Reports::typeReportsCurrentIndexChanged));
    connect(ui->comboOrganizations, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&Reports::organizationCurrentIndexChanged));

    connect(ui->btnEditOrganization, &QToolButton::clicked, this, &Reports::openCatOrganization);
    connect(ui->btnEditContract, &QToolButton::clicked, this, &Reports::openCatContract);
    connect(ui->btnEditDoctor, &QToolButton::clicked, this, &Reports::openCatDoctor);

    connect(ui->btnGenerateReport, &QToolButton::clicked, this, &Reports::generateReport);
    connect(ui->btnOpenDesigner, &QToolButton::clicked, this, &Reports::openDesignerReport);
    connect(ui->btnSettingsReport, &QToolButton::clicked, this, &Reports::openSettingsReport);
}

void Reports::initPercentCombobox()
{
    for (int i = 10; i<310; i+=10){
        ui->comboScalePercent->addItem(QString("%1%").arg(i));
    }
    ui->comboScalePercent->setCurrentIndex(4);
}

void Reports::setStyleForButtom()
{
    ui->btnGenerateReport->setStyleSheet("QToolButton {border: 1px solid #8f8f91; "
                                         "border-radius: 4px;"
                                         "width: 95px;}"
                                         "QToolButton:hover {background-color: rgb(234,243,250);}"
                                         "QToolButton:pressed {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #dadbde, stop: 1 #f6f7fa)}");
    ui->btnZoomIn->setStyleSheet("border: 1px solid #8f8f91; "
                                 "border-radius: 4px;");
    ui->btnZoomOut->setStyleSheet("border: 1px solid #8f8f91; "
                                  "border-radius: 4px;");
    ui->btnPrintReport->setStyleSheet("QToolButton {border: 1px solid #8f8f91; "
                                      "border-radius: 4px;"
                                      "width: 95px;}"
                                      "QToolButton:hover {background-color: rgb(234,243,250);}"
                                      "QToolButton:pressed {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #dadbde, stop: 1 #f6f7fa)}");
    ui->btnOpenDesigner->setStyleSheet("QToolButton {border: 1px solid #8f8f91; "
                                       "border-radius: 4px;"
                                       "width: 95px;}"
                                       "QToolButton:hover {background-color: rgb(234,243,250);}"
                                       "QToolButton:pressed {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #dadbde, stop: 1 #f6f7fa)}");
    ui->btnExportPdf->setStyleSheet("QToolButton {border: 1px solid #8f8f91; "
                                    "border-radius: 4px;"
                                    "width: 95px;}"
                                    "QToolButton:hover {background-color: rgb(234,243,250);}"
                                    "QToolButton:pressed {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #dadbde, stop: 1 #f6f7fa)}");
    ui->btnSettingsReport->setStyleSheet("QToolButton {border: 1px solid #8f8f91; "
                                         "border-radius: 4px;"
                                         "width: 95px;}"
                                         "QToolButton:hover {background-color: rgb(234,243,250);}"
                                         "QToolButton:pressed {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #dadbde, stop: 1 #f6f7fa)}");

    ui->comboScalePercent->setStyle(style_fusion);
    ui->comboScalePercent->setStyleSheet("min-width: 3em;");

#if defined(Q_OS_WIN)
    ui->frame->setStyle(style_fusion);
    ui->frame_filter->setStyle(style_fusion);
    ui->frame_preview->setStyle(style_fusion);
#endif

}

// **********************************************************************************
// --- procesarea solicitarilor raportului

QString Reports::getMainQry()
{
    QString name_report         = ui->comboTypeReport->currentText();
    const int id_organization   = ui->comboOrganizations->itemData(ui->comboOrganizations->currentIndex(), Qt::UserRole).toInt();
    const int id_contract       = ui->comboContracts->itemData(ui->comboContracts->currentIndex(), Qt::UserRole).toInt();
    const int id_doctor         = ui->comboDoctors->itemData(ui->comboDoctors->currentIndex(), Qt::UserRole).toInt();

    const QDateTime m_dateStart = QDateTime(ui->dateStart->date(), QTime(00,00,00));
    const QDateTime m_dateEnd   = QDateTime(ui->dateEnd->date(), QTime(23,59,59));

    qInfo(logInfo()) << tr("Generarea raportului '%1' pe perioada %2 - %3").arg(name_report, m_dateStart.toString("dd.MM.yyyy hh:mm:ss"), m_dateEnd.toString("dd.MM.yyyy hh:mm:ss"));

    if (name_report == "Lista pacientilor (filtru - organizatii)"){

        QString str_qry;
        if (globals::thisMySQL)
            str_qry = "SELECT orderEcho.id,"
                      "   orderEcho.numberDoc,"
                      "   CONCAT(SUBSTRING(orderEcho.dateDoc,9,2), '.' ,SUBSTRING(orderEcho.dateDoc,6,2), '.' ,SUBSTRING(orderEcho.dateDoc,1,4)) AS dateDoc,"
                      "   CONCAT(pacients.name ,' ', pacients.fName ,', ', SUBSTRING(pacients.birthday, 9, 2) ,'.', SUBSTRING(pacients.birthday, 6, 2) ,'.', SUBSTRING(pacients.birthday, 1, 4)) AS Pacients,"
                      "   pacients.IDNP,"
                      "   CONCAT(orderEchoTable.cod, ' - ' ,orderEchoTable.name) AS investigation,";
        else
            str_qry = "SELECT orderEcho.id,"
                      "   orderEcho.numberDoc,"
                      "   substr(orderEcho.dateDoc,9,2) ||'.'|| substr(orderEcho.dateDoc,6,2) ||'.'|| substr(orderEcho.dateDoc,1,4) AS dateDoc,"
                      "   pacients.name || ' ' || pacients.fName || ', ' || substr(pacients.birthday, 9, 2) ||'.'|| substr(pacients.birthday, 6, 2) ||'.'|| substr(pacients.birthday, 1, 4) AS Pacients,"
                      "   pacients.IDNP,"
                      "   orderEchoTable.cod || ' - ' || orderEchoTable.name AS investigation,";

        if (ui->hidePricesTotal->isChecked())
            str_qry = str_qry + "'' AS price,";
        else
            str_qry = str_qry + "   orderEchoTable.price,";

        str_qry = str_qry + "   contracts.name AS contract,";

        if (ui->comboDoctors->currentIndex() > 0){
            if (globals::thisMySQL)
                str_qry = str_qry + "  CONCAT(doctors.name ,' ', SUBSTRING(doctors.fName, 1, 1) ,'.') AS doctor,";
            else
                str_qry = str_qry + "  doctors.name ||' '|| substr(doctors.fName, 1, 1) ||'.' AS doctor,";
        }

        str_qry = str_qry + "   organizations.name AS organization"
                            "  FROM orderEcho "
                            "  INNER JOIN pacients ON orderEcho.id_pacients = pacients.id"
                            "  INNER JOIN orderEchoTable ON orderEchoTable.id_orderEcho = orderEcho.id"
                            "  INNER JOIN contracts ON orderEcho.id_contracts = contracts.id ";

        if (ui->comboDoctors->currentIndex() > 0)
            str_qry = str_qry + "  INNER JOIN doctors ON orderEcho.id_doctors = doctors.id ";

        str_qry = str_qry + "  INNER JOIN organizations ON orderEcho.id_organizations = organizations.id"
                            " WHERE orderEcho.deletionMark = '2' AND ";

        if (ui->cashPayment->isChecked())
            str_qry = str_qry + " orderEcho.cardPayment = 0 AND ";

        if (ui->comboOrganizations->currentIndex() > 0)
            str_qry = str_qry + QString(" orderEcho.id_organizations = '%1' AND ").arg(QString::number(id_organization));
        if (ui->comboContracts->currentIndex() > 0)
            str_qry = str_qry + QString(" orderEcho.id_contracts = '%1' AND ").arg(QString::number(id_contract));
        if (ui->comboDoctors->currentIndex() > 0)
            str_qry = str_qry + QString(" orderEcho.id_doctors = '%1' AND ").arg(QString::number(id_doctor));

        str_qry = str_qry + QString(" orderEcho.dateDoc BETWEEN '%1' AND '%2';").arg(m_dateStart.toString("yyyy-MM-dd hh:mm:ss"), m_dateEnd.toString("yyyy-MM-dd hh:mm:ss"));

        return str_qry;

    } else if (name_report == "Lista pacientilor (filtru - doctori)"){

        return QString("SELECT orderEcho.id, "
                       "  orderEcho.numberDoc, "
                       "  substr(orderEcho.dateDoc,9,2) ||'.'|| substr(orderEcho.dateDoc,6,2) ||'.'|| substr(orderEcho.dateDoc,1,4) AS dateDoc, "
                       "  pacients.name || ' ' || pacients.fName || ', ' || substr(pacients.birthday, 9, 2) ||'.'|| substr(pacients.birthday, 6, 2) ||'.'|| substr(pacients.birthday, 1, 4)  AS Pacients, "
                       "  orderEchoTable.cod || ' - ' || orderEchoTable.name AS investigation, orderEchoTable.price, doctors.name ||' '|| substr(doctors.fName, 1, 1) ||'.' AS doctor, "
                       "  pacients.IDNP "
                       " FROM orderEcho "
                       "  INNER JOIN pacients ON orderEcho.id_pacients = pacients.id "
                       "  INNER JOIN orderEchoTable ON orderEchoTable.id_orderEcho = orderEcho.id "
                       "  INNER JOIN doctors ON orderEcho.id_doctors = doctors.id "
                       " WHERE orderEcho.deletionMark = '2' "
                       "  AND orderEcho.id_doctors = '%1' "
                       "  AND orderEcho.dateDoc BETWEEN '%2' AND '%3';")
                .arg(QString::number(id_doctor), m_dateStart.toString("yyyy-MM-dd hh:mm:ss"), m_dateEnd.toString("yyyy-MM-dd hh:mm:ss"));

    } else if (name_report == "Volumul investigatiilor"){

        QString str;
        str = "SELECT "
              "  SUM(CASE WHEN reportEcho.t_organs_internal = 1 AND reportEcho.t_urinary_system = 1 THEN reportEcho.t_organs_internal ELSE 0 END) AS Complex,"
              "  SUM(CASE WHEN reportEcho.t_organs_internal = 1 AND reportEcho.t_urinary_system = 0 THEN reportEcho.t_organs_internal ELSE 0 END) AS OrgansInternal,"
              "  SUM(CASE WHEN reportEcho.t_urinary_system = 1 AND reportEcho.t_organs_internal = 0 THEN 1 ELSE 0 END) AS UrinarySystem,"
              "  SUM(CASE WHEN reportEcho.t_prostate = 1 THEN 1 ELSE 0 END) AS Prostate,"
              "  SUM(CASE WHEN reportEcho.t_gynecology = 1 THEN 1 ELSE 0 END) AS Gynecology,"
              "  SUM(CASE WHEN reportEcho.t_thyroid = 1 THEN 1 ELSE 0 END) AS Thyroid,"
              "  SUM(CASE WHEN reportEcho.t_breast = 1 THEN 1 ELSE 0 END) AS Breast,"
              "  SUM(CASE WHEN reportEcho.t_gestation0 = 1 THEN 1 ELSE 0 END) AS Gestation0,"
              "  SUM(CASE WHEN reportEcho.t_gestation1 = 1 THEN 1 ELSE 0 END) AS Gestation1 "
              "FROM "
              "  reportEcho "
              "INNER JOIN orderEcho ON reportEcho.id_orderEcho = orderEcho.id "
              "WHERE ";

        if (ui->comboOrganizations->currentIndex() > 0)
            str = str + QString(" orderEcho.id_organizations = '%1' AND ").arg(QString::number(id_organization));
        if (ui->comboContracts->currentIndex() > 0)
            str = str + QString(" orderEcho.id_contracts = '%1' AND ").arg(QString::number(id_contract));
        if (ui->comboDoctors->currentIndex() > 0)
            str = str + QString(" orderEcho.id_doctors = '%1' AND ").arg(QString::number(id_doctor));

        str = str + "  reportEcho.deletionMark = 2 AND "
                    "reportEcho.dateDoc BETWEEN '" + m_dateStart.toString("yyyy-MM-dd hh:mm:ss") + "' AND '" + m_dateEnd.toString("yyyy-MM-dd hh:mm:ss") + "';";
        return str;
    }

    return "";
}

void Reports::setImageForReports()
{
    //------------------------------------------------------------------------------------------------------
    // determinam id organizatiei principale
    int id_organization = 0;
    int id_doctor = 0;
    QMap<QString, QString> items;
    if (db->getObjectDataByMainId("constants", "id_users", globals::idUserApp, items)){
        id_organization = items.constFind("id_organizations").value().toInt();
        id_doctor = items.constFind("id_doctors").value().toInt();
    }

    //------------------------------------------------------------------------------------------------------
    // logotipul
    QByteArray outByteArray = db->getOutByteArrayImage("constants", "logo", "id_users", globals::idUserApp);
    QPixmap outPixmap = QPixmap();
    if (outPixmap.loadFromData(outByteArray))
        exist_logo = 1;
    QImage m_logo = outPixmap.toImage();
    QStandardItem* img_item_logo = new QStandardItem("");
    if (ui->hideLogo->isChecked())
        img_item_logo->setData("", Qt::DisplayRole);
    else
        img_item_logo->setData(m_logo.scaled(300,50), Qt::DisplayRole);

    //------------------------------------------------------------------------------------------------------
    // stampila organizatiei
    QByteArray outByteArray_stamp = db->getOutByteArrayImage("organizations", "stamp", "id", id_organization);
    QPixmap outPixmap_stamp = QPixmap();
    outPixmap_stamp.loadFromData(outByteArray_stamp);
    QImage m_stamp = outPixmap_stamp.toImage();
    QStandardItem* img_item_stamp = new QStandardItem("");
    img_item_stamp->setData(m_stamp.scaled(200,200), Qt::DisplayRole);

    //------------------------------------------------------------------------------------------------------
    // semnatura doctorului
    QByteArray outByteArray_signature = db->getOutByteArrayImage("doctors", "signature", "id", id_doctor);
    QPixmap outPixmap_signature = QPixmap();
    outPixmap_signature.loadFromData(outByteArray_signature);
    QImage m_signature = outPixmap_signature.toImage();
    QStandardItem* img_item_signature = new QStandardItem("");
    img_item_signature->setData(m_signature.scaled(200,200), Qt::DisplayRole);

    //------------------------------------------------------------------------------------------------------
    // setam imaginile in model
    QList<QStandardItem *> items_img;
    items_img.append(img_item_logo);
    items_img.append(img_item_stamp);
    items_img.append(img_item_signature);
    model_img = new QStandardItemModel(this);
    model_img->setColumnCount(3);
    model_img->appendRow(items_img);
}

void Reports::setReportVariabiles()
{
    QString str_presentation_period;
    if (ui->dateStart->date() == ui->dateEnd->date())
        str_presentation_period = tr("pe perioada: ") + ui->dateStart->date().toString("dd.MM.yyyy");
    else
        str_presentation_period = tr("pe perioada: ") + ui->dateStart->date().toString("dd.MM.yyyy") + " - " + ui->dateEnd->date().toString("dd.MM.yyyy");

    m_report->dataManager()->clearUserVariables();
    m_report->dataManager()->setReportVariable("v_date_start",         QDateTime(ui->dateStart->date(), QTime(00,00,00)).toString("yyyy-MM-dd hh:mm:ss"));
    m_report->dataManager()->setReportVariable("v_date_end",           QDateTime(ui->dateEnd->date(), QTime(23,59,59)).toString("yyyy-MM-dd hh:mm:ss"));
    m_report->dataManager()->setReportVariable("v_id_organization",    ui->comboOrganizations->itemData(ui->comboOrganizations->currentIndex(), Qt::UserRole).toInt());
    m_report->dataManager()->setReportVariable("v_name_organization",  (ui->comboOrganizations->currentIndex() == 0) ? "" : "îndreptați de " + ui->comboOrganizations->currentText());
    m_report->dataManager()->setReportVariable("v_presentation_period", str_presentation_period);
    m_report->dataManager()->setReportVariable("v_hide_logo",           (ui->hideLogo->isChecked()) ? 1 : 0);
    m_report->dataManager()->setReportVariable("v_hide_data_organization", (ui->hideDataOrganization->isChecked()) ? 1 : 0);
    m_report->dataManager()->setReportVariable("v_hide_price",          (ui->hidePricesTotal->isChecked()) ? 1 : 0);
    m_report->dataManager()->setReportVariable("v_hide_signature",      (ui->hideSignatureStamped->isChecked()) ? 1 : 0);
    m_report->dataManager()->setReportVariable("v_hide_name_doctor",    (ui->hideNameDoctor->isChecked()) ? 1 : 0);
    m_report->dataManager()->setReportVariable("v_exist_logo",          exist_logo);
}

void Reports::saveSettingsReport()
{
    if (m_id == -1)
        insertUpdateDataReportInTableSettingsReports(true);
    else
        insertUpdateDataReportInTableSettingsReports(false);
}

void Reports::insertUpdateDataReportInTableSettingsReports(const bool insertData)
{
    const int id_organization = ui->comboOrganizations->itemData(ui->comboOrganizations->currentIndex(), Qt::UserRole).toInt();
    const int id_contract     = ui->comboContracts->itemData(ui->comboContracts->currentIndex(), Qt::UserRole).toInt();
    const int id_doctor       = ui->comboDoctors->itemData(ui->comboDoctors->currentIndex(), Qt::UserRole).toInt();

    if (insertData){

        db->getDatabase().transaction();

        QString str = "INSERT INTO settingsReports ("
                      "nameReport, "
                      "dateStart, "
                      "dateEnd, "
                      "id_organizations, "
                      "id_contracts, "
                      "id_doctors, "
                      "showOnLaunch, "
                      "hideLogo, "
                      "hideTitle, "
                      "hideDataOrganization, "
                      "hideFooter, "
                      "hideSignatureStamped, "
                      "hidePrice) "
                      "VALUES (:nameReport, :dateStart, :dateEnd, :id_organizations, :id_contracts, :id_doctors, :showOnLaunch, "
                      ":hideLogo, :hideTitle, :hideDataOrganization, :hideFooter, :hideSignatureStamped, :hidePrice);";
        QSqlQuery qry;
        qry.prepare(str);
        qry.bindValue(":nameReport",           ui->comboTypeReport->currentText());
        qry.bindValue(":dateStart",            ui->dateStart->date().toString("yyyy-MM-dd") + " 00:00:00");
        qry.bindValue(":dateEnd",              ui->dateEnd->date().toString("yyyy-MM-dd") + " 23:59:59");
        qry.bindValue(":id_organizations",     (id_organization > 0) ? id_organization : QVariant());
        qry.bindValue(":id_contracts",         (id_contract > 0) ? id_contract : QVariant());
        qry.bindValue(":id_doctors",           (id_doctor > 0) ? id_doctor : QVariant());
        if (globals::thisMySQL) {
            qry.bindValue(":showOnLaunch",         (ui->showOnLaunch->isChecked()) ? true : false);
            qry.bindValue(":hideLogo",             (ui->hideLogo->isChecked()) ? true : false);
            qry.bindValue(":hideTitle",            0);
            qry.bindValue(":hideDataOrganization", (ui->hideDataOrganization->isChecked()) ? true : false);
            qry.bindValue(":hideFooter",           (ui->hideNameDoctor->isChecked()) ? true : false);
            qry.bindValue(":hideSignatureStamped", (ui->hideSignatureStamped->isChecked()) ? true : false);
            qry.bindValue(":hidePrice",            (ui->hidePricesTotal->isChecked()) ? true : false);
        } else {
            qry.bindValue(":showOnLaunch",         (ui->showOnLaunch->isChecked()) ? 1 : 0);
            qry.bindValue(":hideLogo",             (ui->hideLogo->isChecked()) ? 1 : 0);
            qry.bindValue(":hideTitle",            0);
            qry.bindValue(":hideDataOrganization", (ui->hideDataOrganization->isChecked()) ? 1 : 0);
            qry.bindValue(":hideFooter",           (ui->hideNameDoctor->isChecked()) ? 1 : 0);
            qry.bindValue(":hideSignatureStamped", (ui->hideSignatureStamped->isChecked()) ? 1 : 0);
            qry.bindValue(":hidePrice",            (ui->hidePricesTotal->isChecked()) ? 1 : 0);
        }

        if (qry.exec()){
            db->getDatabase().commit();
        } else {
            db->getDatabase().rollback();
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Setarile raportului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Eroare de executarea a solicitarii la inserarea setarilor raportului '%1'").arg(ui->comboTypeReport->currentText()));
            msgBox.setDetailedText((qry.lastError().text().isEmpty()) ? tr("eroarea indisponibila") : qry.lastError().text());
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();
        }

    } else {

        db->getDatabase().transaction();

        QString str = "UPDATE settingsReports SET "
                      "nameReport           = :nameReport, "
                      "dateStart            = :dateStart, "
                      "dateEnd              = :dateEnd, "
                      "id_organizations     = :id_organizations, "
                      "id_contracts         = :id_contracts, "
                      "id_doctors           = :id_doctors, "
                      "showOnLaunch         = :showOnLaunch, "
                      "hideLogo             = :hideLogo, "
                      "hideTitle            = :hideTitle, "
                      "hideDataOrganization = :hideDataOrganization, "
                      "hideFooter           = :hideFooter, "
                      "hideSignatureStamped = :hideSignatureStamped, "
                      "hidePrice            = :hidePrice "
                      "WHERE id = :id;";
        QSqlQuery qry;
        qry.prepare(str);
        qry.bindValue(":id",                   m_id);
        qry.bindValue(":nameReport",           ui->comboTypeReport->currentText());
        qry.bindValue(":dateStart",            ui->dateStart->date().toString("yyyy-MM-dd") + " 00:00:00");
        qry.bindValue(":dateEnd",              ui->dateEnd->date().toString("yyyy-MM-dd") + " 23:59:59");
        qry.bindValue(":id_organizations",     (id_organization > 0) ? id_organization : QVariant());
        qry.bindValue(":id_contracts",         (id_contract > 0) ? id_contract : QVariant());
        qry.bindValue(":id_doctors",           (id_doctor > 0) ? id_doctor : QVariant());
        if (globals::thisMySQL){
            qry.bindValue(":showOnLaunch",         (ui->showOnLaunch->isChecked()) ? true : false);
            qry.bindValue(":hideLogo",             (ui->hideLogo->isChecked()) ? true : false);
            qry.bindValue(":hideTitle",            0);
            qry.bindValue(":hideDataOrganization", (ui->hideDataOrganization->isChecked()) ? true : false);
            qry.bindValue(":hideFooter",           (ui->hideNameDoctor->isChecked()) ? true : false);
            qry.bindValue(":hideSignatureStamped", (ui->hideSignatureStamped->isChecked()) ? true : false);
            qry.bindValue(":hidePrice",            (ui->hidePricesTotal->isChecked()) ? true : false);
        } else {
            qry.bindValue(":showOnLaunch",         (ui->showOnLaunch->isChecked()) ? 1 : 0);
            qry.bindValue(":hideLogo",             (ui->hideLogo->isChecked()) ? 1 : 0);
            qry.bindValue(":hideTitle",            0);
            qry.bindValue(":hideDataOrganization", (ui->hideDataOrganization->isChecked()) ? 1 : 0);
            qry.bindValue(":hideFooter",           (ui->hideNameDoctor->isChecked()) ? 1 : 0);
            qry.bindValue(":hideSignatureStamped", (ui->hideSignatureStamped->isChecked()) ? 1 : 0);
            qry.bindValue(":hidePrice",            (ui->hidePricesTotal->isChecked()) ? 1 : 0);
        }
        if (qry.exec()){
            db->getDatabase().commit();
        } else {
            db->getDatabase().rollback();
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Setarile raportului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Eroare de executarea a solicitarii la actualizarea setarilor raportului '%1'").arg(ui->comboTypeReport->currentText()));
            msgBox.setDetailedText((qry.lastError().text().isEmpty()) ? tr("eroarea indisponibila") : qry.lastError().text());
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();
        }
    }
}

int Reports::getIdReportShowOnLaunch() const
{
    QSqlQuery qry;
    qry.prepare("SELECT id FROM settingsReports WHERE showOnLaunch = 1;");
    if (qry.exec() && qry.next()){
        return qry.value(0).toInt();
    } else {
        return 0;
    }
}

void Reports::renderStarted()
{
    if (m_report->isShowProgressDialog()){
        m_currentPage = 0;
        m_progressDialog = new QProgressDialog(tr("Start render"),tr("Cancel"),0,0,this);
        //m_progressDialog->setWindowModality(Qt::WindowModal);
#if defined(Q_OS_LINUX)
        connect(m_progressDialog, &QProgressDialog::canceled, m_report, &LimeReport::ReportEngine::cancelRender);
#elif defined(Q_OS_WIN)
        connect(m_progressDialog, SIGNAL(canceled()), m_report, SLOT(cancelRender()));
#endif
        QApplication::processEvents();
        m_progressDialog->show();
    }
}

void Reports::renderPageFinished(int renderedPageCount)
{
    if (m_progressDialog){
        m_progressDialog->setLabelText(QString::number(renderedPageCount)+tr(" page rendered"));
        m_progressDialog->setValue(renderedPageCount);
    }
}

void Reports::renderFinished()
{
    if (m_progressDialog){
        m_progressDialog->close();
        delete m_progressDialog;
    }
    m_progressDialog = 0;
}

// **********************************************************************************
// --- procesarea slot-urilor

void Reports::slotSetTextScalePercentChanged(int percent)
{
    ui->comboScalePercent->setCurrentText(QString("%1%").arg(percent));
}

void Reports::slotScalePercentChanged(const int percent)
{
    Q_UNUSED(percent)
    QString txt = ui->comboScalePercent->currentText();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    m_preview->setScalePercent(txt.remove(txt.count() - 1, 1).toInt());
#else
    m_preview->setScalePercent(txt.remove(txt.size() - 1, 1).toInt());
#endif
}

void Reports::slotPageNavigatorChanged(const int page)
{
    m_preview->pageNavigatorChanged(page);
}

void Reports::slotPagesSet(const int pagesCount)
{
    ui->pageNavigator->setSuffix(tr(" din %1").arg(pagesCount));
    ui->pageNavigator->setMinimum(1);
    ui->pageNavigator->setMaximum(pagesCount);
    ui->pageNavigator->setValue(1);
}

void Reports::slotPageChanged(const int page)
{
    ui->pageNavigator->setValue(page);
}

// **********************************************************************************
// --- procesarea butoanelor

void Reports::generateReport()
{    
    if (ui->comboTypeReport->currentIndex() == 0){
        QMessageBox::warning(this, tr("Verificarea tipului raportului"), tr("Nu este indicat tipul raportului !!!"), QMessageBox::Ok);
        ui->comboTypeReport->setFocus();
        ui->comboTypeReport->showPopup();
        return;
    }

    if (ui->frame_filter->isVisible())
        ui->frame_filter->setVisible(false);

    // *************************************************************************************
    // setam imaginile - logotipul, stampila si semnatura
    setImageForReports();
    m_report->dataManager()->addModel("table_img", model_img, true);

    // *************************************************************************************
    // organizatia
    QSqlQueryModel* print_model_organization = new QSqlQueryModel(this);
    print_model_organization->setQuery(db->getQryFromTableConstantById(globals::idUserApp));
    m_report->dataManager()->addModel("main_organization", print_model_organization, false);

    // *************************************************************************************
    // main table
    QSqlQueryModel* print_model_main = new QSqlQueryModel(this);
    print_model_main->setQuery(getMainQry());
    m_report->dataManager()->addModel("main_table", print_model_main, false);

    // *************************************************************************************
    // load, show and refresh
    setReportVariabiles();
    m_report->setShowProgressDialog(true);
    m_report->isShowProgressDialog();

    m_report->loadFromFile(globals::pathReports + "/" + ui->comboTypeReport->currentText() + ".lrxml");
    m_preview->refreshPages();

    // *************************************************************************************
    // delete model
    delete print_model_organization; // eliberam memoria
    delete print_model_main;
    delete model_img;
}

void Reports::openDesignerReport()
{
    // *************************************************************************************
    // setam imaginile - logotipul, stampila si semnatura
    setImageForReports();
    m_report->dataManager()->addModel("table_img", model_img, true);

    // *************************************************************************************
    // organizatia
    QSqlQueryModel* print_model_organization = new QSqlQueryModel(this);
    print_model_organization->setQuery(db->getQryFromTableConstantById(globals::idUserApp));
    m_report->dataManager()->addModel("main_organization", print_model_organization, false);

    // *************************************************************************************
    // main table
    QSqlQueryModel* print_model_main = new QSqlQueryModel(this);
    qDebug() << getMainQry();
    print_model_main->setQuery(getMainQry());
    m_report->dataManager()->addModel("main_table", print_model_main, false);

    // *************************************************************************************
    // load, show and refresh
    setReportVariabiles();
    m_report->setShowProgressDialog(true);

    if (ui->comboTypeReport->currentText() == tr("<<- selectează raport ->>"))
        m_report->loadFromFile("");
    else
        m_report->loadFromFile(globals::pathReports + "/" + ui->comboTypeReport->currentText() + ".lrxml");

    m_report->designReport();
    m_preview->refreshPages();

    // *************************************************************************************
    // delete model
    delete print_model_organization; // eliberam memoria
    delete print_model_main;
    delete model_img;
}

void Reports::openSettingsReport()
{
    if (ui->frame_filter->isVisible())
        ui->frame_filter->setVisible(false);
    else
        ui->frame_filter->setVisible(true);
}

void Reports::openCustomPeriod()
{
    customPeriod = new CustomPeriod(this);
    customPeriod->setAttribute(Qt::WA_DeleteOnClose);
    customPeriod->setDateStart(ui->dateStart->date());
    customPeriod->setDateEnd(ui->dateEnd->date());
    connect(customPeriod, &CustomPeriod::mChangePeriod, this, [this]()
    {
        ui->dateStart->setDate(customPeriod->getDateStart().date());
        ui->dateEnd->setDate(customPeriod->getDateEnd().date());
    });
    customPeriod->show();
}

// **********************************************************************************
// --- procesarea indexelor combo-urilor

void Reports::typeReportsCurrentIndexChanged(const int index)
{
    if (index == 0) // <<- selecteaza raportul->>
        return;

    if (m_id_onLaunch == -1)
        m_id_onLaunch = getIdReportShowOnLaunch();

    // setam perioada
    QString last_day(QDate(QDate::currentDate().year(), QDate::currentDate().month(), 01).addDays(-1).toString("dd"));  // determinam ultima zi a lunei
    ui->dateStart->setDateTime(QDateTime(QDate(QDate::currentDate().year(), QDate::currentDate().month(), 01), QTime(00, 00, 00)));
    ui->dateEnd->setDateTime(QDateTime(QDate(QDate::currentDate().year(), QDate::currentDate().month(), last_day.toInt()), QTime(23, 59, 59)));

    ui->comboOrganizations->setCurrentIndex(0);  // golim setarile la starea initiala
    ui->comboContracts->setCurrentIndex(0);
    ui->comboDoctors->setCurrentIndex(0);
    ui->hideLogo->setCheckState(Qt::Unchecked);
    ui->hideDataOrganization->setCheckState(Qt::Unchecked);
    ui->hidePricesTotal->setCheckState(Qt::Unchecked);
    ui->hideSignatureStamped->setCheckState(Qt::Unchecked);
    ui->showOnLaunch->setCheckState(Qt::Unchecked);

    if (! ui->frame_filter->isVisible())
        ui->frame_filter->setVisible(true);

    if (ui->comboTypeReport->currentText() == "Lista pacientilor (filtru - organizatii)"){
        ui->comboOrganizations->setEnabled(true);
        ui->comboContracts->setEnabled(true);
        ui->comboDoctors->setEnabled(true);
    } else if (ui->comboTypeReport->currentText() == "Lista pacientilor (filtru - doctori)"){
        ui->comboOrganizations->setEnabled(false);
        ui->comboContracts->setEnabled(false);
        ui->comboDoctors->setEnabled(true);
    } else if (ui->comboTypeReport->currentText() == "Volumul investigatiilor"){
        ui->comboOrganizations->setEnabled(true);
        ui->comboContracts->setEnabled(true);
        ui->comboDoctors->setEnabled(true);
    } else {
        ui->comboOrganizations->setEnabled(false);
        ui->comboContracts->setEnabled(false);
        ui->comboDoctors->setEnabled(false);
    }

    m_id = -1; // id raportului la starea initiala
    loadSettingsReport();
}

void Reports::organizationCurrentIndexChanged(const int index)
{
    const int id_organization = ui->comboOrganizations->itemData(index, Qt::UserRole).toInt();
    if (id_organization == 0)
        return;

    modelContracts->clear();                                        // actualizam solicitarea
    QString strQryContracts = "SELECT id,name FROM contracts "      // filtru dupa organizatia
                              "WHERE deletionMark = 0 AND id_organizations = "
                                + QString::number(id_organization)+ " AND notValid = 0;";
    modelContracts->setQuery(strQryContracts);

    QMap<QString, QString> items;
    if (db->getObjectDataById("organizations", id_organization, items)){
        const int id_contract = items.constFind("id_contracts").value().toInt();
        if (id_contract > 0){
            auto index_contract = modelContracts->match(modelContracts->index(0,0), Qt::UserRole, id_contract, 1, Qt::MatchExactly);
            if (! index_contract.isEmpty()){
                ui->comboContracts->setCurrentIndex(index_contract.first().row());
                ui->comboContracts->setEnabled(true);
            }
        }
    }
}

// **********************************************************************************
// --- procesarea butoanelor cataloagelor

void Reports::openCatOrganization()
{
    const int id_organization = ui->comboOrganizations->itemData(ui->comboOrganizations->currentIndex(), Qt::UserRole).toInt();
    if (id_organization == 0)
        return;
    cat_organization = new CatOrganizations(this);
    cat_organization->setAttribute(Qt::WA_DeleteOnClose);
    cat_organization->setProperty("ItNew", false);
    cat_organization->setProperty("Id", id_organization);
    cat_organization->show();
}

void Reports::openCatContract()
{
    const int id_organization = ui->comboOrganizations->itemData(ui->comboOrganizations->currentIndex(), Qt::UserRole).toInt();
    const int id_contract     = ui->comboContracts->itemData(ui->comboContracts->currentIndex(), Qt::UserRole).toInt();
    if (id_organization == 0 && id_contract == 0)
        return;
    cat_contract = new CatContracts(this);
    cat_contract->setAttribute(Qt::WA_DeleteOnClose);
    cat_contract->setProperty("ItNew", false);
    cat_contract->setProperty("Id", id_contract);
    cat_contract->setProperty("IdOrganization", id_organization);
    cat_contract->show();
}

void Reports::openCatDoctor()
{
    const int id_doctor = ui->comboDoctors->itemData(ui->comboDoctors->currentIndex(), Qt::UserRole).toInt();
    if (id_doctor == 0)
        return;
    cat_doctor = new CatGeneral(this);
    cat_doctor->setAttribute(Qt::WA_DeleteOnClose);
    cat_doctor->setProperty("typeCatalog", CatGeneral::TypeCatalog::Doctors);
    cat_doctor->setProperty("itNew", false);
    cat_doctor->setProperty("Id", id_doctor);
    cat_doctor->show();
}

// **********************************************************************************
// --- evenimente

void Reports::closeEvent(QCloseEvent *event)
{
    if (event->type() == QEvent::Close){
        saveSettingsReport();
    }
}
