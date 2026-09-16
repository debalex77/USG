#include "reportpagegestation1.h"
#include "ui_reportpagegestation1.h"

#include <QMessageBox>

#include <customs/custommessage.h>

#include <views/catalogtableeditor.h>

ReportPageGestation1::ReportPageGestation1(DataBase &db,
                                           QSqlDatabase &currentDB,
                                           QWidget *parent)
    : ReportPageBase(parent)
    , ui(new Ui::ReportPageGestation1)
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

ReportPageGestation1::~ReportPageGestation1()
{
    delete ui;
}

bool ReportPageGestation1::loadData(int idReport)
{
    if (idReport <= 0)
        return false;

    QSqlQuery q(m_currentDB);
    q.prepare(R"(SELECT * FROM tableGestation1 WHERE id_reportEcho = :id_reports)");
    q.bindValue(":id_reports", idReport);
    if(!q.exec()) {
        qWarning(logWarning()).noquote()
        << "ReportPageGestation1 error:" << q.lastError().text()
        << "\nLast query:" << q.lastQuery();
        return false;
    }

    if (!q.next()) {
        qWarning(logWarning()) << "Nu s-au gasit datele pentru sistema 'gestation1' !!!";
        return false;
    }

    // QDateEdit
    const QList<QDateEdit*> des = this->findChildren<QDateEdit*>();
    std::vector<QSignalBlocker> desBlocker;
    desBlocker.reserve(des.size());
    for (QDateEdit *de : des)
        desBlocker.emplace_back(de);

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

    const QSignalBlocker pregnancyBlocker(ui->comboMultiplePregnancy);
    const int pregnancy = q.value("multiplePregnancy").toInt();
    ui->comboMultiplePregnancy->setCurrentIndex(
        pregnancy >= 0 && pregnancy < ui->comboMultiplePregnancy->count() ? pregnancy : 0);

    switch (q.value("view_examination").toInt()) {
    case 1:
        ui->gestation1_view_good->setChecked(true);
        break;
    case 2:
        ui->gestation1_view_medium->setChecked(true);
        break;
    case 3:
        ui->gestation1_view_difficult->setChecked(true);
        break;
    }
    ui->gestation1_antecedent->setText(q.value("antecedent").toString());
    ui->gestation1_LMP->setDate(QDate::fromString(q.value("lmp").toString(), "yyyy-MM-dd"));
    calculateGestationalAge(ui->gestation1_LMP->date());
    calculateDueDate(ui->gestation1_LMP->date());
    ui->gestation1_probableDateBirth->setDate(calculateDueDate(ui->gestation1_LMP->date()));
    ui->gestation1_gestation->setText(q.value("gestation_age").toString());
    ui->gestation1_CRL_dimens->setText(q.value("CRL").toString());
    ui->gestation1_CRL_age->setText(q.value("CRL_age").toString());
    ui->gestation1_BPD_dimens->setText(q.value("BPD").toString());
    ui->gestation1_BPD_age->setText(q.value("BPD_age").toString());
    ui->gestation1_NT_dimens->setText(q.value("NT").toString());
    ui->gestation1_NT_percent->setText(q.value("NT_percent").toString());
    updateNtInterpretation();
    ui->gestation1_BN_dimens->setText(q.value("BN").toString());
    ui->gestation1_BN_percent->setText(q.value("BN_percent").toString());
    ui->gestation1_BCF->setText(q.value("BCF").toString());
    ui->gestation1_FL_dimens->setText(q.value("FL").toString());
    ui->gestation1_FL_age->setText(q.value("FL_age").toString());
    ui->gestation1_callote_cranium->setText(q.value("callote_cranium").toString());
    ui->gestation1_plex_choroid->setText(q.value("plex_choroid").toString());
    ui->gestation1_vertebral_column->setText(q.value("vertebral_column").toString());
    ui->gestation1_stomach->setText(q.value("stomach").toString());
    ui->gestation1_bladder->setText(q.value("bladder").toString());
    ui->gestation1_diaphragm->setText(q.value("diaphragm").toString());
    ui->gestation1_abdominal_wall->setText(q.value("abdominal_wall").toString());
    ui->gestation1_location_placenta->setText(q.value("location_placenta").toString());
    ui->gestation1_sac_vitelin->setText(q.value("sac_vitelin").toString());
    ui->gestation1_amniotic_liquid->setText(q.value("amniotic_liquid").toString());
    ui->gestation1_miometer->setText(q.value("miometer").toString());
    ui->gestation1_cervix->setText(q.value("cervix").toString());
    ui->gestation1_ovary->setText(q.value("ovary").toString());
    ui->gestation1_concluzion->setPlainText(q.value("concluzion").toString());
    ui->gestation1_recommendation->setText(q.value("recommendation").toString());

    return true;
}

