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

#include "reports.h"
#include "qabstractitemview.h"
#include "qprogressdialog.h"
#include "ui_reports.h"

#include <customs/custommessage.h>
#include <common/agentsendemail.h>
#include <QPushButton>

Reports::Reports(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Reports),
    settings(globals().pathSettingsReport)
{
    ui->setupUi(this);

    setWindowTitle(tr("Generatorul de rapoarte"));

    m_report = new LimeReport::ReportEngine(this);
    m_preview = m_report->createPreviewWidget();

    if (globals().isSystemThemeDark)
        m_preview->setPreviewPageBackgroundColor(QColor(75,75,75));
    else
        m_preview->setPreviewPageBackgroundColor(QColor(179,179,179));

    ui->comboTypeReport->addItems(settings.getListReports()); // setam denumirea rapoartelor

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

    ui->frame_preview->resize(616, ui->frame_preview->height());
    ui->comboTypeReport->resize(260, ui->comboTypeReport->height());

    if (globals().isSystemThemeDark) {
        ui->frame_filter->setObjectName("customFrame");
        ui->frame_preview->setObjectName("customFrame");
    }
}

Reports::~Reports()
{
    delete modelDoctors;
    delete modelContracts;
    delete modelOrganizations;
    delete ui;
}

// **********************************************************************************
// --- initierea datelor raportului

void Reports::loadSettingsReport()
{
    QString reportId;
    if (ui->comboTypeReport->currentIndex() == 0){
        reportId = settings.showOnLaunchRaportId();
        if (! reportId.isEmpty()) {
            disconnect(ui->comboTypeReport, QOverload<int>::of(&QComboBox::currentIndexChanged),
                       this, QOverload<int>::of(&Reports::typeReportsCurrentIndexChanged));
            ui->comboTypeReport->setCurrentText(reportId);
            connect(ui->comboTypeReport, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this, QOverload<int>::of(&Reports::typeReportsCurrentIndexChanged));
            ui->showOnLaunch->setChecked(true);
        }
    } else {
        reportId = ui->comboTypeReport->currentText();
        ui->showOnLaunch->setChecked(reportId == settings.showOnLaunchRaportId());
    }

    // perioada
    ui->dateStart->setDate(settings.getValue(reportId, "startDate").toDate());
    ui->dateEnd->setDate(settings.getValue(reportId, "endDate").toDate());

    // organizatia
    auto indexOrganization = modelOrganizations->match(modelOrganizations->index(0, 0), Qt::UserRole,
                                                       settings.getValue(reportId, "id_organization").toInt(), 1, Qt::MatchExactly);
    if (! indexOrganization.isEmpty())
        ui->comboOrganizations->setCurrentIndex(indexOrganization.first().row());

    // contract
    auto indexContract = modelContracts->match(modelContracts->index(0, 0), Qt::UserRole,
                                               settings.getValue(reportId, "id_contract").toInt(), 1, Qt::MatchExactly);
    if (! indexContract.isEmpty())
        ui->comboContracts->setCurrentIndex(indexContract.first().row());

    // doctor
    auto indexDoctor = modelDoctors->match(modelDoctors->index(0,0), Qt::UserRole,
                                           settings.getValue(reportId, "id_doctor").toInt(), 1, Qt::MatchExactly);
    if (! indexDoctor.isEmpty())
        ui->comboDoctors->setCurrentIndex(indexDoctor.first().row());

    ui->hideLogo->setChecked(settings.getValue(reportId, "hideLogo").toBool());
    ui->hideDataOrganization->setChecked(settings.getValue(reportId, "hideDataOrganization").toBool());
    ui->hideNameDoctor->setChecked(settings.getValue(reportId, "hideNameDoctor").toBool());
    ui->hideSignatureStamped->setChecked(settings.getValue(reportId, "hideSignatureStamp").toBool());
    ui->hidePricesTotal->setChecked(settings.getValue(reportId, "hidePriceTotal").toBool());

    generateReport();

}

void Reports::initConnections()
{

    QString style_toolButton = db->getStyleForToolButton();
    ui->btnGenerateReport->setStyleSheet(style_toolButton);
    ui->btnOpenDesigner->setStyleSheet(style_toolButton);
    ui->btnPrintReport->setStyleSheet(style_toolButton);
    ui->btnExportPdf->setStyleSheet(style_toolButton);
    ui->btnSettingsReport->setStyleSheet(style_toolButton);
    ui->btnChoicePeriod->setStyleSheet(style_toolButton);
    ui->btnZoomIn->setStyleSheet(style_toolButton);
    ui->btnZoomOut->setStyleSheet(style_toolButton);
    ui->btnBackPage->setStyleSheet(style_toolButton);
    ui->btnNextPage->setStyleSheet(style_toolButton);
    ui->btnSendEmail->setStyleSheet(style_toolButton);

    ui->btnEditContract->setStyleSheet(style_toolButton);
    ui->btnEditDoctor->setStyleSheet(style_toolButton);
    ui->btnEditOrganization->setStyleSheet(style_toolButton);

    // butoane principale ale generatorului de rapoarte
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

    // filter
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
    connect(ui->btnSendEmail, &QToolButton::clicked, this, &Reports::sendReportToEmail);
}

