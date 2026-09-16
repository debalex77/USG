#include "reportpagegestation2.h"
#include "ui_reportpagegestation2.h"

#include <QMessageBox>

#include <customs/custommessage.h>

#include <views/catalogtableeditor.h>

ReportPageGestation2::ReportPageGestation2(DataBase &db,
                                           QSqlDatabase &currentDB,
                                           QWidget *parent)
    : ReportPageBase(parent)
    , ui(new Ui::ReportPageGestation2)
    , m_db(db)
    , m_currentDB(currentDB)
    , refRanges(new FetalReferenceRanges(m_db, this))
    , toolButtonStyleForText(m_db.toolButtonStyleForText())
{
    ui->setupUi(this);

    initRequiredStructure();
    initInstallEventFilter();

    setDefaultContext();
    setPropertyMaxLengthText();
    initConnections();
}

ReportPageGestation2::~ReportPageGestation2()
{
    delete ui;
}

bool ReportPageGestation2::loadData(int idReport)
{
    if (idReport <= 0)
        return false;

    QSqlQuery q(m_currentDB);
    if (!q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_ges2_select.sql"))) {
        qWarning(logWarning()) << "ReportPageGestation2 prepare error:" << q.lastError().text();
        return false;
    }
    q.bindValue(":id_reports", idReport);
    if(!q.exec()) {
        qWarning(logWarning()).noquote()
        << "ReportPageGestation2 error:" << q.lastError().text()
        << "\nLast query:" << q.lastQuery();
        return false;
    }

    if (!q.next()) {
        qWarning(logWarning()) << "Nu s-au gasit datele pentru sistema 'gestation2' !!!";
        return false;
    }

    // QDateEdit
    const QList<QDateEdit*> des = this->findChildren<QDateEdit*>();
    std::vector<QSignalBlocker> desBlocker;
    desBlocker.reserve(des.size());
    for (QDateEdit *de : des)
        desBlocker.emplace_back(de);

    // QComboBox
    const QList<QComboBox*> combos = this->findChildren<QComboBox*>();
    std::vector<QSignalBlocker> itemsCombos;
    itemsCombos.reserve(combos.size());
    for (QComboBox *combo: combos)
        itemsCombos.emplace_back(combo);

    // QLineEdit
    const QList<QLineEdit*> items = this->findChildren<QLineEdit*>();
    std::vector<QSignalBlocker> itemsBlockers;
    itemsBlockers.reserve(items.size());
    for (QLineEdit *item : items)
        itemsBlockers.emplace_back(item);

    // QPlainTextEdit
    const QList<QPlainTextEdit*> itemsText = this->findChildren<QPlainTextEdit*>();
    std::vector<QSignalBlocker> itemsTextBlockers;
    itemsTextBlockers.reserve(itemsText.size());
    for (QPlainTextEdit *text : itemsText)
        itemsTextBlockers.emplace_back(text);

    ui->gestation2_dateMenstruation->setDate(QDate::fromString(q.value("dateMenstruation").toString(), "yyyy-MM-dd"));
    ui->gestation2_gestation_age->setText(q.value("gestation_age").toString());
    ui->gestation2_view_examination->setCurrentIndex(q.value("view_examination").toInt());
    ui->comboFetalPrezentation->setCurrentIndex(q.value("fetalPrezentation").toInt());
    ui->comboMultiplePregnancy->setCurrentIndex(q.value("multiplePregnancy").toInt());
    if (q.value("trimestru").toInt() == 2){
        ui->gestation2_trimestru2->setChecked(true);
        setGestation2ItemsEnabled(true);
    } else {
        ui->gestation2_trimestru3->setChecked(true);
        setGestation2ItemsEnabled(false);
    }
    ui->gestation2_pregnancy->setCurrentIndex(q.value("single_multiple_pregnancy").toInt());
    ui->gestation2_pregnancy_description->setText(q.value("single_multiple_pregnancy_description").toString());
    ui->gestation2_comment->setPlainText(q.value("comment").toString());
    ui->gestation2_concluzion->setPlainText(q.value("concluzion").toString());
    ui->gestation2_recommendation->setText(q.value("recommendation").toString());

    // tableGestation2_biometry
    ui->gestation2_bpd->setText(q.value("BPD").toString());
    ui->gestation2_bpd_age->setText(q.value("BPD_age").toString());
    ui->gestation2_hc->setText(q.value("HC").toString());
    ui->gestation2_hc_age->setText(q.value("HC_age").toString());
    ui->gestation2_ac->setText(q.value("AC").toString());
    ui->gestation2_ac_age->setText(q.value("AC_age").toString());
    ui->gestation2_fl->setText(q.value("FL").toString());
    ui->gestation2_fl_age->setText(q.value("FL_age").toString());
    ui->gestation2_fetus_age->setText(q.value("FetusCorresponds").toString());
    setDueDateGestation2();

    // tableGestation2_cranium
    ui->gestation2_calloteCranium->setCurrentIndex(q.value("calloteCranium").toInt());
    ui->gestation2_facialeProfile->setCurrentIndex(q.value("facialeProfile").toInt());
    ui->gestation2_nasalBones->setCurrentIndex(q.value("nasalBones").toInt());
    ui->gestation2_nasalBones_dimens->setText(q.value("nasalBones_dimens").toString());
    ui->gestation2_eyeball->setCurrentIndex(q.value("eyeball").toInt());
    ui->gestation2_eyeball_desciption->setText(q.value("eyeball_desciption").toString());
    ui->gestation2_nasolabialTriangle->setCurrentIndex(q.value("nasolabialTriangle").toInt());
    ui->gestation2_nasolabialTriangle_description->setText(q.value("nasolabialTriangle_description").toString());
    ui->gestation2_nasalFold->setText(q.value("nasalFold").toString());

    // tableGestation2_SNC
    ui->gestation2_hemispheres->setCurrentIndex(q.value("hemispheres").toInt());
    ui->gestation2_fissureSilvius->setCurrentIndex(q.value("fissureSilvius").toInt());
    ui->gestation2_corpCalos->setCurrentIndex(q.value("corpCalos").toInt());
    ui->gestation2_ventricularSystem->setCurrentIndex(q.value("ventricularSystem").toInt());
    ui->gestation2_ventricularSystem_description->setText(q.value("ventricularSystem_description").toString());
    ui->gestation2_cavityPellucidSeptum->setCurrentIndex(q.value("cavityPellucidSeptum").toInt());
    ui->gestation2_choroidalPlex->setCurrentIndex(q.value("choroidalPlex").toInt());
    ui->gestation2_choroidalPlex_description->setText(q.value("choroidalPlex_description").toString());
    ui->gestation2_cerebellum->setCurrentIndex(q.value("cerebellum").toInt());
    ui->gestation2_cerebellum_description->setText(q.value("cerebellum_description").toString());
    ui->gestation2_vertebralColumn->setCurrentIndex(q.value("vertebralColumn").toInt());
    ui->gestation2_vertebralColumn_description->setText(q.value("vertebralColumn_description").toString());

    // tableGestation2_heart
    ui->gestation2_heartPosition->setText(q.value("position").toString());
    ui->gestation2_heartBeat->setCurrentIndex(q.value("heartBeat").toInt());
    ui->gestation2_heartBeat_frequency->setText(q.value("heartBeat_frequency").toString());
    ui->gestation2_heartBeat_rhythm->setCurrentIndex(q.value("heartBeat_rhythm").toInt());
    ui->gestation2_pericordialCollections->setCurrentIndex(q.value("pericordialCollections").toInt());
    ui->gestation2_planPatruCamere->setCurrentIndex(q.value("planPatruCamere").toInt());
    ui->gestation2_planPatruCamere_description->setText(q.value("planPatruCamere_description").toString());
    ui->gestation2_ventricularEjectionPathLeft->setCurrentIndex(q.value("ventricularEjectionPathLeft").toInt());
    ui->gestation2_ventricularEjectionPathLeft_description->setText(q.value("ventricularEjectionPathLeft_description").toString());
    ui->gestation2_ventricularEjectionPathRight->setCurrentIndex(q.value("ventricularEjectionPathRight").toInt());
    ui->gestation2_ventricularEjectionPathRight_description->setText(q.value("ventricularEjectionPathRight_description").toString());
    ui->gestation2_intersectionVesselMagistral->setCurrentIndex(q.value("intersectionVesselMagistral").toInt());
    ui->gestation2_intersectionVesselMagistral_description->setText(q.value("intersectionVesselMagistral_description").toString());
    ui->gestation2_planTreiVase->setCurrentIndex(q.value("planTreiVase").toInt());
    ui->gestation2_planTreiVase_description->setText(q.value("planTreiVase_description").toString());
    ui->gestation2_archAorta->setCurrentIndex(q.value("archAorta").toInt());
    ui->gestation2_planBicav->setCurrentIndex(q.value("planBicav").toInt());

    // tableGestation2_thorax
    ui->gestation2_pulmonaryAreas->setCurrentIndex(q.value("pulmonaryAreas").toInt());
    ui->gestation2_pulmonaryAreas_description->setText(q.value("pulmonaryAreas_description").toString());
    ui->gestation2_pleuralCollections->setCurrentIndex(q.value("pleuralCollections").toInt());
    ui->gestation2_diaphragm->setCurrentIndex(q.value("diaphragm").toInt());

    // tableGestation2_abdomen
    ui->gestation2_abdominalWall->setCurrentIndex(q.value("abdominalWall").toInt());
    ui->gestation2_abdominalCollections->setCurrentIndex(q.value("abdominalCollections").toInt());
    ui->gestation2_stomach->setCurrentIndex(q.value("stomach").toInt());
    ui->gestation2_stomach_description->setText(q.value("stomach_description").toString());
    ui->gestation2_abdominalOrgans->setCurrentIndex(q.value("abdominalOrgans").toInt());
    ui->gestation2_cholecist->setCurrentIndex(q.value("cholecist").toInt());
    ui->gestation2_cholecist_description->setText(q.value("cholecist_description").toString());
    ui->gestation2_intestine->setCurrentIndex(q.value("intestine").toInt());
    ui->gestation2_intestine_description->setText(q.value("intestine_description").toString());

    // tableGestation2_urinarySystem
    ui->gestation2_kidneys->setCurrentIndex(q.value("kidneys").toInt());
    ui->gestation2_kidneys_description->setText(q.value("kidneys_descriptions").toString());
    ui->gestation2_ureter->setCurrentIndex(q.value("ureter").toInt());
    ui->gestation2_ureter_description->setText(q.value("ureter_descriptions").toString());
    ui->gestation2_bladder->setCurrentIndex(q.value("bladder").toInt());

    // tableGestation2_other
    ui->gestation2_extremities->setCurrentIndex(q.value("extremities").toInt());
    ui->gestation2_extremities_description->setText(q.value("extremities_descriptions").toString());
    ui->gestation2_placenta->setCurrentIndex(q.value("placenta").toInt());
    ui->gestation2_placenta_localization->setText(q.value("placentaLocalization").toString());
    ui->gestation2_placentaDegreeMaturation->setText(q.value("placentaDegreeMaturation").toString());
    ui->gestation2_placentaDepth->setText(q.value("placentaDepth").toString());
    ui->gestation2_placentaStructure->setCurrentIndex(q.value("placentaStructure").toInt());
    ui->gestation2_placentaStructure_description->setText(q.value("placentaStructure_descriptions").toString());
    ui->gestation2_umbilicalCordon->setCurrentIndex(q.value("umbilicalCordon").toInt());
    ui->gestation2_umbilicalCordon_description->setText(q.value("umbilicalCordon_description").toString());
    ui->gestation2_insertionPlacenta->setCurrentIndex(q.value("insertionPlacenta").toInt());
    ui->gestation2_amnioticIndex->setText(q.value("amnioticIndex").toString());
    ui->gestation2_amnioticIndexAspect->setCurrentIndex(q.value("amnioticIndexAspect").toInt());
    ui->gestation2_amnioticBedDepth->setText(q.value("amnioticBedDepth").toString());
    ui->gestation2_cervix->setText(q.value("cervix").toString());
    ui->gestation2_cervix_description->setText(q.value("cervix_description").toString());
    ui->gestation2_fetusMass->setText(q.value("fetusMass").toString());
    ui->gestation2_fetusSex->setCurrentIndex(q.value("externalGenitalOrgans").toInt());
    updateDescriptionFetusWeight();

    // tableGestation2_doppler
    ui->gestation2_ombilic_PI->setText(q.value("ombilic_PI").toString());
    ui->gestation2_ombilic_RI->setText(q.value("ombilic_RI").toString());
    ui->gestation2_ombilic_SD->setText(q.value("ombilic_SD").toString());
    ui->gestation2_ombilic_flux->setCurrentIndex(q.value("ombilic_flux").toInt());
    ui->gestation2_cerebral_PI->setText(q.value("cerebral_PI").toString());
    ui->gestation2_cerebral_RI->setText(q.value("cerebral_RI").toString());
    ui->gestation2_cerebral_SD->setText(q.value("cerebral_SD").toString());
    ui->gestation2_cerebral_flux->setCurrentIndex(q.value("cerebral_flux").toInt());
    ui->gestation2_uterRight_PI->setText(q.value("uterRight_PI").toString());
    ui->gestation2_uterRight_RI->setText(q.value("uterRight_RI").toString());
    ui->gestation2_uterRight_SD->setText(q.value("uterRight_SD").toString());
    ui->gestation2_uterRight_flux->setCurrentIndex(q.value("uterRight_flux").toInt());
    ui->gestation2_uterLeft_PI->setText(q.value("uterLeft_PI").toString());
    ui->gestation2_uterLeft_RI->setText(q.value("uterLeft_RI").toString());
    ui->gestation2_uterLeft_SD->setText(q.value("uterLeft_SD").toString());
    ui->gestation2_uterLeft_flux->setCurrentIndex(q.value("uterLeft_flux").toInt());
    ui->gestation2_ductVenos->setCurrentIndex(q.value("ductVenos").toInt());
    updateTextDescriptionDoppler();

    return true;
}