bool ReportPageGestation1::saveData(int idReport)
{
    if (idReport <= 0)
        return false;

    if (existDocument(idReport))
        return updateData(idReport);
    else
        return insertData(idReport);

    return false;
}

QString ReportPageGestation1::concluzionText() const
{
    return ui->gestation1_concluzion
               ? ui->gestation1_concluzion->toPlainText().trimmed()
               : QString{};
}

ReportSections::ReportSystem ReportPageGestation1::system() const
{
    return ReportSections::ReportSystem::Gestation1;
}

QDate ReportPageGestation1::LMP() const
{
    return ui->gestation1_LMP->date();
}

QDate ReportPageGestation1::probableDateBirth() const
{
    return ui->gestation1_probableDateBirth->date();
}

ReportSections::ViewExamination ReportPageGestation1::getViewExamination() const
{
    if (ui->gestation1_view_good->isChecked())
        return ReportSections::ViewExamination::Good;
    if (ui->gestation1_view_medium->isChecked())
        return ReportSections::ViewExamination::Medium;
    if (ui->gestation1_view_difficult->isChecked())
        return ReportSections::ViewExamination::Difficult;
    return ReportSections::ViewExamination::Unknown;
}

void ReportPageGestation1::onDateLMPChanged()
{
    QString str_vg = calculateGestationalAge(ui->gestation1_LMP->date());
    if (str_vg == nullptr)
        return;
    ui->gestation1_gestation->setText(str_vg);
    ui->gestation1_probableDateBirth->setDate(calculateDueDate(ui->gestation1_LMP->date()));
}

