#include "reportpagegynecology.h"
#include "documents/reportdialog.h"
#include "ui_reportpagegynecology.h"

ReportPageGynecology::ReportPageGynecology(DataBase &db,
                                           QSqlDatabase &currentDB,
                                           QWidget *parent)
    : ReportPageBase(parent)
    , ui(new Ui::ReportPageGynecology)
    , m_db(db)
    , m_currentDB(currentDB)
    , toolButtonStyleForText(m_db.toolButtonStyleForText())
{
    ui->setupUi(this);

    initRequiredStructure();
    initInstallEventFilter();

    setDefaultContext();
    setPropertyMaxLengthText();
    initConnections();
}

ReportPageGynecology::~ReportPageGynecology()
{
    delete ui;
}

bool ReportPageGynecology::loadData(int idReport)
{
    if (idReport <= 0)
        return false;

    QSqlQuery q(m_currentDB);
    q.prepare(R"(SELECT * FROM tableGynecology WHERE id_reportEcho = :id_reports)");
    q.bindValue(":id_reports", idReport);
    if(!q.exec()) {
        qWarning(logWarning()).noquote()
        << "ReportPageGynecology error:" << q.lastError().text()
        << "\nLast query:" << q.lastQuery();
        return false;
    }

    if (!q.next()) {
        qWarning(logWarning()) << "Nu s-au gasit datele pentru sistema 'gynecology' !!!";
        return false;
    }

    // QLineEdit
    const QList<QLineEdit*> items = this->findChildren<QLineEdit*>();
    std::vector<QSignalBlocker> itemsBlockers;
    itemsBlockers.reserve(items.size());
    for (QLineEdit *item : items)
        itemsBlockers.emplace_back(item);

    // QComboBox
    const QList<QComboBox*> combos = this->findChildren<QComboBox*>();
    std::vector<QSignalBlocker> itemsCombos;
    itemsCombos.reserve(combos.size());
    for (QComboBox *combo: combos)
        itemsCombos.emplace_back(combo);

    // QPlainTextEdit
    const QList<QPlainTextEdit*> itemsText = this->findChildren<QPlainTextEdit*>();
    std::vector<QSignalBlocker> itemsTextBlockers;
    itemsTextBlockers.reserve(itemsText.size());
    for (QPlainTextEdit *text : itemsText)
        itemsTextBlockers.emplace_back(text);

    bool transvaginal_checked = q.value("transvaginal").toBool();
    ui->gynecology_btn_transvaginal->setChecked(transvaginal_checked);
    ui->gynecology_btn_transabdom->setChecked(!transvaginal_checked);

    ui->gynecology_dateMenstruation->setDate(QDate::fromString(q.value("dateMenstruation").toString(), "yyyy-MM-dd"));
    ui->gynecology_antecedent->setText(q.value("antecedent").toString());
    ui->gynecology_uterus_dimens->setText(q.value("uterus_dimens").toString());
    ui->gynecology_uterus_pozition->setText(q.value("uterus_pozition").toString());
    ui->gynecology_uterus_ecostructure->setText(q.value("uterus_ecostructure").toString());
    ui->gynecology_uterus_formations->setPlainText(q.value("uterus_formations").toString());
    ui->gynecology_combo_jonctional_zone->setCurrentText(q.value("junctional_zone").toString());
    ui->gynecology_jonctional_zone_description->setText(q.value("junctional_zone_description").toString());
    ui->gynecology_ecou_dimens->setText(q.value("ecou_dimens").toString());
    ui->gynecology_ecou_ecostructure->setText(q.value("ecou_ecostructure").toString());
    ui->gynecology_cervix_dimens->setText(q.value("cervix_dimens").toString());
    ui->gynecology_cervix_ecostructure->setText(q.value("cervix_ecostructure").toString());
    ui->gynecology_combo_canal_cervical->setCurrentText(q.value("cervical_canal").toString());
    ui->gynecology_canal_cervical_formations->setText(q.value("cervical_canal_formations").toString());
    ui->gynecology_douglas->setText(q.value("douglas").toString());
    ui->gynecology_plex_venos->setText(q.value("plex_venos").toString());
    ui->gynecology_ovary_right_dimens->setText(q.value("ovary_right_dimens").toString());
    ui->gynecology_ovary_left_dimens->setText(q.value("ovary_left_dimens").toString());
    ui->gynecology_ovary_right_volum->setText(q.value("ovary_right_volum").toString());
    ui->gynecology_ovary_left_volum->setText(q.value("ovary_left_volum").toString());
    ui->gynecology_follicule_right->setText(q.value("ovary_right_follicle").toString());
    ui->gynecology_follicule_left->setText(q.value("ovary_left_follicle").toString());
    ui->gynecology_ovary_formations_right->setPlainText(q.value("ovary_right_formations").toString());
    ui->gynecology_ovary_formations_left->setPlainText(q.value("ovary_left_formations").toString());
    ui->gynecology_combo_fallopian_tubes->setCurrentText(q.value("fallopian_tubes").toString());
    ui->gynecology_fallopian_tubes_formations->setText(q.value("fallopian_tubes_formations").toString());
    ui->gynecology_concluzion->setPlainText(q.value("concluzion").toString());
    ui->gynecology_recommendation->setText(q.value("recommendation").toString());

    return true;
}