void Reports::initDisconnections()
{
    // butoane principale ale generatorului de rapoarte
    disconnect(m_report, &LimeReport::ReportEngine::renderStarted, this, &Reports::renderStarted);
    disconnect(m_report, QOverload<int>::of(&LimeReport::ReportEngine::renderPageFinished), this,
               QOverload<int>::of(&Reports::renderPageFinished));
    disconnect(m_report, &LimeReport::ReportEngine::renderFinished, this, &Reports::renderFinished);

    disconnect(ui->comboScalePercent, QOverload<int>::of(&QComboBox::currentIndexChanged),
               this, QOverload<int>::of(&Reports::slotScalePercentChanged));
    disconnect(ui->btnZoomIn, &QToolButton::clicked, m_preview, &LimeReport::PreviewReportWidget::zoomIn);
    disconnect(ui->btnZoomOut, &QToolButton::clicked, m_preview, &LimeReport::PreviewReportWidget::zoomOut);

    disconnect(m_preview, QOverload<int>::of(&LimeReport::PreviewReportWidget::scalePercentChanged),
               this, QOverload<int>::of(&Reports::slotSetTextScalePercentChanged));

    disconnect(ui->pageNavigator, QOverload<int>::of(&QSpinBox::valueChanged),
               this, QOverload<int>::of(&Reports::slotPageNavigatorChanged));
    disconnect(ui->btnBackPage, &QToolButton::clicked, m_preview, &LimeReport::PreviewReportWidget::priorPage);
    disconnect(ui->btnNextPage, &QToolButton::clicked, m_preview, &LimeReport::PreviewReportWidget::nextPage);

    disconnect(m_preview, QOverload<int>::of(&LimeReport::PreviewReportWidget::pagesSet),
               this, QOverload<int>::of(&Reports::slotPagesSet));

    disconnect(m_preview, QOverload<int>::of(&LimeReport::PreviewReportWidget::pageChanged),
               this, QOverload<int>::of(&Reports::slotPageChanged));

    disconnect(ui->btnPrintReport, &QToolButton::clicked, m_preview, &LimeReport::PreviewReportWidget::print);
    disconnect(ui->btnExportPdf, &QToolButton::clicked, m_preview, &LimeReport::PreviewReportWidget::printToPDF);

    // filter
    disconnect(ui->btnChoicePeriod, &QToolButton::clicked, this, &Reports::openCustomPeriod);
    disconnect(ui->comboTypeReport, QOverload<int>::of(&QComboBox::currentIndexChanged),
               this, QOverload<int>::of(&Reports::typeReportsCurrentIndexChanged));
    disconnect(ui->comboOrganizations, QOverload<int>::of(&QComboBox::currentIndexChanged),
               this, QOverload<int>::of(&Reports::organizationCurrentIndexChanged));

    disconnect(ui->btnEditOrganization, &QToolButton::clicked, this, &Reports::openCatOrganization);
    disconnect(ui->btnEditContract, &QToolButton::clicked, this, &Reports::openCatContract);
    disconnect(ui->btnEditDoctor, &QToolButton::clicked, this, &Reports::openCatDoctor);

    disconnect(ui->btnGenerateReport, &QToolButton::clicked, this, &Reports::generateReport);
    disconnect(ui->btnOpenDesigner, &QToolButton::clicked, this, &Reports::openDesignerReport);
    disconnect(ui->btnSettingsReport, &QToolButton::clicked, this, &Reports::openSettingsReport);
    disconnect(ui->btnSendEmail, &QToolButton::clicked, this, &Reports::sendReportToEmail);
}