bool ReportPageGestation2::saveData(int idReport)
{
    if (idReport <= 0)
        return false;

    // ReportDialog owns the transaction; each section must propagate failures.
    struct TableWrite {
        const char *table;
        const char *resource;
        void (ReportPageGestation2::*bind)(QSqlQuery &, int);
    };
    const TableWrite tables[] = {
        {"tableGestation2",               "report_ges2",          &ReportPageGestation2::bindTabGes2Fields},
        {"tableGestation2_biometry",      "report_ges2_biometry", &ReportPageGestation2::bindBiometryFields},
        {"tableGestation2_cranium",       "report_ges2_cranium",  &ReportPageGestation2::bindCraniumFields},
        {"tableGestation2_SNC",           "report_ges2_snc",      &ReportPageGestation2::bindSNCFields},
        {"tableGestation2_heart",         "report_ges2_heart",    &ReportPageGestation2::bindHeartFields},
        {"tableGestation2_thorax",        "report_ges2_thorax",   &ReportPageGestation2::bindThoraxFields},
        {"tableGestation2_abdomen",       "report_ges2_abdomen",  &ReportPageGestation2::bindAbdomenFields},
        {"tableGestation2_urinarySystem", "report_ges2_urinary",  &ReportPageGestation2::bindUrinarySystemFields},
        {"tableGestation2_other",         "report_ges2_other",    &ReportPageGestation2::bindOtherFields},
        {"tableGestation2_doppler",       "report_ges2_doppler",  &ReportPageGestation2::bindDopplerFields}
    };

    for (const auto &table : tables) {
        QSqlQuery exists(m_currentDB);
        if (!exists.prepare(QString("SELECT 1 FROM %1 WHERE id_reportEcho = :id")
                                .arg(QLatin1String(table.table)))) {
            qWarning(logWarning()) << "Gestation2 existence prepare error:"
                                   << exists.lastError().text();
            return false;
        }
        exists.bindValue(":id", idReport);
        if (!exists.exec()) {
            qWarning(logWarning()) << "Gestation2 existence query error:"
                                   << exists.lastError().text();
            return false;
        }
        const bool present = exists.next();
        exists.finish();
        const QString resource = QString(":/sql/queries_doc/%1_%2.sql")
                                     .arg(QLatin1String(table.resource),
                                          present ? QStringLiteral("update")
                                                  : QStringLiteral("insert"));
        QSqlQuery write(m_currentDB);
        if (!write.prepare(m_db.getTextSQL(resource))) {
            qWarning(logWarning()) << "Gestation2 save prepare error:"
                                   << table.table
                                   << write.lastError().text();
            return false;
        }
        (this->*table.bind)(write, idReport);
        if (!write.exec()) {
            qWarning(logWarning()) << "Gestation2 save error:"
                                   << table.table
                                   << write.lastError().text();
            return false;
        }
        qInfo(logInfo()) << "Gestation2: operație executată în tranzacția raportului; tabela="
                         << table.table << "operație=" << (present ? "UPDATE" : "INSERT")
                         << "id=" << idReport;
    }
    return true;
}