bool ReportPageGynecology::saveData(int idReport)
{
    if (idReport <= 0)
        return false;

    if (existDocument(idReport))
        return updateData(idReport);
    else
        return insertData(idReport);
}

QString ReportPageGynecology::concluzionText() const
{
    return ui->gynecology_concluzion
               ? ui->gynecology_concluzion->toPlainText().trimmed()
               : QString{};
}

ReportSections::ReportSystem ReportPageGynecology::system() const
{
    return ReportSections::ReportSystem::Gynecology;
}

bool ReportPageGynecology::isTransvaginal() const
{
    return ui->gynecology_btn_transvaginal->isChecked();
}

void ReportPageGynecology::updateMenstrualStatus()
{
    ReportDialog *report = qobject_cast<ReportDialog*>(window());
    if (!report)
        return;

    const QDate examDate = report->documentDate();
    const QDate lmpDate = ui->gynecology_dateMenstruation->date();

    const int daysFromLmp = lmpDate.daysTo(examDate);

    QString text;
    if (daysFromLmp == 0) {
        text = tr("0 zile de la ciclul menstrual");
    } else if (daysFromLmp > 90) {
        const int years = daysFromLmp / 365;
        text = (years <= 0)
                   ? tr("amenoree < 1 an")
                   : tr("amenoree %1 ani").arg(years);
    } else {
        text = tr("a %1 zi de la ciclul menstrual").arg(daysFromLmp);
    }

    ui->gynecology_text_date_menstruation->setText(text);
}

void ReportPageGynecology::handleSelectTemplate()
{
    CatalogTableEditor *dlg = new CatalogTableEditor(m_db,
                                                     CatalogType::FormType::Selection,
                                                     CatalogType::Type::ConclusionTemplates,
                                                     this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setFilterQuery("Ginecologia");
    connect(dlg, &CatalogTableEditor::dataSelected, this, [this](const QVariantMap &data){
        if (data.isEmpty())
            return;
        ui->gynecology_concluzion->appendPlainText(data["name"].toString());
    });
    dlg->show();
}

void ReportPageGynecology::handleAddTemplate()
{
    /** pregatim variabile locale */
    const QString m_concluzion = ui->gynecology_concluzion->toPlainText().trimmed();
    const QString m_system     = "Ginecologia";

    /** verificam daca sunt valorile */
    if (m_concluzion.isEmpty() || m_system.isEmpty())
        return;

    /** inseram datele */
    insertConclusionTemplate(m_concluzion, m_system);
}

void ReportPageGynecology::handleSelectFindingsTemplates()
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

    /** ----- QAbstractButton + QPlainTextEdit ---------*/
    if (auto btn = qobject_cast<QAbstractButton*>(s)) {

        /** functia standart de cautare a randului dupa conditia */
        auto it = std::find_if(rows_btn_findings.begin(),
                               rows_btn_findings.end(),
                               [btn](const FindingsTemplatesBtn &r)
                               {
                                   return btn == r.btn_select;
                               });

        /** daca it ajuns pana la sfarsit ne oprim */
        if (it == rows_btn_findings.end())
            return;

        /** pointer la obiect */
        const auto& r = *it;
        dlg->setFilterQuery(r.name_system);
        connect(dlg, &CatalogTableEditor::dataSelected, this, [dlg, r](const QVariantMap &data){
            if (data.isEmpty() || !dlg)
                return;
            r.item_edit->setPlainText(data["name"].toString());
            dlg->close();
        });
        dlg->show();
        return;
    }

    /** ----- LineEditCustom + QLineEdit --------- */
    if (auto edit = qobject_cast<LineEditCustom*>(s->parent())) {
        Q_UNUSED(edit);

        dlg->setFilterQuery("Recomandari (ginecologia)");
        connect(dlg, &CatalogTableEditor::dataSelected, this, [dlg, this](const QVariantMap &data){
            if (data.isEmpty() || !dlg)
                return;
            ui->gynecology_recommendation->setText(data["name"].toString());
            dlg->close();
        });
        dlg->show();
    }
}