void Reports::initPercentCombobox()
{
    for (int i = 10; i<310; i+=10){
        ui->comboScalePercent->addItem(QString("%1%").arg(i));
    }
    ui->comboScalePercent->setCurrentIndex(4);
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

    qInfo(logInfo()) << tr("Generarea raportului '%1' pe perioada %2 - %3")
                            .arg(name_report, m_dateStart.toString("dd.MM.yyyy hh:mm:ss"), m_dateEnd.toString("dd.MM.yyyy hh:mm:ss"));

    if (name_report == "Lista pacientilor (filtru - organizatii)"){

        QString str_qry;
        if (globals().thisMySQL)
            str_qry = "SELECT "
                      "  orderEcho.id,"
                      "  orderEcho.numberDoc,"
                      "  DATE_FORMAT(orderEcho.dateDoc, '%d.%m.%Y') AS dateDoc,"
                      "  CONCAT(pacients.name ,' ', pacients.fName ,', ', DATE_FORMAT(pacients.birthday, '%d.%m.%Y')) AS Pacients,"
                      "  pacients.IDNP,"
                      "  CONCAT(orderEchoTable.cod, ' - ' ,orderEchoTable.name) AS investigation,";
        else
            str_qry = "SELECT "
                      "  orderEcho.id,"
                      "  orderEcho.numberDoc,"
                      "  strftime('%d.%m.%Y', orderEcho.dateDoc) AS dateDoc,"
                      "  pacients.name || ' ' || pacients.fName || ', ' || strftime('%d.%m.%Y', pacients.birthday) AS Pacients,"
                      "  pacients.IDNP,"
                      "  orderEchoTable.cod || ' - ' || orderEchoTable.name AS investigation,";

        if (ui->hidePricesTotal->isChecked())
            str_qry = str_qry + "'' AS price,";
        else
            str_qry = str_qry + "   orderEchoTable.price,";

        str_qry = str_qry + "   contracts.name AS contract,";

        if (ui->comboDoctors->currentIndex() > 0){
            if (globals().thisMySQL)
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

    } else if (name_report == "Structura patologiilor"){

        QString str;
        str = "SELECT"
              "  sum(CASE WHEN tableLiver.concluzion LIKE '%Steatoza%' THEN 1 ELSE 0 END) AS Steatoza,"
              "  sum(CASE WHEN tableLiver.concluzion LIKE '%Hepatomegalie%' THEN 1 ELSE 0 END) AS Hepatomegalie,"
              "  sum(CASE WHEN tableLiver.concluzion LIKE '%Chist hepatic%' THEN 1 ELSE 0 END) AS Chist_hepatic,"
              "  sum(CASE WHEN tableLiver.concluzion LIKE '%Hemangiom%' THEN 1 ELSE 0 END) AS Hemangiom,"
              "  sum(CASE WHEN tableLiver.concluzion LIKE '%Hipertenzia portală%' THEN 1 ELSE 0 END) AS Hipertenzia_portala,"
              "  sum(CASE WHEN tableLiver.concluzion LIKE '%Ciroza%' THEN 1 ELSE 0 END) AS Ciroza,"
              "  sum(CASE WHEN tableLiver.concluzion LIKE '%Formațiune de volum a ficatului%' THEN 1 ELSE 0 END) AS Formations_hepatic,"
              "  sum(CASE WHEN tableLiver.concluzion LIKE '%Colecistita cronică acalculoasă%' THEN 1 ELSE 0 END) + sum(CASE WHEN tableLiver.concluzion LIKE '%Brid inflexiune%' THEN 1 ELSE 0 END) AS Colecistita_acalculoasa,"
              "  sum(CASE WHEN tableLiver.concluzion LIKE '%Colecistita cronică calculoasă%' THEN 1 ELSE 0 END) AS Colecistita_calculoasa,"
              "  sum(CASE WHEN tableLiver.concluzion LIKE '%Polip colecistic%' THEN 1 ELSE 0 END) AS Polip_colecist,"
              "  sum(CASE WHEN tableLiver.concluzion LIKE '%Schimbări difuze în parenchimul pancreasului%' THEN 1 ELSE 0 END) AS Pancreatita,"
              "  sum(CASE WHEN tableLiver.concluzion LIKE '%Chist pancreasului%' THEN 1 ELSE 0 END) AS Chist_pancreas,"
              "  sum(CASE WHEN tableLiver.concluzion LIKE '%Formațiune de volum a pancreasului%' THEN 1 ELSE 0 END) AS Formations_pancreas,"
              "  sum(CASE WHEN tableLiver.concluzion LIKE '%Splenomegal%' THEN 1 ELSE 0 END) AS Splenomegalie,"
              "  sum(CASE WHEN tableLiver.concluzion LIKE '%Formațiune de volum a splinei%' THEN 1 ELSE 0 END) AS Formations_spleen "
              "FROM tableLiver "
              "INNER JOIN reportEcho ON tableLiver.id_reportEcho = reportEcho.id "
              "WHERE reportEcho.dateDoc BETWEEN '" + m_dateStart.toString("yyyy-MM-dd hh:mm:ss") + "' AND '" + m_dateEnd.toString("yyyy-MM-dd hh:mm:ss") + "';";

        return str;

    } else if (name_report == "Volumul investigatiilor per cod si institutie"){

        QString str =
            "SELECT "
            "  orderEchoTable.cod,"
            "  investigations.name AS investigation,";

        if (ui->comboOrganizations->currentIndex() > 0)
            str = str + " organizations.name AS organization,";

        if (ui->comboDoctors->currentIndex() > 0)
            str = str + " fullNameDoctors.nameAbbreviated AS doctor,";

        str = str + "  COUNT(*) AS total_count "
                    "FROM "
                    "  orderEchoTable "
                    "JOIN "
                    "  orderEcho ON orderEchoTable.id_orderEcho = orderEcho.id "
                    "JOIN "
                    "  investigations ON investigations.cod = orderEchoTable.cod ";

        if (ui->comboOrganizations->currentIndex() > 0)
            str = str + "JOIN organizations ON organizations.id = orderEcho.id_organizations ";

        if (ui->comboDoctors->currentIndex() > 0)
            str = str + "JOIN fullNameDoctors ON fullNameDoctors.id_doctors = orderEcho.id_doctors ";

        str = str + "WHERE orderEcho.dateDoc BETWEEN '" + m_dateStart.toString("yyyy-MM-dd hh:mm:ss") + "' AND '" + m_dateEnd.toString("yyyy-MM-dd hh:mm:ss") + "' "
                    "AND investigations.use = 1 ";

        if (ui->comboOrganizations->currentIndex() > 0)
            str = str + " AND organizations.id = " + ui->comboOrganizations->itemData(ui->comboOrganizations->currentIndex(), Qt::UserRole).toString();

        if (ui->comboDoctors->currentIndex() > 0)
            str = str + " AND fullNameDoctors.id_doctors = " + ui->comboDoctors->itemData(ui->comboDoctors->currentIndex(), Qt::UserRole).toString();

        str = str + " GROUP BY orderEchoTable.cod, investigations.name ";

        if (ui->comboOrganizations->currentIndex() > 0)
            str = str + ", organizations.name ";

        if (ui->comboDoctors->currentIndex() > 0)
            str = str + ", fullNameDoctors.nameAbbreviated ";

        str = str + "ORDER BY orderEchoTable.cod;";

        return str;

    }

    return "";
}

QString Reports::getQuerySystem(const QString str_sytem)
{
    const QDateTime m_dateStart = QDateTime(ui->dateStart->date(), QTime(00,00,00));
    const QDateTime m_dateEnd   = QDateTime(ui->dateEnd->date(), QTime(23,59,59));

    if (str_sytem == "urinary_system_nozology"){
        QString str;
        str = "SELECT"
              "  sum(CASE WHEN tableKidney.concluzion LIKE '%Micronefrolitiaza%' THEN 1 ELSE 0 END) AS Micronefrolitiaza,"
              "  sum(CASE WHEN tableKidney.concluzion LIKE '%Chist%' THEN 1 ELSE 0 END) AS Chist_renal,"
              "  sum(CASE WHEN tableKidney.concluzion LIKE '%Multichistoza%' THEN 1 ELSE 0 END) AS Multichistoza_renal,"
              "  sum(CASE WHEN tableKidney.concluzion LIKE '%Polichistoza%' THEN 1 ELSE 0 END) AS Polichistoza_renal,"
              "  sum(CASE WHEN tableKidney.concluzion LIKE '%Nefrolitiaza%' THEN 1 ELSE 0 END) AS Nefrolitiaza,"
              "  sum(CASE WHEN tableKidney.concluzion LIKE '%Hidronefroza%' THEN 1 ELSE 0 END) AS Hidronefroza,"
              "  sum(CASE WHEN tableKidney.concluzion LIKE '%Pielonefrita%' THEN 1 ELSE 0 END) AS Pielonefrita,"
              "  sum(CASE WHEN tableKidney.concluzion LIKE '%Bloc%' THEN 1 ELSE 0 END) AS Bloc_renal "
              "FROM tableKidney "
              "INNER JOIN reportEcho ON tableKidney.id_reportEcho = reportEcho.id "
              "WHERE reportEcho.dateDoc BETWEEN '" + m_dateStart.toString("yyyy-MM-dd hh:mm:ss") + "' AND '" + m_dateEnd.toString("yyyy-MM-dd hh:mm:ss") + "';";
        return str;
    } else if (str_sytem == "breast_nozology"){
        QString str;
        str = "SELECT"
              "  sum(CASE WHEN tableBreast.concluzion LIKE '%BI-RADS I%' THEN 1 ELSE 0 END) AS BI_I,"
              "  sum(CASE WHEN tableBreast.concluzion LIKE '%Modificări fibro-chistice%' THEN 1 ELSE 0 END) AS BI_II,"
              "  sum(CASE WHEN tableBreast.concluzion LIKE '%Fibroadenoma%' THEN 1 ELSE 0 END) AS BI_III,"
              "  sum(CASE WHEN tableBreast.concluzion LIKE '%IV%' THEN 1 ELSE 0 END) AS BI_IV "
              "FROM tableBreast "
              "INNER JOIN reportEcho ON tableBreast.id_reportEcho = reportEcho.id "
              "WHERE reportEcho.dateDoc BETWEEN '" + m_dateStart.toString("yyyy-MM-dd hh:mm:ss") + "' AND '" + m_dateEnd.toString("yyyy-MM-dd hh:mm:ss") + "';";
        return str;
    } else if (str_sytem == "ginecology_nozology"){
        QString str;
        str = "SELECT"
              "  sum(CASE WHEN tableGynecology.concluzion LIKE '%Miom uterin nodular%' THEN 1 ELSE 0 END) AS Miom_nodular,"
              "  sum(CASE WHEN tableGynecology.concluzion LIKE '%Miom uterin difuz%' THEN 1 ELSE 0 END) AS Miom_difuz,"
              "  sum(CASE WHEN tableGynecology.concluzion LIKE '%Chist ovarian%' THEN 1 ELSE 0 END) AS Chist_var,"
              "  sum(CASE WHEN tableGynecology.concluzion LIKE '%Polichistoza ovariană%' THEN 1 ELSE 0 END) AS Polichistoza_ovar,"
              "  sum(CASE WHEN tableGynecology.concluzion LIKE '%Endometrioza%' THEN 1 ELSE 0 END) AS Endometrioza,"
              "  sum(CASE WHEN tableGynecology.concluzion LIKE '%Hiperplazia%' THEN 1 ELSE 0 END) + sum(CASE WHEN tableGynecology.concluzion LIKE '%Polip%' THEN 1 ELSE 0 END) AS Hiperplazia_polip,"
              "  sum(CASE WHEN tableGynecology.concluzion LIKE '%Sarcina ectopică%' THEN 1 ELSE 0 END) AS Sarcina_ectopica "
              "FROM tableGynecology "
              "INNER JOIN reportEcho ON tableGynecology.id_reportEcho = reportEcho.id "
              "WHERE reportEcho.dateDoc BETWEEN '" + m_dateStart.toString("yyyy-MM-dd hh:mm:ss") + "' AND '" + m_dateEnd.toString("yyyy-MM-dd hh:mm:ss") + "';";
        return str;
    }

    return nullptr;
}

void Reports::setImageForReports()
{
    //------------------------------------------------------------------------------------------------------
    // ----- logotipul

    QPixmap pix_logo = QPixmap();
    QStandardItem *img_item_logo = new QStandardItem();
    QString name_key_logo = "logo_" + globals().nameUserApp;
    // --- verifiam cache
    if (! globals().cache_img.find(name_key_logo, &pix_logo)){
        if (! globals().c_logo_byteArray.isEmpty() && pix_logo.loadFromData(globals().c_logo_byteArray)){
            globals().cache_img.insert(name_key_logo, pix_logo);
            exist_logo = 1;
        }
    }
    // --- setam logotipul
    if (! pix_logo.isNull()) {
        if (ui->hideLogo->isChecked()) {
            img_item_logo->setData("", Qt::DisplayRole);
        } else {
            img_item_logo->setData(pix_logo.scaled(300,50, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage(), Qt::DisplayRole);
        }
    }

    //------------------------------------------------------------------------------------------------------
    // ----- stampila organizatiei
    QPixmap outPixmap_stamp = QPixmap();
    QStandardItem *img_item_stamp = new QStandardItem();
    QString name_key_stamp_organization = "stamp_organization_id-" + QString::number(globals().c_id_organizations) + "_" + globals().nameUserApp;
    // --- verifiam cache
    if (! globals().cache_img.find(name_key_stamp_organization, &outPixmap_stamp)) {
        if (! globals().main_stamp_organization.isEmpty() && outPixmap_stamp.loadFromData(globals().main_stamp_organization)){
            globals().cache_img.insert(name_key_stamp_organization, outPixmap_stamp);
            exist_stamp_organization = 1;
        }
    }
    // --- setam stampila
    if (! outPixmap_stamp.isNull()) {
        img_item_stamp->setData(outPixmap_stamp.scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage(), Qt::DisplayRole);
    }

    //------------------------------------------------------------------------------------------------------
    // ----- semnatura doctorului
    QPixmap outPixmap_signature = QPixmap();
    QStandardItem* img_item_signature = new QStandardItem();
    QString name_key_signature = "signature_doctor_id-" + QString::number(globals().c_id_doctor) + "_" + globals().nameUserApp;
    // --- verificam cache
    if (! globals().cache_img.find(name_key_signature, &outPixmap_signature)) {
        if(! globals().signature_main_doctor.isEmpty() && outPixmap_signature.loadFromData(globals().signature_main_doctor)) {
            globals().cache_img.insert(name_key_signature, outPixmap_signature);
            exist_signature_doctore = 1;
        }
    }
    // --- setam semnatura
    if (! outPixmap_signature.isNull()) {
        img_item_signature->setData(outPixmap_signature.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage(), Qt::DisplayRole);
    }

    //------------------------------------------------------------------------------------------------------
    // ----- setam imaginile in model
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
        str_presentation_period = tr("perioada: ") + ui->dateStart->date().toString("dd.MM.yyyy");
    else
        str_presentation_period = tr("perioada: ") + ui->dateStart->date().toString("dd.MM.yyyy") + " - " + ui->dateEnd->date().toString("dd.MM.yyyy");

    m_report->dataManager()->clearUserVariables();
    m_report->dataManager()->setReportVariable("v_date_start",         QDateTime(ui->dateStart->date(), QTime(00,00,00)).toString("yyyy-MM-dd hh:mm:ss"));
    m_report->dataManager()->setReportVariable("v_date_end",           QDateTime(ui->dateEnd->date(), QTime(23,59,59)).toString("yyyy-MM-dd hh:mm:ss"));
    m_report->dataManager()->setReportVariable("v_id_organization",    ui->comboOrganizations->itemData(ui->comboOrganizations->currentIndex(), Qt::UserRole).toInt());
    if (ui->comboTypeReport->currentText() == "Volumul investigatiilor per cod si institutie") {
        m_report->dataManager()->setReportVariable("v_name_organization",
                                                   (ui->comboDoctors->currentIndex() == 0) ?
                                                       "îndreptați de \n" + ui->comboOrganizations->currentText() :
                                                       "îndreptați de medic \n" + ui->comboDoctors->currentText() + " (" + ui->comboOrganizations->currentText() + ")");
    } else {
        m_report->dataManager()->setReportVariable("v_name_organization",
                                                   (ui->comboOrganizations->currentIndex() == 0) ?
                                                       "" :
                                                       "îndreptați de " + ui->comboOrganizations->currentText());
    }
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
    QString reportId = ui->comboTypeReport->currentText();

    if (ui->showOnLaunch->isChecked())
        settings.setShowOnLaunchRaport(reportId);

    settings.setValue(reportId, "startDate",       ui->dateStart->date());
    settings.setValue(reportId, "endDate",         ui->dateEnd->date());
    settings.setValue(reportId, "id_organization", ui->comboOrganizations->itemData(ui->comboOrganizations->currentIndex(), Qt::UserRole).toInt());
    settings.setValue(reportId, "id_contract",     ui->comboContracts->itemData(ui->comboContracts->currentIndex(), Qt::UserRole).toInt());
    settings.setValue(reportId, "id_doctor",       ui->comboDoctors->itemData(ui->comboDoctors->currentIndex(), Qt::UserRole).toInt());
    settings.setValue(reportId, "hideLogo",       (ui->hideLogo->isChecked()) ? 1 : 0);
    settings.setValue(reportId, "hideDataOrganization", (ui->hideDataOrganization->isChecked()) ? 1 : 0);
    settings.setValue(reportId, "hideNameDoctor",       (ui->hideNameDoctor->isChecked()) ? 1 : 0);
    settings.setValue(reportId, "hideSignatureStamp",   (ui->hideSignatureStamped->isChecked()) ? 1 : 0);
    settings.setValue(reportId, "hidePriceTotal",       (ui->hidePricesTotal->isChecked()) ? 1 : 0);
    settings.save();

}

void Reports::setReportVolumInvestigationsPerCod()
{
    QString str_doc =
        QStringLiteral(
        "SELECT "
        "  orderEcho.id_doctors AS id,"
        "  fullNameDoctors.nameAbbreviated "
        "FROM "
        "  orderEcho "
        "INNER JOIN "
        "  fullNameDoctors ON fullNameDoctors.id_doctors = orderEcho.id_doctors "
        "WHERE "
        "  orderEcho.dateDoc BETWEEN ? AND ? "
        "  AND orderEcho.id_organizations = ? "
        "GROUP BY "
        "  orderEcho.id_doctors,"
        "  fullNameDoctors.nameAbbreviated "
        "ORDER BY "
        "  fullNameDoctors.nameAbbreviated;"
        );
    list_doctor = new QSqlQuery(str_doc);
    list_doctor->bindValue(0, QDateTime(ui->dateStart->date(), QTime(00,00,00)));
    list_doctor->bindValue(1, QDateTime(ui->dateEnd->date(), QTime(23,59,59)));
    list_doctor->bindValue(2, ui->comboOrganizations->itemData(ui->comboOrganizations->currentIndex(), Qt::UserRole).toInt());
    if (! list_doctor->exec()) {
        qCritical() << "Error: Unable to fetch 'list_doctor' data:" << list_doctor->lastError().text();
        return;
    }

    callbackDatasource = m_report->dataManager()->createCallbackDatasource("list_doctors");
    connect(callbackDatasource, QOverload<const LimeReport::CallbackInfo &, QVariant &>::of(&LimeReport::ICallbackDatasource::getCallbackData),
            this, QOverload<LimeReport::CallbackInfo, QVariant &>::of(&Reports::slotGetCallbackDataDoctor));
    connect(callbackDatasource, QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&LimeReport::ICallbackDatasource::changePos),
            this, QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&Reports::slotChangePosDoctors));

    QString str_investig =
        QStringLiteral(
            "SELECT "
            "  orderEchoTable.cod,"
            "  investigations.name AS investigation,"
            "  organizations.name AS organization,"
            "  fullNameDoctors.nameAbbreviated AS doctor,"
            "  COUNT(*) AS total_count "
            "FROM "
            "  orderEchoTable "
            "JOIN "
            "  orderEcho ON orderEchoTable.id_orderEcho = orderEcho.id "
            "INNER JOIN "
            "  investigations ON investigations.cod = orderEchoTable.cod "
            "INNER JOIN "
            "  organizations ON organizations.id = orderEcho.id_organizations "
            "INNER JOIN "
            "  fullNameDoctors ON fullNameDoctors.id_doctors = orderEcho.id_doctors "
            "WHERE "
            "  orderEcho.dateDoc BETWEEN ? AND ? "
            "  AND investigations.use = 1 "
            "  AND organizations.id = ? "
            "  AND fullNameDoctors.id_doctors = ? "
            "GROUP BY "
            "  orderEchoTable.cod,"
            "  investigations.name,"
            "  organizations.name,"
            "  fullNameDoctors.nameAbbreviated "
            "ORDER BY "
            "  orderEchoTable.cod;"
            );

    list_investigation = new QSqlQuery(str_investig);

    list_investigation->bindValue(0, QDateTime(ui->dateStart->date(), QTime(00,00,00)));
    list_investigation->bindValue(1, QDateTime(ui->dateEnd->date(), QTime(23,59,59)));
    list_investigation->bindValue(2, ui->comboOrganizations->itemData(ui->comboOrganizations->currentIndex(), Qt::UserRole).toInt());
    list_investigation->bindValue(3, ui->comboDoctors->itemData(ui->comboDoctors->currentIndex(), Qt::UserRole).toInt());

    if (! list_investigation->exec()) {
        qCritical() << "Error: Unable to fetch 'list_investigation' data:" << list_investigation->lastError().text();
        return;
    }

    callbackDatasource = m_report->dataManager()->createCallbackDatasource("list_investigations");
    connect(callbackDatasource, QOverload<const LimeReport::CallbackInfo &, QVariant &>::of(&LimeReport::ICallbackDatasource::getCallbackData),
            this, QOverload<const LimeReport::CallbackInfo, QVariant &>::of(&Reports::slotGetCallbackDataInvestigation));
    connect(callbackDatasource, QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&LimeReport::ICallbackDatasource::changePos),
            this, QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&Reports::slotChangePosInvestigation));
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
        //connect(m_progressDialog, &QProgressDialog::canceled, m_report, &LimeReport::ReportEngine::cancelRender);
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
    m_preview->setScalePercent(txt.remove(txt.size() - 1, 1).toInt());
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
    if (m_preview) {

        // deconectam conexiunile ...
        initDisconnections();

        // eliminam m_report
        delete m_preview;
        delete m_report;

        // alocam memoria
        m_report = new LimeReport::ReportEngine(this);
        m_preview = m_report->createPreviewWidget();

        // stilul
        if (globals().isSystemThemeDark)
            m_preview->setPreviewPageBackgroundColor(QColor(75,75,75));
        else
            m_preview->setPreviewPageBackgroundColor(QColor(179,179,179));

        // prezentam in forma
        ui->frame_preview->layout()->addWidget(m_preview);
        ui->pageNavigator->setPrefix(tr("Pagina: "));

        // restabilim conectarile
        initConnections();

    }

    //-----------------------------------------------------------------------------
    // 📌 1 verificam daca e ales raportul
    if (ui->comboTypeReport->currentIndex() == 0){
        QMessageBox::warning(this, tr("Verificarea tipului raportului"), tr("Nu este indicat tipul raportului !!!"), QMessageBox::Ok);
        ui->comboTypeReport->setFocus();
        ui->comboTypeReport->showPopup();
        return;
    }

    //-----------------------------------------------------------------------------
    // 📌 2 inchidem filtru
    if (ui->frame_filter->isVisible())
        ui->frame_filter->setVisible(false);

    //-----------------------------------------------------------------------------
    // 📌 3 setam imaginile - logotipul, stampila si semnatura
    setImageForReports();
    m_report->dataManager()->addModel("table_img", model_img, true);

    //-----------------------------------------------------------------------------
    // 📌 4 setam datele organizatiei
    QSqlQueryModel *print_model_organization = new QSqlQueryModel(this);
    print_model_organization->setQuery(db->getQryFromTableConstantById(globals().idUserApp));
    m_report->dataManager()->addModel("main_organization", print_model_organization, false);

    //-----------------------------------------------------------------------------
    // 📌 5 main table
    QSqlQueryModel *print_model_main = new QSqlQueryModel(this);
    print_model_main->setQuery(getMainQry());
    m_report->dataManager()->addModel("main_table", print_model_main, false);
    if (ui->comboTypeReport->currentText() == "Volumul investigatiilor per cod si institutie"){
            setReportVolumInvestigationsPerCod();
    }

    //-----------------------------------------------------------------------------
    // 📌 6 table - s.urynary, breast, ginecology
    QSqlQueryModel *print_model_systemUrinary = new QSqlQueryModel(this);
    QSqlQueryModel *print_model_breast        = new QSqlQueryModel(this);
    QSqlQueryModel *print_model_ginecology    = new QSqlQueryModel(this);
    if (ui->comboTypeReport->currentText() == "Structura patologiilor"){

        print_model_systemUrinary->setQuery(getQuerySystem("urinary_system_nozology"));
        m_report->dataManager()->addModel("table_urinary_system", print_model_systemUrinary, false);

        print_model_breast->setQuery(getQuerySystem("breast_nozology"));
        m_report->dataManager()->addModel("table_breast", print_model_breast, false);

        print_model_ginecology->setQuery(getQuerySystem("ginecology_nozology"));
        m_report->dataManager()->addModel("table_ginecology", print_model_ginecology, false);
    }

    //-----------------------------------------------------------------------------
    // 📌 7 setam variabile necesare
    setReportVariabiles();

    //-----------------------------------------------------------------------------
    // 📌 8 prezentam progres bar
    m_report->setShowProgressDialog(true);
    m_report->isShowProgressDialog();

    //-----------------------------------------------------------------------------
    // 📌 9 incarcam fisierul si actualizam pagina
    m_report->loadFromFile(globals().pathReports + "/" + ui->comboTypeReport->currentText() + ".lrxml");
    m_preview->refreshPages();

    //-----------------------------------------------------------------------------
    // 📌 10 verificam daca trebuie de exportat in tmp/USG
    if (send_email) {
        QDir dir(globals().main_path_save_documents);
        if (! dir.exists()) {
            QDir().mkpath(globals().main_path_save_documents);
            qInfo(logInfo()) << "A fost creat directorul" << globals().main_path_save_documents;
        } else {
            if (dir.removeRecursively()) {
                qInfo(logInfo()) << "Directorul" << globals().main_path_save_documents << " a fost șters cu succes !";
            } else {
                qWarning(logWarning()) << "Eroare: Nu s-a putut șterge directorul - " << globals().main_path_save_documents;
            }
        }
        m_report->printToPDF(QDir::toNativeSeparators(globals().main_path_save_documents + "/" + ui->comboTypeReport->currentText() + ".pdf")); // pu transmiterea prin email
    }

    // *************************************************************************************
    // 📌 11 elibiram memoria
    print_model_organization->deleteLater(); // eliberam memoria
    print_model_main->deleteLater();
    print_model_systemUrinary->deleteLater();
    print_model_breast->deleteLater();
    print_model_ginecology->deleteLater();
    model_img->deleteLater();
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
    print_model_organization->setQuery(db->getQryFromTableConstantById(globals().idUserApp));
    m_report->dataManager()->addModel("main_organization", print_model_organization, false);

    // *************************************************************************************
    // main table
    QSqlQueryModel* print_model_main = new QSqlQueryModel(this);
    qDebug() << getMainQry();
    print_model_main->setQuery(getMainQry());
    m_report->dataManager()->addModel("main_table", print_model_main, false);

    QSqlQueryModel* print_model_systemUrinary = new QSqlQueryModel(this);
    QSqlQueryModel* print_model_breast = new QSqlQueryModel(this);
    QSqlQueryModel* print_model_ginecology = new QSqlQueryModel(this);
    if (ui->comboTypeReport->currentText() == "Structura patologiilor"){

        print_model_systemUrinary->setQuery(getQuerySystem("urinary_system_nozology"));
        m_report->dataManager()->addModel("table_urinary_system", print_model_systemUrinary, false);

        print_model_breast->setQuery(getQuerySystem("breast_nozology"));
        m_report->dataManager()->addModel("table_breast", print_model_breast, false);

        print_model_ginecology->setQuery(getQuerySystem("ginecology_nozology"));
        m_report->dataManager()->addModel("table_ginecology", print_model_ginecology, false);
    }

    // *************************************************************************************
    // load, show and refresh
    setReportVariabiles();
    m_report->setShowProgressDialog(true);

    if (ui->comboTypeReport->currentText() == tr("<<- selectează raport ->>"))
        m_report->loadFromFile("");
    else
        m_report->loadFromFile(globals().pathReports + "/" + ui->comboTypeReport->currentText() + ".lrxml");

    m_report->designReport();
    m_preview->refreshPages();

    // *************************************************************************************
    // delete model
    delete print_model_organization; // eliberam memoria
    delete print_model_main;
    delete print_model_systemUrinary;
    delete print_model_breast;
    delete print_model_ginecology;
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

    if (globals().show_info_reports){
        info_window = new InfoWindow(this);
        info_window->setTypeInfo(InfoWindow::TypeInfo::INFO_REPORT);
        info_window->setTitle(tr("Descrierea rapoartelor"));
        info_window->setTex(globals().str_content_message_report);
        info_window->exec();
    }

    if (! ui->frame_filter->isVisible())
        ui->frame_filter->setVisible(true);

    if (ui->comboTypeReport->currentText() == "Lista pacientilor (filtru - organizatii)"){

        ui->comboOrganizations->setVisible(true);
        ui->label_organization->setVisible(true);
        ui->btnEditOrganization->setVisible(true);

        ui->comboContracts->setVisible(true);
        ui->label_contract->setVisible(true);
        ui->btnEditContract->setVisible(true);

        ui->comboDoctors->setVisible(true);
        ui->label_doctor->setVisible(true);
        ui->btnEditDoctor->setVisible(true);

        ui->hidePricesTotal->setVisible(true);
        ui->cashPayment->setVisible(true);

    } else if (ui->comboTypeReport->currentText() == "Lista pacientilor (filtru - doctori)"){

        ui->comboOrganizations->setVisible(true);
        ui->label_organization->setVisible(true);
        ui->btnEditOrganization->setVisible(true);

        ui->comboContracts->setVisible(true);
        ui->label_contract->setVisible(true);
        ui->btnEditContract->setVisible(true);

        ui->comboDoctors->setVisible(true);
        ui->label_doctor->setVisible(true);
        ui->btnEditDoctor->setVisible(true);

        ui->hidePricesTotal->setVisible(true);
        ui->cashPayment->setVisible(true);

    } else if (ui->comboTypeReport->currentText() == "Volumul investigatiilor"){

        ui->comboOrganizations->setVisible(true);
        ui->label_organization->setVisible(true);
        ui->btnEditOrganization->setVisible(true);

        ui->comboContracts->setVisible(true);
        ui->label_contract->setVisible(true);
        ui->btnEditContract->setVisible(true);

        ui->comboDoctors->setVisible(false);
        ui->label_doctor->setVisible(false);
        ui->btnEditDoctor->setVisible(false);

        ui->hidePricesTotal->setVisible(false);
        ui->cashPayment->setVisible(false);

    } else if (ui->comboTypeReport->currentText() == "Structura patologiilor"){

        ui->comboOrganizations->setVisible(false);
        ui->label_organization->setVisible(false);
        ui->btnEditOrganization->setVisible(false);

        ui->comboContracts->setVisible(false);
        ui->label_contract->setVisible(false);
        ui->btnEditContract->setVisible(false);

        ui->comboDoctors->setVisible(false);
        ui->label_doctor->setVisible(false);
        ui->btnEditDoctor->setVisible(false);

        ui->hidePricesTotal->setVisible(false);
        ui->cashPayment->setVisible(false);

    } else if (ui->comboTypeReport->currentText() == "Volumul investigatiilor per cod si institutie"){

        ui->comboOrganizations->setVisible(true);
        ui->label_organization->setVisible(true);
        ui->btnEditOrganization->setVisible(true);

        ui->comboContracts->setVisible(true);
        ui->label_contract->setVisible(true);
        ui->btnEditContract->setVisible(true);

        ui->comboDoctors->setVisible(true);
        ui->label_doctor->setVisible(true);
        ui->btnEditDoctor->setVisible(true);

        ui->hidePricesTotal->setVisible(false);
        ui->cashPayment->setVisible(false);

    } else {

        ui->comboOrganizations->setVisible(false);
        ui->label_organization->setVisible(false);
        ui->btnEditOrganization->setVisible(false);

        ui->comboContracts->setVisible(false);
        ui->label_contract->setVisible(false);
        ui->btnEditContract->setVisible(false);

        ui->comboDoctors->setVisible(false);
        ui->label_doctor->setVisible(false);
        ui->btnEditDoctor->setVisible(false);

        ui->hidePricesTotal->setVisible(false);
        ui->cashPayment->setVisible(false);
    }

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
        m_emailTo = items.constFind("email").value();
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