QString ReportPageGestation2::concluzionText() const
{
    return ui->gestation2_concluzion
               ? ui->gestation2_concluzion->toPlainText().trimmed()
               : QString{};
}

ReportSections::ReportSystem ReportPageGestation2::system() const
{
    return ReportSections::ReportSystem::Gestation2;
}

QDate ReportPageGestation2::LMP() const
{
    return ui->gestation2_dateMenstruation->date();
}

QDate ReportPageGestation2::probableDateBirth() const
{
    return ui->gestation2_probabilDateBirth->date();
}

bool ReportPageGestation2::isGestation2() const
{
    return ui->gestation2_trimestru2->isChecked();
}

void ReportPageGestation2::onDateLMPChanged()
{
    QString str_vg = calculateGestationalAge(ui->gestation2_dateMenstruation->date());
    if (str_vg == nullptr)
        return;
    ui->gestation2_gestation_age->setText(str_vg);

    updateGestationTrimestru(ui->gestation2_dateMenstruation->date());
}

void ReportPageGestation2::setProbabilDateBirth()
{
    int weeks, days;
    extractWeeksAndDays(ui->gestation2_fetus_age->text(), weeks, days);
    if (weeks > 0)
        ui->gestation2_probabilDateBirth->setDate(calculateDueDateFromFetalAge(weeks, days));
}

void ReportPageGestation2::updateDescriptionFetusWeight()
{
    if (ui->gestation2_fetusMass->text().isEmpty()) {
        ui->gestation2_descriptionFetusWeight->setText(tr("Întroduceți masa fătului"));
        return;
    }

    int weeks, days;
    extractWeeksAndDays(ui->gestation2_fetus_age->text(), weeks, days);
    double current_weight = ui->gestation2_fetusMass->text().toDouble();

    ReferenceInterpretationResult res;
    res = refRanges->determineFmfPercentile(FetalReferenceRanges::Type::FetalWeight,
                                            current_weight,
                                            weeks);
    if (res.ok)
        ui->gestation2_descriptionFetusWeight->setText(tr("Masa fătului ") + res.interpretation);
    else
        ui->gestation2_descriptionFetusWeight->setText(res.errorString);
}

void ReportPageGestation2::updateTextDescriptionDoppler()
{
    QString txt_umbelicalArtery    = getPercentageByDopplerUmbelicalArtery();
    QString txt_uterineArteryLeft  = getPercentageByDopplerUterineArteryLeft();
    QString txt_uterineArteryRight = getPercentageByDopplerUterineArteryRight();
    QString txt_CMA                = getPercentageByDopplerCMA();

    ui->gestation2_infoDoppler->clear();
    ui->gestation2_comment->clear();

    if (txt_umbelicalArtery != nullptr)
        ui->gestation2_infoDoppler->append(txt_umbelicalArtery);

    if (txt_uterineArteryLeft != nullptr)
        ui->gestation2_infoDoppler->append(txt_uterineArteryLeft);

    if (txt_uterineArteryRight != nullptr)
        ui->gestation2_infoDoppler->append(txt_uterineArteryRight);

    if (txt_CMA != nullptr)
        ui->gestation2_infoDoppler->append(txt_CMA);

    ui->gestation2_infoDoppler->append("Devierea de calcul cu 'Fetal Medicine Foundation' "
                                       "este de până la 0.10 percentile.");
    ui->gestation2_comment->appendPlainText(ui->gestation2_infoDoppler->toPlainText());
}