void ReportPageGestation1::handleSelectTemplate()
{
    CatalogTableEditor *dlg = new CatalogTableEditor(m_db,
                                                     CatalogType::FormType::Selection,
                                                     CatalogType::Type::ConclusionTemplates,
                                                     this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setFilterQuery("Sarcina 11-14 săptămâni");
    connect(dlg, &CatalogTableEditor::dataSelected, this, [this](const QVariantMap &data){
        if (data.isEmpty())
            return;
        ui->gestation1_concluzion->appendPlainText(data["name"].toString());
    });
    dlg->show();
}

void ReportPageGestation1::handleAddTemplate()
{
    /** pregatim variabile locale */
    const QString m_concluzion = ui->gestation1_concluzion->toPlainText().trimmed();
    const QString m_system     = "Sarcina 11-14 săptămâni";

    /** verificam daca sunt valorile */
    if (m_concluzion.isEmpty() || m_system.isEmpty())
        return;

    /** inseram datele */
    insertConclusionTemplate(m_concluzion, m_system);
}

void ReportPageGestation1::handleSelectFindingsTemplates()
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

void ReportPageGestation1::handleAddFindingsTemplates()
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

void ReportPageGestation1::initRequiredStructure()
{
    rows_action_findings = {
        {ui->gestation1_recommendation->actionOpenList(), ui->gestation1_recommendation->actionAddItem(), ui->gestation1_recommendation, "Recomandari (gestatation1)"}
    };
}

void ReportPageGestation1::initInstallEventFilter()
{
    qApp->installEventFilter(this);

    foreach (QWidget *widget, findChildren<QWidget*>()) {
        widget->installEventFilter(this);
    }

    foreach (QWidget *widget, findChildren<QWidget*>()) {
        if (qobject_cast<QLineEdit *>(widget) ||
            qobject_cast<QPlainTextEdit *>(widget)) {

            widget->setFocusPolicy(Qt::StrongFocus);
        }
        if (QPushButton *button = qobject_cast<QPushButton *>(widget)) {
            button->setAutoDefault(false);
            button->setDefault(false);
        }
    }
}

void ReportPageGestation1::setDefaultContext()
{
    ui->gestation1_view_medium->setChecked(true);
    ui->gestation1_LMP->setDate(QDate::currentDate());

    ui->interpretationNT->setText(tr("Întroduceți valoarea NT"));

    if (ui->gestation1_antecedent->text().isEmpty())
        ui->gestation1_antecedent->setText("abs.");
    if (ui->gestation1_BCF->text().isEmpty())
        ui->gestation1_BCF->setText("prezenți, ritmici");
    if (ui->gestation1_callote_cranium->text().isEmpty())
        ui->gestation1_callote_cranium->setText("norm.");
    if (ui->gestation1_plex_choroid->text().isEmpty())
        ui->gestation1_plex_choroid->setText("norm.");
    if (ui->gestation1_vertebral_column->text().isEmpty())
        ui->gestation1_vertebral_column->setText("integră");
    if (ui->gestation1_stomach->text().isEmpty())
        ui->gestation1_stomach->setText("norm.");
    if (ui->gestation1_bladder->text().isEmpty())
        ui->gestation1_bladder->setText("norm.");
    if (ui->gestation1_diaphragm->text().isEmpty())
        ui->gestation1_diaphragm->setText("norm.");
    if (ui->gestation1_abdominal_wall->text().isEmpty())
        ui->gestation1_abdominal_wall->setText("integru");
    if (ui->gestation1_location_placenta->text().isEmpty())
        ui->gestation1_location_placenta->setText("peretele anterior");
    if (ui->gestation1_amniotic_liquid->text().isEmpty())
        ui->gestation1_amniotic_liquid->setText("omogen, transparent");
    if (ui->gestation1_miometer->text().isEmpty())
        ui->gestation1_miometer->setText("omogen; formațiuni solide, lichidiene abs.");
    if (ui->gestation1_cervix->text().isEmpty())
        ui->gestation1_cervix->setText("omogen; formaț3iuni solide, lichidiene abs.; închis, lungimea 32,9 mm");
    if (ui->gestation1_ovary->text().isEmpty())
        ui->gestation1_ovary->setText("aspect ecografic normal");
    if (ui->gestation1_recommendation->text().isEmpty())
        ui->gestation1_recommendation->setText("consultația ginecologului, examen ecografic la 18-20 săptămâni a sarcinei");
}

void ReportPageGestation1::setPropertyMaxLengthText()
{
    ui->gestation1_gestation->setInputMask("99s. 9z.");
    ui->gestation1_CRL_age->setInputMask("99s. 9z.");
    ui->gestation1_BPD_age->setInputMask("99s. 9z.");
    ui->gestation1_FL_age->setInputMask("99s. 9z.");

    ui->gestation1_antecedent->setMaxLength(150);
    ui->gestation1_antecedent->setPlaceholderText(tr("... maximum 150 caractere"));
    ui->gestation1_gestation->setMaxLength(20);
    ui->gestation1_CRL_dimens->setMaxLength(5);
    ui->gestation1_CRL_age->setMaxLength(20);
    ui->gestation1_BPD_dimens->setMaxLength(5);
    ui->gestation1_BPD_age->setMaxLength(20);
    ui->gestation1_NT_dimens->setMaxLength(5);
    ui->gestation1_NT_percent->setMaxLength(20);
    ui->gestation1_BN_dimens->setMaxLength(5);
    ui->gestation1_BN_percent->setMaxLength(20);
    ui->gestation1_BCF->setMaxLength(30);
    ui->gestation1_FL_dimens->setMaxLength(5);
    ui->gestation1_FL_age->setMaxLength(20);
    ui->gestation1_callote_cranium->setMaxLength(50);
    ui->gestation1_callote_cranium->setPlaceholderText(tr("... maximum 50 caractere"));
    ui->gestation1_plex_choroid->setMaxLength(50);
    ui->gestation1_plex_choroid->setPlaceholderText(tr("... maximum 50 caractere"));
    ui->gestation1_vertebral_column->setMaxLength(50);
    ui->gestation1_vertebral_column->setPlaceholderText(tr("... maximum 50 caractere"));
    ui->gestation1_stomach->setMaxLength(50);
    ui->gestation1_stomach->setPlaceholderText(tr("... maximum 50 caractere"));
    ui->gestation1_diaphragm->setMaxLength(50);
    ui->gestation1_diaphragm->setPlaceholderText(tr("... maximum 50 caractere"));
    ui->gestation1_bladder->setMaxLength(50);
    ui->gestation1_bladder->setPlaceholderText(tr("... maximum 50 caractere"));
    ui->gestation1_abdominal_wall->setMaxLength(50);
    ui->gestation1_abdominal_wall->setPlaceholderText(tr("... maximum 50 caractere"));
    ui->gestation1_location_placenta->setMaxLength(50);
    ui->gestation1_location_placenta->setPlaceholderText(tr("... maximum 50 caractere"));
    ui->gestation1_sac_vitelin->setMaxLength(50);
    ui->gestation1_sac_vitelin->setPlaceholderText(tr("... maximum 50 caractere"));
    ui->gestation1_amniotic_liquid->setMaxLength(40);
    ui->gestation1_amniotic_liquid->setPlaceholderText(tr("... maximum 40 caractere"));
    ui->gestation1_miometer->setMaxLength(200);
    ui->gestation1_miometer->setPlaceholderText(tr("... maximum 200 caractere"));
    ui->gestation1_cervix->setMaxLength(200);
    ui->gestation1_cervix->setPlaceholderText(tr("... maximum 200 caractere"));
    ui->gestation1_ovary->setMaxLength(200);
    ui->gestation1_ovary->setPlaceholderText(tr("... maximum 200 caractere"));
    ui->gestation1_sac_vitelin->setMaxLength(50);
    ui->gestation1_sac_vitelin->setPlaceholderText(tr("... maximum 50 caractere"));

    //--- concluzion
    ui->gestation1_concluzion->setPlaceholderText(tr("... maximum 500 caractere"));

    //--- recommandation
    ui->gestation1_recommendation->setPlaceholderText(tr("... maximum 255 caractere"));
    ui->gestation1_recommendation->setMaxLength(255);
}

void ReportPageGestation1::initConnections()
{
    // QDateEdit
    const QList<QDateEdit*> des = this->findChildren<QDateEdit*>();
    for (QDateEdit *de : des)
        connect(de, &QDateEdit::dateChanged,
                this, &ReportPageGestation1::dataWasModified, Qt::UniqueConnection);

    // QLineEdit
    const QList<QLineEdit*> items = this->findChildren<QLineEdit*>();
    for (QLineEdit *item : items)
        connect(item, &QLineEdit::textChanged,
                this, &ReportPageGestation1::dataWasModified, Qt::UniqueConnection);

    // QPlainTextEdit
    const QList<QPlainTextEdit*> tes = this->findChildren<QPlainTextEdit*>();
    for (QPlainTextEdit *te : tes)
        connect(te, &QPlainTextEdit::textChanged,
                this, &ReportPageGestation1::dataWasModified, Qt::UniqueConnection);

    // QToolButton
    const QList<QToolButton*> btns = this->findChildren<QToolButton*>();
    for (QToolButton *btn : btns)
        btn->setStyleSheet(toolButtonStyleForText);

    // calcularea varstei gestationale
    connect(ui->gestation1_LMP, &QDateEdit::dateChanged,
            this, &ReportPageGestation1::onDateLMPChanged, Qt::UniqueConnection);

    //--- conclusion (QToolButton) - concluzia
    connect(ui->btn_add_template_gestation1, &QAbstractButton::clicked,
            this, &ReportPageGestation1::handleAddTemplate, Qt::UniqueConnection);
    connect(ui->btn_template_gestation1, &QAbstractButton::clicked,
            this, &ReportPageGestation1::handleSelectTemplate, Qt::UniqueConnection);

    //--- findings (QAction + LineEditCustom) - descrierea formatiunilor, calculelor etc.
    for (auto &r : rows_action_findings) {
        connect(r.action_add, &QAction::triggered,
                this, &ReportPageGestation1::handleAddFindingsTemplates, Qt::UniqueConnection);
        connect(r.action_select, &QAction::triggered,
                this, &ReportPageGestation1::handleSelectFindingsTemplates, Qt::UniqueConnection);
    }

    connect(ui->gestation1_concluzion, &QPlainTextEdit::textChanged,
            this, [this](){
                emit concluzionTextChanged();
            });

    connect(ui->gestation1_NT_dimens, &QLineEdit::editingFinished,
            this, &ReportPageGestation1::updateNtInterpretation);
    connect(ui->gestation1_CRL_dimens, &QLineEdit::editingFinished,
            this, &ReportPageGestation1::updateNtInterpretation);
    connect(ui->comboMultiplePregnancy, &QComboBox::currentIndexChanged,
            this, &ReportPageGestation1::dataWasModified);

}

void ReportPageGestation1::updateNtInterpretation()
{
    ui->interpretationNT->setText(tr("Introduceți valori valide pentru NT și CRL"));
    ui->gestation1_NT_percent->clear();
    const QString nt_txt  = ui->gestation1_NT_dimens->text().trimmed();
    const QString crl_txt = ui->gestation1_CRL_dimens->text().trimmed();

    if (nt_txt.isEmpty() || crl_txt.isEmpty()) {
        ui->gestation1_NT_percent->clear();
        return;
    }
    const double nt = nt_txt.toDouble();
    const double crl = crl_txt.toDouble();

    if (nt <= 0.0 || crl <= 0.0) {
        ui->gestation1_NT_percent->clear();
        qWarning(logWarning()) << "Valori invalide pentru NT/CRL:"
                               << "NT =" << nt_txt
                               << "CRL =" << crl_txt;
        return;
    }

    NtPercentileResult res;
    res = refRanges->determineNtPercentile(nt, crl);
    if (res.ok) {
        ui->gestation1_NT_percent->setText(QString::number(res.percentile, 'f', 1));
        ui->interpretationNT->setText("NT " + res.interpretation);
    } else {
        ui->interpretationNT->setText(res.errorString);
    }
}

QString ReportPageGestation1::calculateGestationalAge(const QDate &lmp)
{
    QDate today = QDate::currentDate();
    int daysDifference = lmp.daysTo(today);

    if (daysDifference < 0) {
        return nullptr;
    }

    int weeks = daysDifference / 7;
    int remainingDays = daysDifference % 7;

    return QString("%1s. %2z.").arg(weeks).arg(remainingDays);
}

QDate ReportPageGestation1::calculateDueDate(const QDate &lmp)
{
    return lmp.addDays(280);  // Adăugăm 280 de zile (40 săptămâni)
}

bool ReportPageGestation1::existDocument(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(R"(
        SELECT EXISTS(
            SELECT 1
            FROM tableGestation1
            WHERE id_reportEcho = :id_reportEcho
        )
    )");
    q.bindValue(":id_reportEcho", idReport);
    if (!q.exec()) {
        qWarning(logWarning()) << q.lastError().text();
        return false;
    }

    if (q.next())
        return q.value(0).toBool();

    return false;
}

void ReportPageGestation1::bindFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":multiplePregnancy", ui->comboMultiplePregnancy->currentIndex());
    q.bindValue(":view_examination", static_cast<int>(getViewExamination()));
    q.bindValue(":antecedent", ui->gestation1_antecedent->text().isEmpty()
                                ? QVariant()
                                : ui->gestation1_antecedent->text());
    q.bindValue(":lmp",             ui->gestation1_LMP->date().toString("yyyy-MM-dd"));
    q.bindValue(":gestation_age",   ui->gestation1_gestation->text());
    q.bindValue(":CRL",             ui->gestation1_CRL_dimens->text());
    q.bindValue(":CRL_age",         ui->gestation1_CRL_age->text());
    q.bindValue(":BPD",             ui->gestation1_BPD_dimens->text());
    q.bindValue(":BPD_age",         ui->gestation1_BPD_age->text());
    q.bindValue(":NT",              ui->gestation1_NT_dimens->text());
    q.bindValue(":NT_percent",      ui->gestation1_NT_percent->text());
    q.bindValue(":BN",              ui->gestation1_BN_dimens->text());
    q.bindValue(":BN_percent",      ui->gestation1_BN_percent->text());
    q.bindValue(":BCF",             ui->gestation1_BCF->text());
    q.bindValue(":FL",              ui->gestation1_FL_dimens->text());
    q.bindValue(":FL_age",          ui->gestation1_FL_age->text());
    q.bindValue(":callote_cranium", ui->gestation1_callote_cranium->text());
    q.bindValue(":plex_choroid",    ui->gestation1_plex_choroid->text());
    q.bindValue(":vertebral_column", ui->gestation1_vertebral_column->text());
    q.bindValue(":stomach",          ui->gestation1_stomach->text());
    q.bindValue(":bladder",          ui->gestation1_bladder->text());
    q.bindValue(":diaphragm",        ui->gestation1_diaphragm->text());
    q.bindValue(":abdominal_wall",   ui->gestation1_abdominal_wall->text());
    q.bindValue(":location_placenta", ui->gestation1_location_placenta->text());
    q.bindValue(":sac_vitelin",       ui->gestation1_sac_vitelin->text());
    q.bindValue(":amniotic_liquid",   ui->gestation1_amniotic_liquid->text());
    q.bindValue(":miometer",          ui->gestation1_miometer->text());
    q.bindValue(":cervix",            ui->gestation1_cervix->text());
    q.bindValue(":ovary",             ui->gestation1_ovary->text());
    q.bindValue(":concluzion",       ui->gestation1_concluzion->toPlainText());
    q.bindValue(":recommendation", ui->gestation1_recommendation->text().isEmpty()
                                    ? QVariant()
                                    : ui->gestation1_recommendation->text());
}