void Reports::sendReportToEmail()
{
    // ---------------------------------------------------------------------
    // 📌 1 generam raportul cu export in PDF (tmp/USG)
    send_email = true;
    generateReport();

    // ---------------------------------------------------------------------
    // 📌 2 prezentam agentul
    AgentSendEmail *agent_sendEmail = new AgentSendEmail(this);
    agent_sendEmail->setAttribute(Qt::WA_DeleteOnClose);
    agent_sendEmail->setProperty("ThisReports", true);
    agent_sendEmail->setProperty("NameReport",  ui->comboTypeReport->currentText());
    agent_sendEmail->setProperty("EmailFrom",   globals().main_email_organization);
    agent_sendEmail->setProperty("EmailTo",     m_emailTo);
    agent_sendEmail->setProperty("NamePatient", ui->comboOrganizations->currentText());
    agent_sendEmail->setProperty("NameDoctor",  globals().main_name_abbreviat_doctor);
    agent_sendEmail->setProperty("DateInvestigation", QVariant());
    agent_sendEmail->show();

    // ---------------------------------------------------------------------
    // 📌 3 schimbam valoarea variabilei
    send_email = false;
}

// **********************************************************************************
// --- procesarea functiilor LimeReport

void Reports::slotGetCallbackDataDoctor(LimeReport::CallbackInfo info, QVariant &data)
{
    if (! list_doctor)
        return;
    prepareData(list_doctor, info, data);
}