void ReportPageGestation2::handleSelectTemplate()
{
    CatalogTableEditor *dlg = new CatalogTableEditor(m_db,
                                                     CatalogType::FormType::Selection,
                                                     CatalogType::Type::ConclusionTemplates,
                                                     this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    if (ui->gestation2_trimestru2->isChecked())
        dlg->setFilterQuery("Sarcina 15-27 săptămâni");
    else
        dlg->setFilterQuery("Sarcina 27-40 săptămâni");
    connect(dlg, &CatalogTableEditor::dataSelected, this, [this](const QVariantMap &data){
        if (data.isEmpty())
            return;
        ui->gestation2_concluzion->appendPlainText(data["name"].toString());
    });
    dlg->show();
}

void ReportPageGestation2::handleAddTemplate()
{
    /** pregatim variabile locale */
    const QString m_concluzion = ui->gestation2_concluzion->toPlainText().trimmed();
    QString m_system;
    if (ui->gestation2_trimestru2->isChecked())
        m_system = "Sarcina 15-27 săptămâni";
    else
        m_system = "Sarcina 27-40 săptămâni";

    /** verificam daca sunt valorile */
    if (m_concluzion.isEmpty() || m_system.isEmpty())
        return;

    /** inseram datele */
    insertConclusionTemplate(m_concluzion, m_system);
}

void ReportPageGestation2::handleSelectFindingsTemplates()
{
    QObject* s = sender();
    if (!s)
        return;

    /** deschidem catalogul */
    CatalogTableEditor *dlg = new CatalogTableEditor(m_db,
                                                     CatalogType::FormType::Selection,
                                                     CatalogType::Type::SystemTemplates,
                                                     this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    /** ----- 1. LineEditCustom + QLineEdit --------- */
    if (auto edit = qobject_cast<LineEditCustom*>(s->parent())) {

        /** functia standart de cautare a randului dupa conditia */
        auto it = std::find_if(rows_action_findings.begin(),
                               rows_action_findings.end(),
                               [edit](const FindingsTemplatesActions &r)
                               {
                                   return r.item_edit == edit && edit->actionOpenList() == r.action_select;
                               });

        /** daca it ajuns pana la sfarsit ne oprim */
        if (it == rows_action_findings.end())
            return;

        /** pointer la obiect */
        const auto& r = *it;
        dlg->setFilterQuery(r.name_system);
        connect(dlg, &CatalogTableEditor::dataSelected, this, [dlg, r](const QVariantMap &data){
            if (data.isEmpty() || !dlg)
                return;
            r.item_edit->setText(data["name"].toString());
            dlg->close();
        });
        dlg->show();
    }
}

void ReportPageGestation2::handleAddFindingsTemplates()
{
    QObject* s = sender();
    if (!s) return;

    /** ----- 1. LineEditCustom + QLineEdit --------- */
    if (auto edit = qobject_cast<LineEditCustom*>(s->parent())) {

        /** functia standart de cautare a randului dupa conditia */
        auto it = std::find_if(rows_action_findings.begin(),
                               rows_action_findings.end(),
                               [edit](const FindingsTemplatesActions& r)
                               {
                                   return r.item_edit == edit && edit->actionAddItem() == r.action_add;
                               });

        /** daca it ajuns pana la sfarsit ne oprim */
        if (it == rows_action_findings.end())
            return;

        const auto& r = *it;                                       /** pointer la obiect */
        insertFindigsTemplate(r.item_edit->text(), r.name_system); /** inseram sablonul */
        return;
    }
}

void ReportPageGestation2::updateGestationTrimestru(const QDate &dateMenstruation)
{
    const QDate today = QDate::currentDate();

    if (!dateMenstruation.isValid() || dateMenstruation > today) {
        ui->gestation2_trimestru2->setChecked(false);
        ui->gestation2_trimestru3->setChecked(false);
        return;
    }

    const qint64 days = dateMenstruation.daysTo(today);
    const int weeks = static_cast<int>(days / 7);

    if (weeks >= 14 && weeks < 28) {
        // Trimestrul II
        ui->gestation2_trimestru2->setChecked(true);
    }
    else if (weeks >= 28) {
        // Trimestrul III
        ui->gestation2_trimestru3->setChecked(true);
    }
    else {
        // Trimestrul I
        ui->gestation2_trimestru2->setChecked(false);
        ui->gestation2_trimestru3->setChecked(false);
    }

    setGestation2ItemsEnabled(ui->gestation2_trimestru2->isChecked());
}

void ReportPageGestation2::initRequiredStructure()
{
    rows_action_findings = {
        {ui->gestation2_recommendation->actionOpenList(),
         ui->gestation2_recommendation->actionAddItem(),
         ui->gestation2_recommendation,
         "Recomandari (gestatation2)"}
    };
}

void ReportPageGestation2::initInstallEventFilter()
{
    qApp->installEventFilter(this);

    foreach (QWidget *widget, findChildren<QWidget*>()) {
        widget->installEventFilter(this);
    }

    foreach (QWidget *widget, findChildren<QWidget*>()) {
        if (qobject_cast<QLineEdit *>(widget) ||
            qobject_cast<QPlainTextEdit *>(widget) ||
            qobject_cast<QToolButton *>(widget) ||
            qobject_cast<QComboBox *>(widget)) {

            widget->setFocusPolicy(Qt::StrongFocus);
        }
        if (QPushButton *button = qobject_cast<QPushButton *>(widget)) {
            button->setAutoDefault(false);
            button->setDefault(false);
        }
    }
}

void ReportPageGestation2::setDefaultContext()
{
    ui->gestation2_dateMenstruation->setDate(QDate::currentDate());

    ui->gestation2_descriptionFetusWeight->setText(tr("Întroduceți masa fătului"));

    if (ui->gestation2_recommendation->text().isEmpty())
        ui->gestation2_recommendation->setText("consulta\310\233ia ginecologului");
}

void ReportPageGestation2::setPropertyMaxLengthText()
{
    ui->gestation2_gestation_age->setMaxLength(20);
    ui->gestation2_gestation_age->setPlaceholderText(tr("... maximum 20 caractere"));
    ui->gestation2_pregnancy_description->setMaxLength(250);
    ui->gestation2_pregnancy_description->setPlaceholderText(tr("... maximum 250 caractere"));
    ui->gestation2_eyeball_desciption->setMaxLength(100);
    ui->gestation2_eyeball_desciption->setPlaceholderText(tr("... maximum 100 caractere"));
    ui->gestation2_nasolabialTriangle_description->setMaxLength(100);
    ui->gestation2_nasolabialTriangle_description->setPlaceholderText(tr("... maximum 100 caractere"));
    ui->gestation2_ventricularSystem_description->setMaxLength(70);
    ui->gestation2_ventricularSystem_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_choroidalPlex_description->setMaxLength(70);
    ui->gestation2_choroidalPlex_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_cerebellum_description->setMaxLength(70);
    ui->gestation2_cerebellum_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_vertebralColumn_description->setMaxLength(100);
    ui->gestation2_vertebralColumn_description->setPlaceholderText(tr("... maximum 100 caractere"));
    ui->gestation2_heartPosition->setMaxLength(50);
    ui->gestation2_heartPosition->setPlaceholderText(tr("... maximum 50 caractere"));
    ui->gestation2_planPatruCamere_description->setMaxLength(70);
    ui->gestation2_planPatruCamere_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_ventricularEjectionPathLeft_description->setMaxLength(70);
    ui->gestation2_ventricularEjectionPathLeft_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_ventricularEjectionPathRight_description->setMaxLength(70);
    ui->gestation2_ventricularEjectionPathRight_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_intersectionVesselMagistral_description->setMaxLength(70);
    ui->gestation2_intersectionVesselMagistral_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_planTreiVase_description->setMaxLength(70);
    ui->gestation2_planTreiVase_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_pulmonaryAreas_description->setMaxLength(70);
    ui->gestation2_pulmonaryAreas_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_diaphragm_description->setMaxLength(70);
    ui->gestation2_diaphragm_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_stomach_description->setMaxLength(50);
    ui->gestation2_stomach_description->setPlaceholderText(tr("... maximum 50 caractere"));
    ui->gestation2_cholecist_description->setMaxLength(50);
    ui->gestation2_cholecist_description->setPlaceholderText(tr("... maximum 50 caractere"));
    ui->gestation2_intestine_description->setMaxLength(70);
    ui->gestation2_intestine_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_kidneys_description->setMaxLength(70);
    ui->gestation2_kidneys_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_ureter_description->setMaxLength(70);
    ui->gestation2_ureter_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_extremities_description->setMaxLength(150);
    ui->gestation2_extremities_description->setPlaceholderText(tr("... maximum 150 caractere"));
    ui->gestation2_placenta_localization->setMaxLength(50);
    ui->gestation2_placenta_localization->setPlaceholderText(tr("... maximum 50 caractere"));
    ui->gestation2_placentaStructure_description->setMaxLength(150);
    ui->gestation2_placentaStructure_description->setPlaceholderText(tr("... maximum 150 caractere"));
    ui->gestation2_umbilicalCordon_description->setMaxLength(70);
    ui->gestation2_umbilicalCordon_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_cervix_description->setMaxLength(150);
    ui->gestation2_cervix_description->setPlaceholderText(tr("... maximum 150 caractere"));
    ui->gestation2_comment->setPlaceholderText(tr("... maximum 250 caractere"));
    ui->gestation2_gestation_age->setInputMask("99s. 9z.");
    ui->gestation2_bpd_age->setInputMask("99s. 9z.");
    ui->gestation2_hc_age->setInputMask("99s. 9z.");
    ui->gestation2_ac_age->setInputMask("99s. 9z.");
    ui->gestation2_fl_age->setInputMask("99s. 9z.");
    ui->gestation2_fetus_age->setInputMask("99s. 9z.");

    //--- concluzion
    ui->gestation2_concluzion->setPlaceholderText(QObject::tr("... maximum 500 caractere"));

    //--- recommandation
    ui->gestation2_recommendation->setPlaceholderText(QObject::tr("... maximum 255 caractere"));
    ui->gestation2_recommendation->setMaxLength(255);
}

void ReportPageGestation2::initConnections()
{
    // QDateEdit
    const QList<QDateEdit*> des = this->findChildren<QDateEdit*>();
    for (QDateEdit *de : des)
        connect(de, &QDateEdit::dateChanged,
                this, &ReportPageGestation2::dataWasModified, Qt::UniqueConnection);

    // QComboBox
    const QList<QComboBox*> combos = this->findChildren<QComboBox*>();
    for (QComboBox *combo: combos)
        connect(combo, &QComboBox::currentTextChanged,
                this, &ReportPageGestation2::dataWasModified, Qt::UniqueConnection);

    // QLineEdit
    const QList<QLineEdit*> items = this->findChildren<QLineEdit*>();
    for (QLineEdit *item : items)
        connect(item, &QLineEdit::textChanged,
                this, &ReportPageGestation2::dataWasModified, Qt::UniqueConnection);

    // QPlainTextEdit
    const QList<QPlainTextEdit*> tes = this->findChildren<QPlainTextEdit*>();
    for (QPlainTextEdit *te : tes)
        connect(te, &QPlainTextEdit::textChanged,
                this, &ReportPageGestation2::dataWasModified, Qt::UniqueConnection);

    // QToolButton
    const QList<QToolButton*> btns = this->findChildren<QToolButton*>();
    for (QToolButton *btn : btns)
        btn->setStyleSheet(toolButtonStyleForText);

    // calcularea varstei gestationale
    connect(ui->gestation2_dateMenstruation, &QDateEdit::dateChanged,
            this, &ReportPageGestation2::onDateLMPChanged, Qt::UniqueConnection);

    // data probabila a nasterii
    connect(ui->gestation2_fetus_age, &QLineEdit::textEdited,
            this, &ReportPageGestation2::setProbabilDateBirth, Qt::UniqueConnection);

    // masa fatului
    connect(ui->gestation2_fetusMass, &QLineEdit::editingFinished,
            this, &ReportPageGestation2::updateDescriptionFetusWeight, Qt::UniqueConnection);
    connect(ui->gestation2_fetus_age, &QLineEdit::editingFinished,
            this, &ReportPageGestation2::updateDescriptionFetusWeight, Qt::UniqueConnection);

    // a.ombelicala, a.uterina si a.cerebrala medie
    connect(ui->gestation2_ombilic_PI, &QLineEdit::editingFinished,
            this, &ReportPageGestation2::updateTextDescriptionDoppler, Qt::UniqueConnection);
    connect(ui->gestation2_uterLeft_PI, &QLineEdit::editingFinished,
            this, &ReportPageGestation2::updateTextDescriptionDoppler, Qt::UniqueConnection);
    connect(ui->gestation2_uterRight_PI, &QLineEdit::editingFinished,
            this, &ReportPageGestation2::updateTextDescriptionDoppler, Qt::UniqueConnection);
    connect(ui->gestation2_cerebral_PI, &QLineEdit::editingFinished,
            this, &ReportPageGestation2::updateTextDescriptionDoppler, Qt::UniqueConnection);
    //-- doppler depinnde de fatul corespunde
    connect(ui->gestation2_fetus_age, &QLineEdit::editingFinished,
            this, &ReportPageGestation2::updateTextDescriptionDoppler, Qt::UniqueConnection);

    //--- conclusion (QToolButton) - concluzia
    connect(ui->btnAddTemplate, &QAbstractButton::clicked,
            this, &ReportPageGestation2::handleAddTemplate, Qt::UniqueConnection);
    connect(ui->btnSelectTemplate, &QAbstractButton::clicked,
            this, &ReportPageGestation2::handleSelectTemplate, Qt::UniqueConnection);

    //--- findings (QAction + LineEditCustom) - descrierea formatiunilor, calculelor etc.
    for (auto &r : rows_action_findings) {
        connect(r.action_add, &QAction::triggered,
                this, &ReportPageGestation2::handleAddFindingsTemplates, Qt::UniqueConnection);
        connect(r.action_select, &QAction::triggered,
                this, &ReportPageGestation2::handleSelectFindingsTemplates, Qt::UniqueConnection);
    }

    connect(ui->gestation2_concluzion, &QPlainTextEdit::textChanged,
            this, [this](){
                emit concluzionTextChanged();
            });
}

void ReportPageGestation2::setGestation2ItemsEnabled(bool trim2)
{
    ui->gestation2_nasalBones->setEnabled(trim2);
    ui->gestation2_nasalBones_dimens->setEnabled(trim2);
    ui->gestation2_nasalFold->setEnabled(trim2);
    ui->label_gestation2_nasalBones->setEnabled(trim2);
    ui->label_gestation2_nasalBones_dimens->setEnabled(trim2);
    ui->label_gestation2_nasalBones_mm->setEnabled(trim2);
    ui->label_gestation2_nasalFold->setEnabled(trim2);
    ui->label_gestation2_nasalFold_mm->setEnabled(trim2);

    ui->label_gestation2_fissureSilvius->setEnabled(trim2);
    ui->gestation2_fissureSilvius->setEnabled(trim2);
    ui->label_gestation2_ventricularEjectionPathLeft->setEnabled(trim2);
    ui->gestation2_ventricularEjectionPathLeft->setEnabled(trim2);
    ui->gestation2_ventricularEjectionPathLeft_description->setEnabled(trim2);
    ui->label_gestation2_ventricularEjectionPathRight->setEnabled(trim2);
    ui->gestation2_ventricularEjectionPathRight->setEnabled(trim2);
    ui->gestation2_ventricularEjectionPathRight_description->setEnabled(trim2);
    ui->label_gestation2_planBicav->setEnabled(trim2);
    ui->gestation2_planBicav->setEnabled(trim2);
}

QString ReportPageGestation2::calculateGestationalAge(const QDate &lmp)
{
    QDate today = QDate::currentDate();
    int daysDifference = lmp.daysTo(today);

    if (daysDifference < 0) {
        return nullptr;
    }

    int weeks = daysDifference / 7;
    int remainingDays = daysDifference % 7;

    ui->gestation2_trimestru2->setChecked(weeks >= 14 && weeks < 28);
    ui->gestation2_trimestru3->setChecked(weeks >= 28);
    setGestation2ItemsEnabled(ui->gestation2_trimestru2->isChecked());

    return QString("%1s. %2z.").arg(weeks).arg(remainingDays);
}

QDate ReportPageGestation2::calculateProbabilDateBirth(const int week, const int day)
{
    int fetalAgeTotalDays = (week * 7) + day;
    int remainingDaysToDueDate = 280 - fetalAgeTotalDays; // Restul până la 40 săptămâni

    return QDate::currentDate().addDays(remainingDaysToDueDate);
}

void ReportPageGestation2::extractWeeksAndDays(const QString &str_vg, int &weeks, int &days)
{
    static const QRegularExpression regex(R"((\d+)s\.\s*(\d+)z\.)");
    QRegularExpressionMatch match = regex.match(str_vg);

    if (match.hasMatch()) {
        weeks = match.captured(1).toInt();  // Prima grupare (\d+) -> săptămâni
        days = match.captured(2).toInt();   // A doua grupare (\d+) -> zile
    } else {
        weeks = 0;
        days = 0;
    }
}

QDate ReportPageGestation2::calculateDueDateFromFetalAge(int fetalAgeWeeks, int fetalAgeDays)
{
    int fetalAgeTotalDays = (fetalAgeWeeks * 7) + fetalAgeDays;
    int remainingDaysToDueDate = 280 - fetalAgeTotalDays; // Restul până la 40 săptămâni

    return QDate::currentDate().addDays(remainingDaysToDueDate);
}

void ReportPageGestation2::setDueDateGestation2()
{
    int weeks, days;
    extractWeeksAndDays(ui->gestation2_fetus_age->text(), weeks, days);
    if (weeks > 0)
        ui->gestation2_probabilDateBirth->setDate(calculateDueDateFromFetalAge(weeks, days));
}



QString ReportPageGestation2::getPercentageByDopplerUmbelicalArtery()
{
    if (ui->gestation2_ombilic_PI->text().isEmpty())
        return QString();

    int weeks, days;
    extractWeeksAndDays(ui->gestation2_fetus_age->text(), weeks, days);
    double current_PI = ui->gestation2_ombilic_PI->text().replace(",", ".").toDouble();

    ReferenceInterpretationResult res;
    res = refRanges->determineFmfPercentile(FetalReferenceRanges::Type::UmbilicalArteryPI,
                                            current_PI,
                                            weeks);
    if (res.ok) {
        ui->gestation2_ombilic_flux->setCurrentIndex(res.flow);
        return "Doppler a.ombelicale: " + res.interpretation;
    } else {
        qWarning(logWarning()) << res.errorString;
    }
    return QString();
}

QString ReportPageGestation2::getPercentageByDopplerUterineArteryLeft()
{
    if (ui->gestation2_uterLeft_PI->text().isEmpty())
        return QString();

    int weeks, days;
    extractWeeksAndDays(ui->gestation2_fetus_age->text(), weeks, days);
    double current_PI = ui->gestation2_uterLeft_PI->text().replace(",", ".").toDouble();

    ReferenceInterpretationResult res;
    res = refRanges->determineFmfPercentile(FetalReferenceRanges::Type::UterineArteryPI,
                                            current_PI,
                                            weeks);
    if (res.ok) {
        ui->gestation2_uterLeft_flux->setCurrentIndex(res.flow);
        return "Doppler a.uterină stânga: " + res.interpretation;
    } else {
        qWarning(logWarning()) << res.errorString;
    }

    return QString();
}

QString ReportPageGestation2::getPercentageByDopplerUterineArteryRight()
{
    if (ui->gestation2_uterRight_PI->text().isEmpty())
        return QString();

    int weeks, days;
    extractWeeksAndDays(ui->gestation2_fetus_age->text(), weeks, days);
    double current_PI = ui->gestation2_uterRight_PI->text().replace(",", ".").toDouble();

    ReferenceInterpretationResult res;
    res = refRanges->determineFmfPercentile(FetalReferenceRanges::Type::UterineArteryPI,
                                            current_PI,
                                            weeks);
    if (res.ok) {
        ui->gestation2_uterRight_flux->setCurrentIndex(res.flow);
        return "Doppler a.uterină dreapta: " + res.interpretation;
    } else {
        qWarning(logWarning()) << res.errorString;
    }

    return QString();
}

QString ReportPageGestation2::getPercentageByDopplerCMA()
{
    if (ui->gestation2_cerebral_PI->text().isEmpty())
        return QString();

    int weeks, days;
    extractWeeksAndDays(ui->gestation2_fetus_age->text(), weeks, days);
    double current_PI = ui->gestation2_cerebral_PI->text().replace(",", ".").toDouble();

    ReferenceInterpretationResult res;
    res = refRanges->determineFmfPercentile(FetalReferenceRanges::Type::MiddleCerebralArteryPI,
                                            current_PI,
                                            weeks);
    if (res.ok) {
        ui->gestation2_cerebral_flux->setCurrentIndex(res.flow);
        return "Doppler a.cerebrală medie: " + res.interpretation;
    } else {
        qWarning(logWarning()) << res.errorString;
    }

    return QString();
}

void ReportPageGestation2::bindTabGes2Fields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":gestation_age", ui->gestation2_gestation_age->text().isEmpty()
                                      ? QVariant()
                                      : ui->gestation2_gestation_age->text());
    q.bindValue(":trimestru",        ui->gestation2_trimestru2->isChecked() ? 2 : 3);
    q.bindValue(":dateMenstruation", ui->gestation2_dateMenstruation->date().toString("yyyy-MM-dd"));
    q.bindValue(":view_examination", ui->gestation2_view_examination->currentIndex());
    q.bindValue(":single_multiple_pregnancy", ui->gestation2_pregnancy->currentIndex());
    q.bindValue(":single_multiple_pregnancy_description",
                ui->gestation2_pregnancy_description->text().isEmpty()
                    ? QVariant() : ui->gestation2_pregnancy_description->text());
    q.bindValue(":antecedent",     QVariant());
    q.bindValue(":comment",        ui->gestation2_comment->toPlainText().isEmpty()
                                ? QVariant()
                                : ui->gestation2_comment->toPlainText());
    q.bindValue(":concluzion", ui->gestation2_concluzion->toPlainText());
    q.bindValue(":recommendation", ui->gestation2_recommendation->text().isEmpty()
                                       ? QVariant()
                                       : ui->gestation2_recommendation->text());
    q.bindValue(":fetalPrezentation", ui->comboFetalPrezentation->currentIndex());
    q.bindValue(":multiplePregnancy", ui->comboMultiplePregnancy->currentIndex());
}