void ReportPageGynecology::handleAddFindingsTemplates()
{
    QObject* s = sender();
    if (!s)
        return;

    /** ----- QAbstractButton + QPlainTextEdit ---------*/
    if (auto btn = qobject_cast<QAbstractButton*>(s)) {

        /** functia standart de cautare a randului dupa conditia */
        auto it = std::find_if(rows_btn_findings.begin(),
                               rows_btn_findings.end(),
                               [btn](const FindingsTemplatesBtn &r)
                               {
                                   return btn == r.btn_add;
                               });

        /** daca it ajuns pana la sfarsit ne oprim */
        if (it == rows_btn_findings.end())
            return;

        /** pointer la obiect */
        const auto& r = *it;
        insertFindigsTemplate(r.item_edit->toPlainText(), r.name_system);
        return;
    }

    /** ----- LineEditCustom + QLineEdit --------- */
    if (auto edit = qobject_cast<LineEditCustom*>(s->parent())) {
        Q_UNUSED(edit);
        insertFindigsTemplate(ui->gynecology_recommendation->text().trimmed(), "Recomandari (ginecologia)");
    }
}

void ReportPageGynecology::initRequiredStructure()
{
    rows_btn_findings = {
        {ui->btnSelectTempletsUter     , ui->btnAddTempletsUter     , ui->gynecology_uterus_formations     , "Ginecologia (uter)"      },
        {ui->btnSelectTempletsOvarLeft , ui->btnAddTempletsOvarLeft , ui->gynecology_ovary_formations_left , "Ginecologia (ovar stang)"},
        {ui->btnSelectTempletsOvarRight, ui->btnAddTempletsOvarRight, ui->gynecology_ovary_formations_right, "Ginecologia (ovar drept)"}
    };
}

void ReportPageGynecology::initInstallEventFilter()
{
    this->installEventFilter(this);

    foreach (QWidget *widget, findChildren<QWidget*>()) {
        widget->installEventFilter(this);
    }

    foreach (QWidget *widget, findChildren<QWidget*>()) {
        if (qobject_cast<QLineEdit *>(widget) ||
            qobject_cast<QPlainTextEdit *>(widget) ||
            qobject_cast<QComboBox *>(widget)) {
            widget->setFocusPolicy(Qt::StrongFocus);
        }
        if (QPushButton *button = qobject_cast<QPushButton *>(widget)) {
            button->setAutoDefault(false);
            button->setDefault(false);
        }
    }
}

void ReportPageGynecology::setDefaultContext()
{
    ui->gynecology_btn_transvaginal->setChecked(true);
    ui->gynecology_dateMenstruation->setDate(QDate::currentDate());

    QTimer::singleShot(0, this, [this]() {
        updateMenstrualStatus();
    });

    if (ui->gynecology_antecedent->text().isEmpty())
        ui->gynecology_antecedent->setText("abs.");
    if (ui->gynecology_uterus_pozition->text().isEmpty())
        ui->gynecology_uterus_pozition->setText("anteflexie");
    if (ui->gynecology_uterus_ecostructure->text().isEmpty())
        ui->gynecology_uterus_ecostructure->setText("omogenă");
    if (ui->gynecology_uterus_formations->toPlainText().isEmpty())
        ui->gynecology_uterus_formations->setPlainText("abs.");
    if (ui->gynecology_ecou_ecostructure->text().isEmpty())
        ui->gynecology_ecou_ecostructure->setText("omogenă");
    if (ui->gynecology_cervix_ecostructure->text().isEmpty())
        ui->gynecology_cervix_ecostructure->setText("omogenă");
    if (ui->gynecology_douglas->text().isEmpty())
        ui->gynecology_douglas->setText("liber");
    if (ui->gynecology_plex_venos->text().isEmpty())
        ui->gynecology_plex_venos->setText("nu sunt dilatate");
    if (ui->gynecology_ovary_formations_right->toPlainText().isEmpty())
        ui->gynecology_ovary_formations_right->setPlainText("abs.");
    if (ui->gynecology_ovary_formations_left->toPlainText().isEmpty())
        ui->gynecology_ovary_formations_left->setPlainText("abs.");
    if (ui->gynecology_recommendation->text().isEmpty())
        ui->gynecology_recommendation->setText("consultația ginecologului");
}

