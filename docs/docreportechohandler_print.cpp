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

#include <docs/docreportechohandler_p.h>
#include <customs/custommessage.h>

constexpr QSize k_LogoSize  = {300, 50};
constexpr QSize k_StampSize = {200, 200};

struct LogoStamp
{
    int exist_logo = 0;
    int exist_stamp_doctor = 0;
    int exist_signature_doctor = 0;
    int exist_stam_organization = 0;
};

QStandardItem* mkImageItem(const QString& cacheKey,
                           const QByteArray& bytes,
                           const QSize& targetSize)
{
    QPixmap pix;
    /** încearcam din cache */
    if (! globals().cache_img.find(cacheKey, &pix)) {
        if (! bytes.isEmpty()) {
            QPixmap raw;
            if (raw.loadFromData(bytes)) {
                pix = raw.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                globals().cache_img.insert(cacheKey, pix); // cache pe varianta scalată
            }
        }
    }
    if (pix.isNull())
        return new QStandardItem; // item gol

    auto *it = new QStandardItem;
    it->setData(pix,           Qt::DecorationRole);   // pt. Qt Views (optional)
    it->setData(pix.toImage(), Qt::DisplayRole);      // pt. LimeReport (de obicei citește DisplayRole)
    return it;
}

void showCustomMsg(DocReportEcho*o, const QString details)
{
    CustomMessage *msgBox = new CustomMessage(o);
    msgBox->setWindowTitle(QObject::tr("Printarea documentului"));
    msgBox->setTextTitle(QObject::tr("Printarea documentului nu este posibila."));
    msgBox->setDetailedText(details);
    msgBox->exec();
    msgBox->deleteLater();
}

// *******************************************************************
// **************** FUNCTIILE STATICE ********************************

static void setModelLogoStamp(QStandardItemModel* model_img, LogoStamp& s_logo_stamp)
{
    // 1. logo
    const QString keyLogo = QStringLiteral("logo_%1").arg(globals().nameUserApp);
    QStandardItem* itLogo = mkImageItem(keyLogo, globals().c_logo_byteArray, k_LogoSize);
    s_logo_stamp.exist_logo = itLogo->data(Qt::DisplayRole).isValid() ? 1 : 0;

    // 2. ștampila organizației
    const QString keyOrg = QStringLiteral("stamp_organization_id-%1_%2")
                               .arg(globals().c_id_organizations).arg(globals().nameUserApp);
    QStandardItem* itOrg = mkImageItem(keyOrg, globals().main_stamp_organization, k_StampSize);
    s_logo_stamp.exist_stam_organization = itOrg->data(Qt::DisplayRole).isValid() ? 1 : 0;

    // 3. semnătura doctorului
    const QString keySig = QStringLiteral("signature_doctor_id-%1_%2")
                               .arg(globals().c_id_doctor).arg(globals().nameUserApp);
    QStandardItem* itSig = mkImageItem(keySig, globals().signature_main_doctor, k_StampSize);
    s_logo_stamp.exist_signature_doctor = itSig->data(Qt::DisplayRole).isValid() ? 1 : 0;

    // 4. ștampila doctorului
    const QString keyDoc = QStringLiteral("stamp_doctor_id-%1_%2")
                               .arg(globals().c_id_doctor).arg(globals().nameUserApp);
    QStandardItem* itDoc = mkImageItem(keyDoc, globals().stamp_main_doctor, k_StampSize);
    s_logo_stamp.exist_stamp_doctor = itDoc->data(Qt::DisplayRole).isValid() ? 1 : 0;

    // set în model (o singură linie, 4 coloane)
    if (! model_img)
        return;
    model_img->clear();
    model_img->setColumnCount(4);

    QList<QStandardItem*> row;
    row.reserve(4);
    row << itLogo << itOrg << itSig << itDoc;
    model_img->appendRow(row);
}

static void setModelPatient(DocReportEcho*o, DataBase *db, QSqlQueryModel* model_patient)
{
    if (model_patient->rowCount() > 0)
        model_patient->clear();
    model_patient->setQuery(db->getQryFromTablePatientById(o->getIdPacient()));
}