void ReportPageGestation2::bindBiometryFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":BPD",     ui->gestation2_bpd->text());
    q.bindValue(":BPD_age", ui->gestation2_bpd_age->text());
    q.bindValue(":HC",      ui->gestation2_hc->text());
    q.bindValue(":HC_age",  ui->gestation2_hc_age->text());
    q.bindValue(":AC",      ui->gestation2_ac->text());
    q.bindValue(":AC_age",  ui->gestation2_ac_age->text());
    q.bindValue(":FL",      ui->gestation2_fl->text());
    q.bindValue(":FL_age",  ui->gestation2_fl_age->text());
    q.bindValue(":FetusCorresponds", ui->gestation2_fetus_age->text());
}

void ReportPageGestation2::bindCraniumFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":calloteCranium", ui->gestation2_calloteCranium->currentIndex());
    q.bindValue(":facialeProfile", ui->gestation2_facialeProfile->currentIndex());
    q.bindValue(":nasalBones", ui->gestation2_nasalBones->currentIndex());
    q.bindValue(":nasalBones_dimens", ui->gestation2_nasalBones_dimens->text().isEmpty()
                                          ? QVariant()
                                          : ui->gestation2_nasalBones_dimens->text());
    q.bindValue(":eyeball", ui->gestation2_eyeball->currentIndex());
    q.bindValue(":eyeball_desciption", ui->gestation2_eyeball_desciption->text().isEmpty()
                                           ? QVariant()
                                           : ui->gestation2_eyeball_desciption->text());
    q.bindValue(":nasolabialTriangle", ui->gestation2_nasolabialTriangle->currentIndex());
    q.bindValue(":nasolabialTriangle_description", ui->gestation2_nasolabialTriangle_description->text().isEmpty()
                                                       ? QVariant()
                                                       : ui->gestation2_nasolabialTriangle_description->text());
    q.bindValue(":nasalFold", ui->gestation2_nasalFold->text().isEmpty()
                                  ? QVariant()
                                  : ui->gestation2_nasalFold->text());
}