void ReportPageGynecology::setPropertyMaxLengthText()
{
    ui->gynecology_antecedent->setMaxLength(150);
    ui->gynecology_uterus_dimens->setMaxLength(25);
    ui->gynecology_uterus_pozition->setMaxLength(30);
    ui->gynecology_uterus_ecostructure->setMaxLength(30);
    ui->gynecology_antecedent->setPlaceholderText(tr("...maximum 150 caractere"));
    ui->gynecology_uterus_dimens->setPlaceholderText(tr("...maximum 25 caractere"));
    ui->gynecology_uterus_pozition->setPlaceholderText(tr("...maximum 30 caractere"));
    ui->gynecology_uterus_ecostructure->setPlaceholderText(tr("...maximum 30 caractere"));

    ui->gynecology_uterus_formations->setPlaceholderText(tr("... maximum 500 caractere"));
    ui->gynecology_jonctional_zone_description->setMaxLength(256);
    ui->gynecology_jonctional_zone_description->setPlaceholderText(tr("... maximum 256 caractere"));
    ui->gynecology_canal_cervical_formations->setMaxLength(256);
    ui->gynecology_canal_cervical_formations->setPlaceholderText(tr("... maximum 256 caractere"));

    ui->gynecology_ecou_dimens->setMaxLength(5);
    ui->gynecology_ecou_ecostructure->setMaxLength(100);
    ui->gynecology_douglas->setMaxLength(100);
    ui->gynecology_plex_venos->setMaxLength(150);
    ui->gynecology_ovary_left_dimens->setMaxLength(25);
    ui->gynecology_ovary_right_dimens->setMaxLength(25);
    ui->gynecology_ovary_left_volum->setMaxLength(5);
    ui->gynecology_ovary_right_volum->setMaxLength(5);
    ui->gynecology_follicule_left->setMaxLength(100);
    ui->gynecology_follicule_right->setMaxLength(100);
    ui->gynecology_ecou_dimens->setPlaceholderText(tr("... maximum 5 caractere"));
    ui->gynecology_ecou_ecostructure->setPlaceholderText(tr("... maximum 100 caractere"));
    ui->gynecology_douglas->setPlaceholderText(tr("... maximum 100 caractere"));
    ui->gynecology_plex_venos->setPlaceholderText(tr("... maximum 150 caractere"));
    ui->gynecology_ovary_left_dimens->setPlaceholderText(tr("... maximum 25 caractere"));
    ui->gynecology_ovary_right_dimens->setPlaceholderText(tr("... maximum 25 caractere"));
    ui->gynecology_ovary_left_volum->setPlaceholderText(tr("... maximum 5 caractere"));
    ui->gynecology_ovary_right_volum->setPlaceholderText(tr("... maximum 5 caractere"));
    ui->gynecology_follicule_left->setPlaceholderText(tr("... maximum 100 caractere"));
    ui->gynecology_follicule_right->setPlaceholderText(tr("... maximum 100 caractere"));

    ui->gynecology_fallopian_tubes_formations->setMaxLength(256);
    ui->gynecology_fallopian_tubes_formations->setPlaceholderText(tr("... maximum 256 caractere"));
    ui->gynecology_ovary_formations_left->setPlaceholderText(tr("... maximum 300 caractere"));
    ui->gynecology_ovary_formations_right->setPlaceholderText(tr("... maximum 300 caractere"));

    //--- concluzion
    ui->gynecology_concluzion->setPlaceholderText(tr("... maximum 500 caractere"));
}