bool ReportPageGestation1::insertData(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_ges1_insert.sql"));
    bindFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Insert tableGestation1 error:"
                               << q.lastError().text();
        return false;
    }
    qInfo(logInfo()) << "ReportPageGestation1: INSERT executat în tranzacția raportului; id=" << idReport;
    return true;
}

bool ReportPageGestation1::updateData(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_ges1_update.sql"));
    bindFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Update tableGestation1 error:"
                               << q.lastError().text();
        return false;
    }
    qInfo(logInfo()) << "ReportPageGestation1: UPDATE executat în tranzacția raportului; id=" << idReport;
    return true;
}

void ReportPageGestation1::insertConclusionTemplate(const QString m_conclusion, const QString m_system)
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

void ReportPageGestation1::insertFindigsTemplate(const QString m_description, const QString m_typeSystem)
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

bool ReportPageGestation1::eventFilter(QObject *obj, QEvent *event)
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
        QWidget *contents = ui->scrollArea_gestation1->widget();
        while (target && target->parentWidget() && target->parentWidget() != contents) {
            target = target->parentWidget();
        }

        if (target && contents->isAncestorOf(target)) {
            QTimer::singleShot(0, this, [this, target]() {
                ui->scrollArea_gestation1->ensureWidgetVisible(target, 20, 20);
            });
        }
    }

    return ReportPageBase::eventFilter(obj, event);
}

void ReportPageGestation1::changeEvent(QEvent *event)
{
    ReportPageBase::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
}