void ReportPageGestation2::bindSNCFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":hemispheres", ui->gestation2_hemispheres->currentIndex());
    q.bindValue(":fissureSilvius", ui->gestation2_fissureSilvius->currentIndex());
    q.bindValue(":corpCalos", ui->gestation2_corpCalos->currentIndex());
    q.bindValue(":ventricularSystem", ui->gestation2_ventricularSystem->currentIndex());
    q.bindValue(":ventricularSystem_description", ui->gestation2_ventricularSystem_description->text().isEmpty()
                                                      ? QVariant()
                                                      : ui->gestation2_ventricularSystem_description->text());
    q.bindValue(":cavityPellucidSeptum", ui->gestation2_cavityPellucidSeptum->currentIndex());
    q.bindValue(":choroidalPlex", ui->gestation2_choroidalPlex->currentIndex());
    q.bindValue(":choroidalPlex_description", ui->gestation2_choroidalPlex_description->text().isEmpty()
                                                  ? QVariant()
                                                  : ui->gestation2_choroidalPlex_description->text());
    q.bindValue(":cerebellum", ui->gestation2_cerebellum->currentIndex());
    q.bindValue(":cerebellum_description", ui->gestation2_cerebellum_description->text().isEmpty()
                                               ? QVariant()
                                               : ui->gestation2_cerebellum_description->text());
    q.bindValue(":vertebralColumn", ui->gestation2_vertebralColumn->currentIndex());
    q.bindValue(":vertebralColumn_description", ui->gestation2_vertebralColumn_description->text().isEmpty()
                                                    ? QVariant()
                                                    : ui->gestation2_vertebralColumn_description->text());
}