void ReportPageGynecology::initConnections()
{
    // QLineEdit
    const QList<QLineEdit*> items = this->findChildren<QLineEdit*>();
    for (QLineEdit *item : items)
        connect(item, &QLineEdit::textChanged,
                this, &ReportPageGynecology::dataWasModified, Qt::UniqueConnection);

    // QComboBox
    const QList<QComboBox*> combos = this->findChildren<QComboBox*>();
    for (QComboBox *combo: combos)
        connect(combo, &QComboBox::currentTextChanged,
                this, &ReportPageGynecology::dataWasModified, Qt::UniqueConnection);

    // QPlainTextEdit
    const QList<QPlainTextEdit*> tes = this->findChildren<QPlainTextEdit*>();
    for (QPlainTextEdit *te : tes)
        connect(te, &QPlainTextEdit::textChanged,
                this, &ReportPageGynecology::dataWasModified, Qt::UniqueConnection);

    // stil
    const QList<QToolButton*> btns = this->findChildren<QToolButton*>();
    for (QToolButton *btn : btns)
        btn->setStyleSheet(toolButtonStyleForText);

    // conclusion (QToolButton) - concluzia
    connect(ui->btn_add_template_gynecology, &QAbstractButton::clicked,
            this, &ReportPageGynecology::handleAddTemplate, Qt::UniqueConnection);
    connect(ui->btn_template_gynecology, &QAbstractButton::clicked,
            this, &ReportPageGynecology::handleSelectTemplate, Qt::UniqueConnection);

    // recomandari
    connect(ui->gynecology_recommendation->actionAddItem(), &QAction::triggered,
            this, &ReportPageGynecology::handleAddFindingsTemplates, Qt::UniqueConnection);
    connect(ui->gynecology_recommendation->actionOpenList(), &QAction::triggered,
            this, &ReportPageGynecology::handleSelectFindingsTemplates, Qt::UniqueConnection);

    // findings (QToolButton + QPlainTextEdit) - descrierea formatiunilor, calculelor etc.
    for (auto &r : rows_btn_findings) {
        connect(r.btn_add, &QToolButton::clicked,
                this, &ReportPageGynecology::handleAddFindingsTemplates, Qt::UniqueConnection);
        connect(r.btn_select, &QToolButton::clicked,
                this, &ReportPageGynecology::handleSelectFindingsTemplates, Qt::UniqueConnection);
    }

    connect(ui->gynecology_dateMenstruation, &QDateEdit::dateChanged,
            this, &ReportPageGynecology::updateMenstrualStatus, Qt::UniqueConnection);

    connect(ui->gynecology_concluzion, &QPlainTextEdit::textChanged,
            this, [this](){
                emit concluzionTextChanged();
            });
}