void Reports::slotGetCallbackDataInvestigation(LimeReport::CallbackInfo info, QVariant &data)
{
    if (! list_investigation)
        return;
    prepareData(list_investigation, info, data);
}

void Reports::prepareData(QSqlQuery *qry, LimeReport::CallbackInfo info, QVariant &data)
{
    switch (info.dataType) {
    case LimeReport::CallbackInfo::ColumnCount:
        data = qry->record().count();
        break;
    case LimeReport::CallbackInfo::IsEmpty:
        data = ! qry->first();
        break;
    case LimeReport::CallbackInfo::HasNext:
        data = qry->next();
        qry->previous();
        break;
    case LimeReport::CallbackInfo::ColumnHeaderData:
        if (info.index < qry->record().count())
            data = qry->record().fieldName(info.index);
        break;
    case LimeReport::CallbackInfo::ColumnData:
        data = qry->value(qry->record().indexOf(info.columnName));
        break;
    default:
        break;
    }
}

void Reports::slotChangePosDoctors(const LimeReport::CallbackInfo::ChangePosType &type, bool &result)
{
    QSqlQuery *ds = list_doctor;
    if (!ds)
        return;
    if (type == LimeReport::CallbackInfo::First)
        result = ds->first();
    else
        result = ds->next();

    if (result){
        list_investigation->bindValue(0, QDateTime(ui->dateStart->date(), QTime(00,00,00)));
        list_investigation->bindValue(1, QDateTime(ui->dateEnd->date(), QTime(23,59,59)));
        list_investigation->bindValue(2, ui->comboOrganizations->itemData(ui->comboOrganizations->currentIndex(), Qt::UserRole).toInt());
        list_investigation->bindValue(3, list_doctor->value(list_doctor->record().indexOf("id")));
        if (! list_investigation->exec()) {
            qCritical() << "Error: Unable to execute investigations query (slotChangePos):" << list_investigation->lastError().text();
            return;
        }
    }
}

void Reports::slotChangePosInvestigation(const LimeReport::CallbackInfo::ChangePosType &type, bool &result)
{
    QSqlQuery *ds = list_investigation;
    if (!ds)
        return;
    if (type == LimeReport::CallbackInfo::First)
        result = ds->first();
    else
        result = ds->next();
}

// **********************************************************************************
// --- evenimente

void Reports::closeEvent(QCloseEvent *event)
{
    if (event->type() == QEvent::Close){
        ui->pageNavigator->setValue(0); // fix bug - la inchiderea -> crash qApp
        saveSettingsReport();
    }
}