void ReportPageGestation2::bindHeartFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":position",
                ui->gestation2_heartPosition->text().isEmpty()
                    ? QVariant()
                    : ui->gestation2_heartPosition->text());
    q.bindValue(":heartBeat", ui->gestation2_heartBeat->currentIndex());
    q.bindValue(":heartBeat_frequency",
                ui->gestation2_heartBeat_frequency->text().isEmpty()
                    ? QVariant()
                    : ui->gestation2_heartBeat_frequency->text());
    q.bindValue(":heartBeat_rhythm", ui->gestation2_heartBeat_rhythm->currentIndex());
    q.bindValue(":pericordialCollections", ui->gestation2_pericordialCollections->currentIndex());
    q.bindValue(":planPatruCamere", ui->gestation2_planPatruCamere->currentIndex());
    q.bindValue(":planPatruCamere_description",
                ui->gestation2_planPatruCamere_description->text().isEmpty()
                    ? QVariant()
                    : ui->gestation2_planPatruCamere_description->text());
    q.bindValue(":ventricularEjectionPathLeft", ui->gestation2_ventricularEjectionPathLeft->currentIndex());
    q.bindValue(":ventricularEjectionPathLeft_description",
                ui->gestation2_ventricularEjectionPathLeft_description->text().isEmpty()
                    ? QVariant()
                    : ui->gestation2_ventricularEjectionPathLeft_description->text());
    q.bindValue(":ventricularEjectionPathRight", ui->gestation2_ventricularEjectionPathRight->currentIndex());
    q.bindValue(":ventricularEjectionPathRight_description",
                ui->gestation2_ventricularEjectionPathRight_description->text().isEmpty()
                    ? QVariant()
                    : ui->gestation2_ventricularEjectionPathRight_description->text());
    q.bindValue(":intersectionVesselMagistral", ui->gestation2_intersectionVesselMagistral->currentIndex());
    q.bindValue(":intersectionVesselMagistral_description",
                ui->gestation2_intersectionVesselMagistral_description->text().isEmpty()
                    ? QVariant()
                    : ui->gestation2_intersectionVesselMagistral_description->text());
    q.bindValue(":planTreiVase", ui->gestation2_planTreiVase->currentIndex());
    q.bindValue(":planTreiVase_description",
                ui->gestation2_planTreiVase_description->text().isEmpty()
                    ? QVariant()
                    : ui->gestation2_planTreiVase_description->text());
    q.bindValue(":archAorta", ui->gestation2_archAorta->currentIndex());
    q.bindValue(":planBicav", ui->gestation2_planBicav->currentIndex());
}

void ReportPageGestation2::bindThoraxFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":pulmonaryAreas", ui->gestation2_pulmonaryAreas->currentIndex());
    q.bindValue(":pulmonaryAreas_description",
                ui->gestation2_pulmonaryAreas_description->text().isEmpty()
                    ? QVariant()
                    : ui->gestation2_pulmonaryAreas_description->text());
    q.bindValue(":pleuralCollections", ui->gestation2_pleuralCollections->currentIndex());
    q.bindValue(":diaphragm", ui->gestation2_diaphragm->currentIndex());
}

void ReportPageGestation2::bindAbdomenFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":abdominalWall", ui->gestation2_abdominalWall->currentIndex());
    q.bindValue(":abdominalCollections", ui->gestation2_abdominalCollections->currentIndex());
    q.bindValue(":stomach", ui->gestation2_stomach->currentIndex());
    q.bindValue(":stomach_description", ui->gestation2_stomach_description->text().isEmpty()
                                            ? QVariant()
                                            : ui->gestation2_stomach_description->text());
    q.bindValue(":abdominalOrgans", ui->gestation2_abdominalOrgans->currentIndex());
    q.bindValue(":cholecist", ui->gestation2_cholecist->currentIndex());
    q.bindValue(":cholecist_description", ui->gestation2_cholecist_description->text().isEmpty()
                                              ? QVariant()
                                              : ui->gestation2_cholecist_description->text());
    q.bindValue(":intestine", ui->gestation2_intestine->currentIndex());
    q.bindValue(":intestine_description", ui->gestation2_intestine_description->text().isEmpty()
                                              ? QVariant()
                                              : ui->gestation2_intestine_description->text());
}

void ReportPageGestation2::bindUrinarySystemFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":kidneys", ui->gestation2_kidneys->currentIndex());
    q.bindValue(":kidneys_descriptions", ui->gestation2_kidneys_description->text().isEmpty()
                                             ? QVariant()
                                             : ui->gestation2_kidneys_description->text());
    q.bindValue(":ureter", ui->gestation2_ureter->currentIndex());
    q.bindValue(":ureter_descriptions", ui->gestation2_ureter_description->text().isEmpty()
                                            ? QVariant()
                                            : ui->gestation2_ureter_description->text());
    q.bindValue(":bladder", ui->gestation2_bladder->currentIndex());
}

void ReportPageGestation2::bindOtherFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":externalGenitalOrgans", ui->gestation2_fetusSex->currentIndex());
    q.bindValue(":externalGenitalOrgans_aspect", QVariant());
    q.bindValue(":extremities", ui->gestation2_extremities->currentIndex());
    q.bindValue(":extremities_descriptions", ui->gestation2_extremities_description->text().isEmpty()
                                                 ? QVariant()
                                                 : ui->gestation2_extremities_description->text());
    q.bindValue(":fetusMass", ui->gestation2_fetusMass->text().isEmpty()
                                  ? QVariant()
                                  : ui->gestation2_fetusMass->text());
    q.bindValue(":placenta", ui->gestation2_placenta->currentIndex());
    q.bindValue(":placentaLocalization", ui->gestation2_placenta_localization->text().isEmpty()
                                             ? QVariant()
                                             : ui->gestation2_placenta_localization->text());
    q.bindValue(":placentaDegreeMaturation", ui->gestation2_placentaDegreeMaturation->text().isEmpty()
                                                 ? QVariant()
                                                 : ui->gestation2_placentaDegreeMaturation->text());
    q.bindValue(":placentaDepth", ui->gestation2_placentaDepth->text().isEmpty()
                                      ? QVariant()
                                      : ui->gestation2_placentaDepth->text());
    q.bindValue(":placentaStructure", ui->gestation2_placentaStructure->currentIndex());
    q.bindValue(":placentaStructure_descriptions", ui->gestation2_placentaStructure_description->text().isEmpty()
                                                       ? QVariant()
                                                       : ui->gestation2_placentaStructure_description->text());
    q.bindValue(":umbilicalCordon", ui->gestation2_umbilicalCordon->currentIndex());
    q.bindValue(":umbilicalCordon_description", ui->gestation2_umbilicalCordon_description->text().isEmpty()
                                                    ? QVariant()
                                                    : ui->gestation2_umbilicalCordon_description->text());
    q.bindValue(":insertionPlacenta", ui->gestation2_insertionPlacenta->currentIndex());
    q.bindValue(":amnioticIndex", ui->gestation2_amnioticIndex->text().isEmpty()
                                      ? QVariant()
                                      : ui->gestation2_amnioticIndex->text());
    q.bindValue(":amnioticIndexAspect", ui->gestation2_amnioticIndexAspect->currentIndex());
    q.bindValue(":amnioticBedDepth", ui->gestation2_amnioticBedDepth->text().isEmpty()
                                         ? QVariant()
                                         : ui->gestation2_amnioticBedDepth->text());
    q.bindValue(":cervix", ui->gestation2_cervix->text().isEmpty()
                               ? QVariant()
                               : ui->gestation2_cervix->text());
    q.bindValue(":cervix_description", ui->gestation2_cervix_description->text().isEmpty()
                                           ? QVariant()
                                           : ui->gestation2_cervix_description->text());
}