bool ReportPageGynecology::existDocument(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(R"(
        SELECT EXISTS(
            SELECT 1
            FROM tableGynecology
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

void ReportPageGynecology::bindFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":transvaginal",
                globals().thisMySQL
                    ? QVariant(ui->gynecology_btn_transvaginal->isChecked())
                    : QVariant(int(ui->gynecology_btn_transvaginal->isChecked())));
    q.bindValue(":dateMenstruation",    ui->gynecology_dateMenstruation->date().toString("yyyy-MM-dd"));
    q.bindValue(":antecedent", ui->gynecology_antecedent->text().isEmpty()
                                    ? QVariant()
                                    : ui->gynecology_antecedent->text());
    q.bindValue(":uterus_dimens", ui->gynecology_uterus_dimens->text().isEmpty()
                                    ? QVariant()
                                    : ui->gynecology_uterus_dimens->text());
    q.bindValue(":uterus_pozition", ui->gynecology_uterus_pozition->text().isEmpty()
                                        ? QVariant()
                                        : ui->gynecology_uterus_pozition->text());
    q.bindValue(":uterus_ecostructure", ui->gynecology_uterus_ecostructure->text().isEmpty()
                                            ? QVariant()
                                            : ui->gynecology_uterus_ecostructure->text());
    q.bindValue(":uterus_formations", ui->gynecology_uterus_formations->toPlainText().isEmpty()
                                        ? QVariant()
                                        : ui->gynecology_uterus_formations->toPlainText());
    q.bindValue(":junctional_zone", ui->gynecology_combo_jonctional_zone->currentText());
    q.bindValue(":junctional_zone_description", ui->gynecology_jonctional_zone_description->text().isEmpty()
                                                ? QVariant()
                                                : ui->gynecology_jonctional_zone_description->text());
    q.bindValue(":ecou_dimens", ui->gynecology_ecou_dimens->text().isEmpty()
                                    ? QVariant()
                                    : ui->gynecology_ecou_dimens->text());
    q.bindValue(":ecou_ecostructure", ui->gynecology_ecou_ecostructure->text().isEmpty()
                                        ? QVariant()
                                        : ui->gynecology_ecou_ecostructure->text());
    q.bindValue(":cervix_dimens", ui->gynecology_cervix_dimens->text().isEmpty()
                                    ? QVariant()
                                    : ui->gynecology_cervix_dimens->text());
    q.bindValue(":cervix_ecostructure", ui->gynecology_cervix_ecostructure->text().isEmpty()
                                            ? QVariant()
                                            : ui->gynecology_cervix_ecostructure->text());
    q.bindValue(":cervical_canal", ui->gynecology_combo_canal_cervical->currentText());
    q.bindValue(":cervical_canal_formations", ui->gynecology_canal_cervical_formations->text().isEmpty()
                                                ? QVariant()
                                                : ui->gynecology_canal_cervical_formations->text());
    q.bindValue(":douglas",                ui->gynecology_douglas->text());
    q.bindValue(":plex_venos",             ui->gynecology_plex_venos->text());
    q.bindValue(":ovary_right_dimens",     ui->gynecology_ovary_right_dimens->text());
    q.bindValue(":ovary_left_dimens",      ui->gynecology_ovary_left_dimens->text());
    q.bindValue(":ovary_right_volum",      ui->gynecology_ovary_right_volum->text());
    q.bindValue(":ovary_left_volum",       ui->gynecology_ovary_left_volum->text());
    q.bindValue(":ovary_right_follicle",   ui->gynecology_follicule_right->text());
    q.bindValue(":ovary_left_follicle",    ui->gynecology_follicule_left->text());
    q.bindValue(":ovary_right_formations", ui->gynecology_ovary_formations_right->toPlainText());
    q.bindValue(":ovary_left_formations",  ui->gynecology_ovary_formations_left->toPlainText());
    q.bindValue(":fallopian_tubes",        ui->gynecology_combo_fallopian_tubes->currentText());
    q.bindValue(":fallopian_tubes_formations", ui->gynecology_fallopian_tubes_formations->text().isEmpty()
                                                    ? QVariant()
                                                    : ui->gynecology_fallopian_tubes_formations->text());
    q.bindValue(":concluzion", ui->gynecology_concluzion->toPlainText());
    q.bindValue(":recommendation", ui->gynecology_recommendation->text().isEmpty()
                                        ? QVariant()
                                        : ui->gynecology_recommendation->text());
}

bool ReportPageGynecology::insertData(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_gynecology_insert.sql"));
    bindFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Insert tableGynecology error:"
                               << q.lastError().text();
        return false;
    }
    qInfo(logInfo()) << "ReportPageGynecology: INSERT executat în tranzacția raportului; id=" << idReport;
    return true;
}

bool ReportPageGynecology::updateData(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_gynecology_update.sql"));
    bindFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Update tableGynecology error:"
                               << q.lastError().text();
        return false;
    }
    qInfo(logInfo()) << "ReportPageGynecology: UPDATE executat în tranzacția raportului; id=" << idReport;
    return true;
}

void ReportPageGynecology::insertConclusionTemplate(const QString m_conclusion, const QString m_system)
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

void ReportPageGynecology::insertFindigsTemplate(const QString m_description, const QString m_typeSystem)
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

bool ReportPageGynecology::eventFilter(QObject *obj, QEvent *event)
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
        QWidget *contents = ui->scrollArea_gynecology->widget();
        while (target && target->parentWidget() && target->parentWidget() != contents) {
            target = target->parentWidget();
        }

        if (target && contents->isAncestorOf(target)) {
            QTimer::singleShot(0, this, [this, target]() {
                ui->scrollArea_gynecology->ensureWidgetVisible(target, 20, 20);
            });
        }
    }

    return ReportPageBase::eventFilter(obj, event);
}

void ReportPageGynecology::changeEvent(QEvent *event)
{
    ReportPageBase::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
}