static void setModelOrganization(DataBase *db, QSqlQueryModel* model_organization)
{
    if (model_organization->rowCount() > 0)
        model_organization->clear();
    model_organization->setQuery(db->getQryFromTableConstantById(globals().idUserApp));
}

static void showTemplateComplex(DocReportEcho*o, Ui::DocReportEcho* ui, LimeReport::ReportEngine *m_report, DataBase *db,
                                QSqlQueryModel* model_organsInternal, QSqlQueryModel* model_urinarySystem,
                                Enums::TYPE_PRINT typeReport, QString &filePDF)
{
    /** extragem datele, setam model + setam variabile */
    model_organsInternal->setQuery(db->getQryForTableOrgansInternalById(o->getId()));
    model_urinarySystem->setQuery(db->getQryForTableUrinarySystemById(o->getId()));
    m_report->dataManager()->addModel("table_organs_internal", model_organsInternal, false);
    m_report->dataManager()->addModel("table_urinary_system", model_urinarySystem, false);
    m_report->dataManager()->setReportVariable("unit_measure_volum", "ml");

    /** completam sablonul */
    if (! m_report->loadFromFile(globals().pathTemplatesDocs + "/Complex.lrxml")){
        showCustomMsg(o, QObject::tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/Complex.lrxml"));
        return;
    }

    /** ascundem dialogul */
    o->hide();

    /** prezentarea preview, designer sua exportam in PDF */
    m_report->setShowProgressDialog(true);
    if (typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
        qInfo(logInfo()) << QObject::tr("Printare (designer) - complex: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    } else if (typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW){
        qInfo(logInfo()) << QObject::tr("Printare (preview) - complex: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->previewReport();
    } else if (typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
        qInfo(logInfo()) << QObject::tr("Printare (export PDF) - complex: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->printToPDF(filePDF + "_complex.pdf");
    } else {
        qInfo(logInfo()) << QObject::tr("Printare (designer) - complex: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    }
}

static void showTemplateOrgansInternal(DocReportEcho*o, Ui::DocReportEcho* ui, LimeReport::ReportEngine *m_report, DataBase *db,
                                       QSqlQueryModel* model_organsInternal, Enums::TYPE_PRINT typeReport, QString &filePDF)
{
    /** extragem datele, setam model + setam variabile */
    m_report->dataManager()->setReportVariable("unit_measure_volum", "ml");
    model_organsInternal->setQuery(db->getQryForTableOrgansInternalById(o->getId()));
    m_report->dataManager()->addModel("table_organs_internal", model_organsInternal, false);

    /** completam sablonul */
    if (! m_report->loadFromFile(globals().pathTemplatesDocs + "/Organs internal.lrxml")){
        showCustomMsg(o, QObject::tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/Organs internal.lrxml"));
        return;
    }

    /** ascundem dialogul */
    o->hide();

    /** prezentarea preview, designer sua exportam in PDF */
    m_report->setShowProgressDialog(true);
    if (typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
        qInfo(logInfo()) << QObject::tr("Printare (designer) - organs internal: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    } else if (typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW){
        qInfo(logInfo()) << QObject::tr("Printare (preview) - organs internal: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->previewReport();
    } else if (typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
        qInfo(logInfo()) << QObject::tr("Printare (export PDF) - organs internal: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->printToPDF(filePDF + "organs_internal.pdf");
    } else {
        qInfo(logInfo()) << QObject::tr("Printare (designer) - organs internal: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    }
}

static void showTemplateUrinarySystem(DocReportEcho*o, Ui::DocReportEcho* ui, LimeReport::ReportEngine *m_report, DataBase *db,
                                      QSqlQueryModel* model_urinarySystem, Enums::TYPE_PRINT typeReport, QString &filePDF)
{
    /** extragem datele, setam model + setam variabile */
    m_report->dataManager()->setReportVariable("unit_measure_volum", "ml");
    model_urinarySystem->setQuery(db->getQryForTableUrinarySystemById(o->getId()));
    m_report->dataManager()->addModel("table_urinary_system", model_urinarySystem, false);

    /** completam sablonul */
    if (! m_report->loadFromFile(globals().pathTemplatesDocs + "/Urinary system.lrxml")){
        showCustomMsg(o, QObject::tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/Urinary system.lrxml"));
        return;
    }

    /** ascundem dialogul */
    o->hide();

    /** prezentarea preview, designer sua exportam in PDF */
    m_report->setShowProgressDialog(true);
    if (typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
        qInfo(logInfo()) << QObject::tr("Printare (designer) - urinary system: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    } else if (typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW){
        qInfo(logInfo()) << QObject::tr("Printare (preview) - urinary system: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->previewReport();
    } else if (typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
        qInfo(logInfo()) << QObject::tr("Printare (export PDF) - urinary system: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->printToPDF(filePDF + "_urinary_sistem.pdf");
    } else {
        qInfo(logInfo()) << QObject::tr("Printare (designer) - urinary system: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    }
}

static void showTemplateProstate(DocReportEcho*o, Ui::DocReportEcho* ui, LimeReport::ReportEngine *m_report, DataBase *db,
                                 QSqlQueryModel* model_prostate, Enums::TYPE_PRINT typeReport, QString &filePDF)
{
    /** extragem datele, setam model + setam variabile */
    m_report->dataManager()->setReportVariable("method_examination", (ui->prostate_radioBtn_transrectal->isChecked()) ? "transrectal" : "transabdominal");
    m_report->dataManager()->setReportVariable("unit_measure_volum", "cm3");
    model_prostate->setQuery(db->getQryForTableProstateById(o->getId()));
    m_report->dataManager()->addModel("table_prostate", model_prostate, false);

    /** completam sablonul */
    if (! m_report->loadFromFile(globals().pathTemplatesDocs + "/Prostate.lrxml")){
        showCustomMsg(o, QObject::tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/Prostate.lrxml"));
        return;
    }

    /** ascundem dialogul */
    o->hide();

    /** prezentarea preview, designer sua exportam in PDF */
    m_report->setShowProgressDialog(true);
    if (typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
        qInfo(logInfo()) << QObject::tr("Printare (designer) - prostate: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    } else if (typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW){
        qInfo(logInfo()) << QObject::tr("Printare (preview) - prostate: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->previewReport();
    } else if (typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
        qInfo(logInfo()) << QObject::tr("Printare (export PDF) - prostate: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->printToPDF(filePDF + "_prostate.pdf");
    } else {
        qInfo(logInfo()) << QObject::tr("Printare (designer) - prostate: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    }
}

static void showTemplateGynecology(DocReportEcho*o, Ui::DocReportEcho* ui, LimeReport::ReportEngine *m_report, DataBase *db,
                                   QSqlQueryModel* model_gynecology, Enums::TYPE_PRINT typeReport, QString &filePDF)
{
    /** extragem datele, setam model + setam variabile */
    m_report->dataManager()->setReportVariable("method_examination", (ui->gynecology_btn_transvaginal->isChecked()) ? "transvaginal" : "transabdominal");
    m_report->dataManager()->setReportVariable("unit_measure_volum", "cm3");
    model_gynecology->setQuery(db->getQryForTableGynecologyById(o->getId()));
    m_report->dataManager()->addModel("table_gynecology", model_gynecology, false);

    /** completam sablonul */
    if (! m_report->loadFromFile(globals().pathTemplatesDocs + "/Gynecology.lrxml")){
        showCustomMsg(o, QObject::tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/Gynecology.lrxml"));
        return;
    }

    /** ascundem dialogul */
    o->hide();

    /** prezentarea preview, designer sua exportam in PDF */
    m_report->setShowProgressDialog(true);
    if (typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
        qInfo(logInfo()) << QObject::tr("Printare (designer) - gynecology: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    } else if (typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW){
        qInfo(logInfo()) << QObject::tr("Printare (preview) - gynecology: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->previewReport();
    } else if (typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
        qInfo(logInfo()) << QObject::tr("Printare (export PDF) - gynecology: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->printToPDF(filePDF + "_gynecology.pdf");
    } else {
        qInfo(logInfo()) << QObject::tr("Printare (designer) - gynecology: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    }
}

static void showTemplateBreast(DocReportEcho*o, Ui::DocReportEcho* ui, LimeReport::ReportEngine *m_report, DataBase *db,
                               QSqlQueryModel* model_breast, Enums::TYPE_PRINT typeReport, QString &filePDF)
{
    /** extragem datele, setam model + setam variabile */
    model_breast->setQuery(db->getQryForTableBreastById(o->getId()));
    m_report->dataManager()->addModel("table_breast", model_breast, false);

    /** completam sablonul */
    if (! m_report->loadFromFile(globals().pathTemplatesDocs + "/Breast.lrxml")){
        showCustomMsg(o, QObject::tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/Breast.lrxml"));
        return;
    }

    /** ascundem dialogul */
    o->hide();

    /** prezentarea preview, designer sua exportam in PDF */
    m_report->setShowProgressDialog(true);
    if (typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
        qInfo(logInfo()) << QObject::tr("Printare (designer) - breast: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    } else if (typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW){
        qInfo(logInfo()) << QObject::tr("Printare (preview) - breast: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->previewReport();
    } else if (typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
        qInfo(logInfo()) << QObject::tr("Printare (export PDF) - breast: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->printToPDF(filePDF + "_breast.pdf");
    } else {
        qInfo(logInfo()) << QObject::tr("Printare (designer) - breast: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    }
}

static void showTemplateThyroid(DocReportEcho*o, Ui::DocReportEcho* ui, LimeReport::ReportEngine *m_report, DataBase *db,
                                QSqlQueryModel* model_thyroid, Enums::TYPE_PRINT typeReport, QString &filePDF)
{
    /** extragem datele, setam model + setam variabile */
    m_report->dataManager()->setReportVariable("unit_measure_volum", "cm3");
    model_thyroid->setQuery(db->getQryForTableThyroidById(o->getId()));
    m_report->dataManager()->addModel("table_thyroid", model_thyroid, false);

    /** completam sablonul */
    if (! m_report->loadFromFile(globals().pathTemplatesDocs + "/Thyroid.lrxml")){
        showCustomMsg(o, QObject::tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/Thyroid.lrxml"));
        return;
    }

    /** ascundem dialogul */
    o->hide();

    /** prezentarea preview, designer sua exportam in PDF */
    m_report->setShowProgressDialog(true);
    if (typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
        qInfo(logInfo()) << QObject::tr("Printare (designer) - thyroid: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    } else if (typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW){
        qInfo(logInfo()) << QObject::tr("Printare (preview) - thyroid: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->previewReport();
    } else if (typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
        qInfo(logInfo()) << QObject::tr("Printare (export PDF) - thyroid: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->printToPDF(filePDF + "_thyroid.pdf");
    } else {
        qInfo(logInfo()) << QObject::tr("Printare (designer) - thyroid: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    }
}

static void showTemplateGestation0(DocReportEcho*o, Ui::DocReportEcho* ui, LimeReport::ReportEngine *m_report, DataBase *db,
                                   QSqlQueryModel* model_gestation0, Enums::TYPE_PRINT typeReport, QString &filePDF)
{
    m_report->dataManager()->setReportVariable("v_lmp", ui->gestation0_LMP->date().toString("dd.MM.yyyy"));
    m_report->dataManager()->setReportVariable("v_probable_date_birth", ui->gestation0_probableDateBirth->date().toString("dd.MM.yyyy"));

    //** extragem datele, setam model + setam variabile */
    m_report->dataManager()->setReportVariable("ivestigation_view", o->getViewExaminationGestation(0));
    model_gestation0->setQuery(db->getQryForTableGestation0dById(o->getId()));
    m_report->dataManager()->addModel("table_gestation0", model_gestation0, false);

    /** completam sablonul */
    if (! m_report->loadFromFile(globals().pathTemplatesDocs + "/Gestation0.lrxml")){
        showCustomMsg(o, QObject::tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/Gestation0.lrxml"));
        return;
    }

    /** ascundem dialogul */
    o->hide();

    /** prezentarea preview, designer sua exportam in PDF */
    m_report->setShowProgressDialog(true);
    if (typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
        qInfo(logInfo()) << QObject::tr("Printare (designer) - gestation0: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    } else if (typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW){
        qInfo(logInfo()) << QObject::tr("Printare (preview) - gestation0: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->previewReport();
    } else if (typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
        qInfo(logInfo()) << QObject::tr("Printare (export PDF) - gestation0: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->printToPDF(filePDF + "_gestation0.pdf");
    } else {
        qInfo(logInfo()) << QObject::tr("Printare (designer) - gestation0: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    }
}

static void showTemplateGestation1(DocReportEcho*o, Ui::DocReportEcho* ui, LimeReport::ReportEngine *m_report, DataBase *db,
                                   QSqlQueryModel* model_gestation1, Enums::TYPE_PRINT typeReport, QString &filePDF)
{
    //** extragem datele, setam model + setam variabile */
    m_report->dataManager()->setReportVariable("v_lmp", ui->gestation1_LMP->date().toString("dd.MM.yyyy"));
    m_report->dataManager()->setReportVariable("v_probable_date_birth", ui->gestation1_probableDateBirth->date().toString("dd.MM.yyyy"));
    m_report->dataManager()->setReportVariable("ivestigation_view", o->getViewExaminationGestation(1));
    model_gestation1->setQuery(db->getQryForTableGestation1dById(o->getId()));
    m_report->dataManager()->addModel("table_gestation1", model_gestation1, false);

    /** completam sablonul */
    if (! m_report->loadFromFile(globals().pathTemplatesDocs + "/Gestation1.lrxml")){
        showCustomMsg(o, QObject::tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/Gestation1.lrxml"));
        return;
    }

    /** ascundem dialogul */
    o->hide();

    /** prezentarea preview, designer sua exportam in PDF */
    m_report->setShowProgressDialog(true);
    if (typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
        qInfo(logInfo()) << QObject::tr("Printare (designer) - gestation1: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    } else if (typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW){
        qInfo(logInfo()) << QObject::tr("Printare (preview) - gestation1: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->previewReport();
    } else if (typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
        qInfo(logInfo()) << QObject::tr("Printare (export PDF) - gestation1: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->printToPDF(filePDF + "_gestation1.pdf");
    } else {
        qInfo(logInfo()) << QObject::tr("Printare (designer) - gestation1: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    }
}

static void showTemplateGestation2(DocReportEcho*o, Ui::DocReportEcho* ui, LimeReport::ReportEngine *m_report, DataBase *db,
                                   QSqlQueryModel* model_gestation2, Enums::TYPE_PRINT typeReport, QString &filePDF)
{
    //** extragem datele, setam model + setam variabile */
    m_report->dataManager()->setReportVariable("v_lmp", ui->gestation2_dateMenstruation->date().toString("dd.MM.yyyy"));
    m_report->dataManager()->setReportVariable("v_probable_date_birth", ui->gestation2_probabilDateBirth->date().toString("dd.MM.yyyy"));
    model_gestation2->setQuery(db->getQryForTableGestation2(o->getId()));
    m_report->dataManager()->addModel("table_gestation2", model_gestation2, false);

    /** completam sablonul */
    if (! m_report->loadFromFile((ui->gestation2_trimestru2->isChecked())
                                  ? globals().pathTemplatesDocs + "/Gestation2.lrxml"
                                  : globals().pathTemplatesDocs + "/Gestation3.lrxml")){
        showCustomMsg(o, QObject::tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/Gestation2.lrxml"));
        return;
    }

    /** ascundem dialogul */
    o->hide();

    /** prezentarea preview, designer sua exportam in PDF */
    m_report->setShowProgressDialog(true);
    if (typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
        qInfo(logInfo()) << QObject::tr("Printare (designer) - gestation2: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    } else if (typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW) {
        qInfo(logInfo()) << QObject::tr("Printare (preview) - gestation2: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->previewReport();
    } else if (typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
        qInfo(logInfo()) << QObject::tr("Printare (export PDF) - gestation2: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->printToPDF(filePDF + "_gestation2.pdf");
    } else {
        qInfo(logInfo()) << QObject::tr("Printare (designer) - gestation2: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    }
}

static void showTemplateLymphNodes(DocReportEcho*o, Ui::DocReportEcho* ui, LimeReport::ReportEngine *m_report, DataBase *db,
                                   QSqlQueryModel* model_lymphNodes, Enums::TYPE_PRINT typeReport, QString &filePDF)
{
    //** extragem datele, setam model + setam variabile */
    QString type_investig;
    if (ui->ln_typeInvestig->currentIndex() == 1)
        type_investig = "soft_tissues";
    else if (ui->ln_typeInvestig->currentIndex() == 2)
        type_investig = "lymph_nodes";
    else
        type_investig = "tissues_nodes";

    m_report->dataManager()->setReportVariable("v_section_type_text", type_investig);
    model_lymphNodes->setQuery(db->getQryForTableLymphNodes(o->getId()));
    m_report->dataManager()->addModel("table_lymph", model_lymphNodes, false);

    /** completam sablonul */
    if (! m_report->loadFromFile(globals().pathTemplatesDocs + "/LymphNodes.lrxml")) {
        showCustomMsg(o, QObject::tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/LymphNodes.lrxml"));
        return;
    }

    /** ascundem dialogul */
    o->hide();

    /** prezentarea preview, designer sua exportam in PDF */
    m_report->setShowProgressDialog(true);
    if (typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
        qInfo(logInfo()) << QObject::tr("Printare (designer) - LymphNodes: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    } else if (typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW) {
        qInfo(logInfo()) << QObject::tr("Printare (preview) - LymphNodes: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->previewReport();
    } else if (typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
        qInfo(logInfo()) << QObject::tr("Printare (export PDF) - LymphNodes: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->printToPDF(filePDF + "_lymphNodes.pdf");
    } else {
        qInfo(logInfo()) << QObject::tr("Printare (designer) - LymphNodes: document 'Raport ecografic' nr.%1")
        .arg(ui->editDocNumber->text());
        m_report->designReport();
    }
}

// *******************************************************************
// **************** FUNCTIA PRINCIPALA *******************************

void DocReportEchoHandler::Impl::printDoc(Enums::TYPE_PRINT typeReport, QString &filePDF,
                                          LimeReport::ReportEngine *m_report, DataBase *db)
{
    /** 1. verificam daca e indicat drum spre sabloane de tipar */
    if (globals().pathTemplatesDocs.isEmpty()){

        QMessageBox::warning(&o,
                             QObject::tr("Verificarea set\304\203rilor"),
                             QObject::tr("Nu este indicat directoriul cu \310\231abloanele de tipar.<br>"
                                "Tip\304\203rirea documentului nu este posibil\304\203."),
                             QMessageBox::Ok);
        return;
    }

    /** 2. determinam UI */
    auto ui = o.uiPtr();

    /** 3. actualizam datele flag-lor */
    DocReportEcho::SectionsSystem s = o.getSectionsSystem();

    /** 4. alocam memoria */
    m_report = new LimeReport::ReportEngine(&o);
    auto model_img          = new QStandardItemModel();
    auto model_patient      = new QSqlQueryModel(m_report);
    auto model_organization = new QSqlQueryModel(m_report);

    /** 5. titlu raportului - dupa ce am alocat memoria pu 'm_report', alfel - crash */
    m_report->setPreviewWindowTitle(QObject::tr("Raport ecografic nr.") +
                                    ui->editDocNumber->text() + QObject::tr(" din ") +
                                    ui->editDocDate->dateTime().toString("dd.MM.yyyy hh:mm:ss") +
                                    QObject::tr(" (printare)"));

    /** 6. setam modelurile */
    LogoStamp s_logo_stamp;
    setModelLogoStamp(model_img, s_logo_stamp);
    setModelPatient(&o, db, model_patient);
    setModelOrganization(db, model_organization);

    /** 7. transmitem modelurile generatorului de rapoarte */
    m_report->dataManager()->addModel("table_logo", model_img, true);
    m_report->dataManager()->addModel("main_organization", model_organization, false);
    m_report->dataManager()->addModel("table_patient", model_patient, false);

    /** 8. setam variabile necesare */
    m_report->dataManager()->clearUserVariables();
    m_report->dataManager()->setReportVariable("v_exist_logo", s_logo_stamp.exist_logo);
    m_report->dataManager()->setReportVariable("v_export_pdf", (typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF) ? 1 : 0);
    m_report->dataManager()->setReportVariable("v_exist_stam_organization", s_logo_stamp.exist_stam_organization);
    m_report->dataManager()->setReportVariable("v_exist_stamp_doctor", s_logo_stamp.exist_stamp_doctor);
    m_report->dataManager()->setReportVariable("v_exist_signature_doctor", s_logo_stamp.exist_signature_doctor);
    m_report->dataManager()->setReportVariable("unitMeasure", (globals().unitMeasure == "milimetru") ? "mm" : "cm");
    QString str_recmmand = o.getAllRecommandation().join(", ");
    m_report->dataManager()->setReportVariable("all_recomandation", str_recmmand);

    /** 9. prezentam forma de tipar dupa sisteme */
    if ((s & DocReportEcho::OrgansInternal) && (s & DocReportEcho::UrinarySystem)) { // complex
        auto model_organsInternal = new QSqlQueryModel(m_report);
        auto model_urinarySystem  = new QSqlQueryModel(m_report);
        showTemplateComplex(&o, ui, m_report, db, model_organsInternal, model_urinarySystem, typeReport, filePDF);
    }
    if ((s & DocReportEcho::OrgansInternal) && ! (s & DocReportEcho::UrinarySystem)) { // organs internal
        auto model_organsInternal = new QSqlQueryModel(m_report);
        showTemplateOrgansInternal(&o, ui, m_report, db, model_organsInternal, typeReport, filePDF);
    }
    if (! (s & DocReportEcho::OrgansInternal) && (s & DocReportEcho::UrinarySystem)) { // urinary system
        auto model_urinarySystem  = new QSqlQueryModel(m_report);
        showTemplateUrinarySystem(&o, ui, m_report, db, model_urinarySystem, typeReport, filePDF);
    }
    if (s & DocReportEcho::Prostate) {
        auto model_prostate = new QSqlQueryModel(m_report);
        showTemplateProstate(&o, ui, m_report, db, model_prostate,  typeReport, filePDF);
    }
    if (s & DocReportEcho::Gynecology) {
        auto model_gynecology = new QSqlQueryModel(m_report);
        showTemplateGynecology(&o, ui, m_report, db, model_gynecology,  typeReport, filePDF);
    }
    if (s & DocReportEcho::Breast) {
        auto model_breast = new QSqlQueryModel(m_report);
        showTemplateBreast(&o, ui, m_report, db, model_breast,  typeReport, filePDF);
    }
    if (s & DocReportEcho::Thyroid) {
        auto model_thyroid = new QSqlQueryModel(m_report);
        showTemplateThyroid(&o, ui, m_report, db, model_thyroid,  typeReport, filePDF);
    }
    if (s & DocReportEcho::Gestation0) {
        auto model_gestation0 = new QSqlQueryModel(m_report);
        showTemplateGestation0(&o, ui, m_report, db, model_gestation0,  typeReport, filePDF);
    }
    if (s & DocReportEcho::Gestation1) {
        auto model_gestation1 = new QSqlQueryModel(m_report);
        showTemplateGestation1(&o, ui, m_report, db, model_gestation1,  typeReport, filePDF);
    }
    if (s & DocReportEcho::Gestation2) {
        auto model_gestation2 = new QSqlQueryModel(m_report);
        showTemplateGestation2(&o, ui, m_report, db, model_gestation2,  typeReport, filePDF);
    }
    if (s & DocReportEcho::LymphNodes) {
        auto model_lymphNodes = new QSqlQueryModel(m_report);
        showTemplateLymphNodes(&o, ui, m_report, db, model_lymphNodes, typeReport, filePDF);
    }

    /** 10. eliberam memoria */
    m_report->deleteLater(); // se destrug toate modele
}