void ReportPageGestation2::bindDopplerFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":ombilic_PI", (ui->gestation2_ombilic_PI->text().isEmpty()) ? QVariant() : ui->gestation2_ombilic_PI->text());
    q.bindValue(":ombilic_RI", (ui->gestation2_ombilic_RI->text().isEmpty()) ? QVariant() : ui->gestation2_ombilic_RI->text());
    q.bindValue(":ombilic_SD", (ui->gestation2_ombilic_SD->text().isEmpty()) ? QVariant() : ui->gestation2_ombilic_SD->text());
    q.bindValue(":ombilic_flux", ui->gestation2_ombilic_flux->currentIndex());

    q.bindValue(":cerebral_PI", (ui->gestation2_cerebral_PI->text().isEmpty()) ? QVariant() : ui->gestation2_cerebral_PI->text());
    q.bindValue(":cerebral_RI", (ui->gestation2_cerebral_RI->text().isEmpty()) ? QVariant() : ui->gestation2_cerebral_RI->text());
    q.bindValue(":cerebral_SD", (ui->gestation2_cerebral_SD->text().isEmpty()) ? QVariant() : ui->gestation2_cerebral_SD->text());
    q.bindValue(":cerebral_flux", ui->gestation2_cerebral_flux->currentIndex());

    q.bindValue(":uterRight_PI", (ui->gestation2_uterRight_PI->text().isEmpty()) ? QVariant() : ui->gestation2_uterRight_PI->text());
    q.bindValue(":uterRight_RI", (ui->gestation2_uterRight_RI->text().isEmpty()) ? QVariant() : ui->gestation2_uterRight_RI->text());
    q.bindValue(":uterRight_SD", (ui->gestation2_uterRight_SD->text().isEmpty()) ? QVariant() : ui->gestation2_uterRight_SD->text());
    q.bindValue(":uterRight_flux", ui->gestation2_uterRight_flux->currentIndex());

    q.bindValue(":uterLeft_PI", (ui->gestation2_uterLeft_PI->text().isEmpty()) ? QVariant() : ui->gestation2_uterLeft_PI->text());
    q.bindValue(":uterLeft_RI", (ui->gestation2_uterLeft_RI->text().isEmpty()) ? QVariant() : ui->gestation2_uterLeft_RI->text());
    q.bindValue(":uterLeft_SD", (ui->gestation2_uterLeft_SD->text().isEmpty()) ? QVariant() : ui->gestation2_uterLeft_SD->text());
    q.bindValue(":uterLeft_flux", ui->gestation2_uterLeft_flux->currentIndex());

    q.bindValue(":ductVenos", ui->gestation2_ductVenos->currentIndex());
}

void ReportPageGestation2::insertConclusionTemplate(const QString m_conclusion, const QString m_system)
{
    /** verificam dublajul */
    QSqlQuery q(m_currentDB);
    q.prepare("SELECT COUNT(name) FROM conclusionTemplates WHERE name = :name");
    q.bindValue(":name", m_conclusion);
    if(q.exec() && q.next()) {
        if (q.value(0).toInt() > 0) {
            QMessageBox msgBox(QMessageBox::Question,
                               tr("Verificarea dublajului"),
                               tr("Concluzia <b>%1</b> există ca șablon.<br>"
                                  "Doriți să prelungiți validarea ?").arg(m_conclusion),
                               QMessageBox::Yes | QMessageBox::No, this);

            if (msgBox.exec() == QMessageBox::No){
                return;
            }
        }
    }

    /** inseram datele */
    QString str = "INSERT INTO conclusionTemplates (deletionMark, cod, name, %1%, uuid) VALUES (?,?,?,?,?)";
    str.replace("%1%", globals().thisMySQL ? "`system`" : "system");
    q.prepare(str);
    q.addBindValue(0);
    q.addBindValue(m_db.getLastIdForTable("conclusionTemplates") + 1);
    q.addBindValue(m_conclusion);
    q.addBindValue(m_system);
    q.addBindValue(QUuid::createUuid().toRfc4122());
    if (!q.exec()) {
        qWarning(logWarning()) << "handleAddTemplate error:" << q.lastError().text();
        CustomMessage msg(this);
        msg.setWindowTitle(QGuiApplication::applicationDisplayName());
        msg.setTextTitle(tr("Inserarea șablonului <b>%1</b><br>"
                            "în baza de date nu s-a efectuat !!!")
                             .arg(m_conclusion));
        msg.setDetailedText(q.lastError().text());
        msg.exec();
        return;
    }

    emit showPopUp(tr("Șablonul adăugat cu succes<br>"
                      "în baza de date."));
}

void ReportPageGestation2::insertFindigsTemplate(const QString m_description, const QString m_typeSystem)
{
    /** verificam dublajul */
    QSqlQuery q(m_currentDB);
    q.prepare(R"(SELECT name FROM formationsSystemTemplates WHERE name = :name AND typeSystem = :typeSystem)");
    q.bindValue(":name",       m_description);
    q.bindValue(":typeSystem", m_typeSystem);
    if (q.exec() && q.next()) {
        if (q.value(0).toString() == m_description) {
            QMessageBox msgBox(QMessageBox::Question,
                               tr("Verificarea dublajului"),
                               tr("Descrierea <b><u>'%1'</u></b> există ca șablon.<br>"
                                  "Doriți să prelungiți validarea ?")
                                   .arg(m_description),
                               QMessageBox::Yes | QMessageBox::No, this);

            if (msgBox.exec() == QMessageBox::No){
                return;
            }
        }
    }

    /** inseram sablonul */
    q.prepare(R"(INSERT INTO formationsSystemTemplates (id, deletionMark, name, typeSystem, uuid) VALUES (?, ?, ?, ?, ?))");
    q.addBindValue(m_db.getLastIdForTable("formationsSystemTemplates") + 1);
    q.addBindValue(0);
    q.addBindValue(m_description);
    q.addBindValue(m_typeSystem);
    q.addBindValue(QUuid::createUuid().toRfc4122());
    if (q.exec()) {
        emit showPopUp(tr("Șablonul adăugat cu succes<br>"
                          "în baza de date."));
    }
}

bool ReportPageGestation2::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (keyEvent->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))
                return false;

            focusNextChild();
            return true;
        }
    }

    if (event->type() == QEvent::FocusIn) {
        QWidget *w = qobject_cast<QWidget *>(obj);
        if (!w)
            return ReportPageBase::eventFilter(obj, event);

        QWidget *target = w;

        // dacă focusul intră pe subcontrolul unui widget compus,
        // urcăm până la copilul direct din scrollArea contents
        QWidget *contents = ui->scrollArea_gestation2->widget();
        while (target && target->parentWidget() && target->parentWidget() != contents) {
            target = target->parentWidget();
        }

        if (target && contents->isAncestorOf(target)) {
            QTimer::singleShot(0, this, [this, target]() {
                ui->scrollArea_gestation2->ensureWidgetVisible(target, 20, 20);
            });
        }
    }

    return ReportPageBase::eventFilter(obj, event);
}

void ReportPageGestation2::changeEvent(QEvent *event)
{
    ReportPageBase::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
}
