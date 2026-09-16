#include "reportpageorgansinternal.h"
#include "ui_reportpageorgansinternal.h"

#include <QPushButton>

#include <customs/custommessage.h>

ReportPageOrgansInternal::ReportPageOrgansInternal(DataBase &db, QSqlDatabase &currentDB, QWidget *parent)
    : ReportPageBase(parent)
    , ui(new Ui::ReportPageOrgansInternal)
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

ReportPageOrgansInternal::~ReportPageOrgansInternal()
{
    delete ui;
}

bool ReportPageOrgansInternal::loadData(int idReport)
{
    if (idReport <= 0)
        return false;

    QString str = R"(
        SELECT
            l.id,
            l.id_reportEcho,
            l.[left]            AS liver_left_lobe,
            l.[right]           AS liver_right_lobe,
            l.contur            AS liver_contur,
            l.parenchim         AS liver_parenchim,
            l.ecogenity         AS liver_ecogenity,
            l.formations        AS liver_formations,
            l.ductsIntrahepatic AS liver_ductsIntrahepatic,
            l.porta             AS liver_porta,
            l.lienalis          AS liver_lienalis,
            l.concluzion        AS liver_concluzion,
            l.recommendation    AS liver_recommendation,
            c.form              AS cholecist_form,
            c.dimens            AS cholecist_dimens,
            c.walls             AS cholecist_walls,
            c.choledoc          AS cholecist_choledoc,
            c.formations        AS cholecist_formations,
            p.cefal             AS pancreas_cefal,
            p.corp              AS pancreas_corp,
            p.tail              AS pancreas_tail,
            p.texture           AS pancreas_texture,
            p.ecogency          AS pancreas_ecogency,
            p.formations        AS pancreas_formations,
            s.dimens            AS spleen_dimens,
            s.contur            AS spleen_contur,
            s.parenchim         AS spleen_parenchim,
            s.formations        AS spleen_formations,
            i.formations        AS intestinal_formation
        FROM
            tableLiver AS l
        LEFT JOIN
            tableCholecist AS c ON c.id_reportEcho = l.id_reportEcho
        LEFT JOIN
            tablePancreas AS p ON p.id_reportEcho = l.id_reportEcho
        LEFT JOIN
            tableSpleen AS s ON s.id_reportEcho = l.id_reportEcho
        LEFT JOIN
            tableIntestinalLoop AS i ON i.id_reportEcho = l.id_reportEcho
        WHERE
            l.id_reportEcho = :id_reports
    )";
    if (globals().thisMySQL) {
        str = str.replace("[", "`");
        str = str.replace("]", "`");
    }

    QSqlQuery q(m_currentDB);
    q.prepare(str);
    q.bindValue(":id_reports", idReport);
    if(!q.exec()) {
        qWarning(logWarning()).noquote()
            << "ReportPageOrgansInternal error:" << q.lastError().text()
            << "\nLast query:" << q.lastQuery();
        return false;
    }

    if (!q.next()) {
        qWarning(logWarning()) << "Nu s-au gasit datele pentru sistema 'organsInternal' !!!";
        return false;
    }

    const QList<QLineEdit*> items = this->findChildren<QLineEdit*>();
    std::vector<QSignalBlocker> itemsBlockers;
    itemsBlockers.reserve(items.size());
    for (QLineEdit *item : items)
        itemsBlockers.emplace_back(item);

    const QList<QPlainTextEdit*> itemsText = this->findChildren<QPlainTextEdit*>();
    std::vector<QSignalBlocker> itemsTextBlockers;
    itemsTextBlockers.reserve(itemsText.size());
    for (QPlainTextEdit *text : itemsText)
        itemsTextBlockers.emplace_back(text);

    // liver
    ui->liver_left->setText(q.value("liver_left_lobe").toString());
    ui->liver_right->setText(q.value("liver_right_lobe").toString());
    ui->liver_contour->setText(q.value("liver_contur").toString());
    ui->liver_parenchyma->setText(q.value("liver_parenchim").toString());
    ui->liver_ecogenity->setText(q.value("liver_ecogenity").toString());
    ui->liver_formations->setText(q.value("liver_formations").toString());
    ui->liver_duct_hepatic->setText(q.value("liver_ductsIntrahepatic").toString());
    ui->liver_porta->setText(q.value("liver_porta").toString());
    ui->liver_lienalis->setText(q.value("liver_lienalis").toString());
    ui->organsInternal_concluzion->setPlainText(q.value("liver_concluzion").toString());
    ui->organsInternal_recommendation->setText(q.value("liver_recommendation").toString());
    // cholecist
    ui->cholecist_form->setText(q.value("cholecist_form").toString());
    ui->cholecist_dimens->setText(q.value("cholecist_dimens").toString());
    ui->cholecist_walls->setText(q.value("cholecist_walls").toString());
    ui->cholecist_coledoc->setText(q.value("cholecist_choledoc").toString());
    ui->cholecist_formations->setText(q.value("cholecist_formations").toString());
    // pancreas
    ui->pancreas_cefal->setText(q.value("pancreas_cefal").toString());
    ui->pancreas_corp->setText(q.value("pancreas_corp").toString());
    ui->pancreas_tail->setText(q.value("pancreas_tail").toString());
    ui->pancreas_parenchyma->setText(q.value("pancreas_texture").toString());
    ui->pancreas_ecogenity->setText(q.value("pancreas_ecogency").toString());
    ui->pancreas_formations->setText(q.value("pancreas_formations").toString());
    // spleen
    ui->spleen_contour->setText(q.value("spleen_dimens").toString());
    ui->spleen_dimens->setText(q.value("spleen_contur").toString());
    ui->spleen_parenchyma->setText(q.value("spleen_parenchim").toString());
    ui->spleen_formations->setText(q.value("spleen_formations").toString());
    // intestinal loop
    ui->intestinalHandles->setPlainText(q.value("intestinal_formation").toString());

    return true;
}

bool ReportPageOrgansInternal::saveData(int idReport)
{
    if (idReport <= 0)
        return false;

    if (existDocument(idReport))
        return updateData(idReport);
    else
        return insertData(idReport);

    return false;
}

QString ReportPageOrgansInternal::concluzionText() const
{
    return ui->organsInternal_concluzion
               ? ui->organsInternal_concluzion->toPlainText().trimmed()
               : QString{};
}

ReportSections::ReportSystem ReportPageOrgansInternal::system() const
{
    return ReportSections::ReportSystem::OrgansInternal;
}

void ReportPageOrgansInternal::handleSelectTemplate()
{
    CatalogTableEditor *dlg = new CatalogTableEditor(m_db,
                                                     CatalogType::FormType::Selection,
                                                     CatalogType::Type::ConclusionTemplates,
                                                     this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setFilterQuery("Organe interne");
    connect(dlg, &CatalogTableEditor::dataSelected, this, [this](const QVariantMap &data){
        if (data.isEmpty())
            return;
        ui->organsInternal_concluzion->appendPlainText(data["name"].toString());
    });
    dlg->show();
}

void ReportPageOrgansInternal::handleAddTemplate()
{
    /** pregatim variabile locale */
    const QString m_concluzion = ui->organsInternal_concluzion->toPlainText().trimmed();
    const QString m_system     = "Organe interne";

    /** verificam daca sunt valorile */
    if (m_concluzion.isEmpty() || m_system.isEmpty())
        return;

    /** inseram datele */
    insertConclusionTemplate(m_concluzion, m_system);
}

void ReportPageOrgansInternal::handleSelectFindingsTemplates()
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

        return;
    }

    /** ----- 2. QAbstractButton + QPlainTextEdit ---------
     ** doar 'ui->intestinalHandles'                       */
    if (auto btn = qobject_cast<QAbstractButton*>(s)) {

        Q_UNUSED(btn);

        dlg->setFilterQuery("Intestine");
        connect(dlg, &CatalogTableEditor::dataSelected, this, [dlg, this](const QVariantMap &data){
            if (data.isEmpty() || !dlg)
                return;
            ui->intestinalHandles->setPlainText(data["name"].toString());
            dlg->close();
        });
        dlg->show();
    }

}

void ReportPageOrgansInternal::handleAddFindingsTemplates()
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

    /** ----- 2. QAbstractButton + QPlainTextEdit ---------
     ** doar 'ui->intestinalHandles'                       */
    if (auto btn = qobject_cast<QAbstractButton*>(s)) {
        Q_UNUSED(btn);
        insertFindigsTemplate(ui->intestinalHandles->toPlainText().trimmed(), "Intestine");
    }
}

void ReportPageOrgansInternal::initRequiredStructure()
{
    /** btnSelect, btnAdd, itemEdit, nameSystem */
    rows_action_findings = {
        {ui->liver_formations->actionOpenList(),              ui->liver_formations->actionAddItem() ,             ui->liver_formations ,    "Ficat"},
        {ui->cholecist_formations->actionOpenList(),          ui->cholecist_formations->actionAddItem(),          ui->cholecist_formations, "Colecist"},
        {ui->pancreas_formations->actionOpenList(),           ui->pancreas_formations->actionAddItem(),           ui->pancreas_formations,  "Pancreas"},
        {ui->spleen_formations->actionOpenList(),             ui->spleen_formations->actionAddItem(),             ui->spleen_formations,    "Splina"},
        {ui->organsInternal_recommendation->actionOpenList(), ui->organsInternal_recommendation->actionAddItem(), ui->organsInternal_recommendation, "Recomandari (org.interne)"}
    };
}

void ReportPageOrgansInternal::initInstallEventFilter()
{
    qApp->installEventFilter(this);

    foreach (QWidget *widget, findChildren<QWidget*>()) {
        widget->installEventFilter(this);
    }

    foreach (QWidget *widget, findChildren<QWidget*>()) {
        if (qobject_cast<QLineEdit *>(widget) || qobject_cast<QPlainTextEdit *>(widget)) {
            widget->setFocusPolicy(Qt::StrongFocus);
        }
        if (QPushButton *button = qobject_cast<QPushButton *>(widget)) {
            button->setAutoDefault(false);
            button->setDefault(false);
        }
    }
}

void ReportPageOrgansInternal::setDefaultContext()
{
    //--- liver
    if (ui->liver_contour->text().isEmpty())
        ui->liver_contour->setText("clar");
    if (ui->liver_parenchyma->text().isEmpty())
        ui->liver_parenchyma->setText("omogena");
    if (ui->liver_ecogenity->text().isEmpty())
        ui->liver_ecogenity->setText("medie");
    if (ui->liver_formations->text().isEmpty())
        ui->liver_formations->setText("lichidiene, solide abs.");
    if (ui->liver_duct_hepatic->text().isEmpty())
        ui->liver_duct_hepatic->setText("nu sunt dilatate");

    //--- cholecist
    if (ui->cholecist_form->text().isEmpty())
        ui->cholecist_form->setText("obisnuita");
    if (ui->cholecist_formations->text().isEmpty())
        ui->cholecist_formations->setText("abs.");

    //--- pancreas
    if (ui->pancreas_ecogenity->text().isEmpty())
        ui->pancreas_ecogenity->setText("sporita");
    if (ui->pancreas_parenchyma->text().isEmpty())
        ui->pancreas_parenchyma->setText("omogena");
    if (ui->pancreas_formations->text().isEmpty())
        ui->pancreas_formations->setText("lichidiene, solide abs.");

    //--- spleen
    if (ui->spleen_contour->text().isEmpty())
        ui->spleen_contour->setText("clar");
    if (ui->spleen_parenchyma->text().isEmpty())
        ui->spleen_parenchyma->setText("omogena");
    if (ui->spleen_formations->text().isEmpty())
        ui->spleen_formations->setText("lichidiene, solide abs.");

    //--- intestinalLoop
    if (ui->intestinalHandles->toPlainText().isEmpty())
        ui->intestinalHandles->setPlainText("formațiuni abs., ganglioni limfatici mezenteriali 5-10 mm fără aglomerări");
}

void ReportPageOrgansInternal::setPropertyMaxLengthText()
{
    //--- liver
    ui->liver_left->setMaxLength(5);
    ui->liver_right->setMaxLength(5);
    ui->liver_contour->setMaxLength(20);
    ui->liver_parenchyma->setMaxLength(20);
    ui->liver_ecogenity->setMaxLength(30);
    ui->liver_formations->setMaxLength(300);
    ui->liver_duct_hepatic->setMaxLength(50);
    ui->liver_porta->setMaxLength(5);
    ui->liver_lienalis->setMaxLength(5);
    ui->liver_left->setPlaceholderText(tr("...maximum 5 caractere"));
    ui->liver_right->setPlaceholderText(tr("...maximum 5 caractere"));
    ui->liver_contour->setPlaceholderText(tr("...maximum 20 caractere"));
    ui->liver_parenchyma->setPlaceholderText(tr("...maximum 20 caractere"));
    ui->liver_ecogenity->setPlaceholderText(tr("...maximum 30 caractere"));
    ui->liver_formations->setPlaceholderText(tr("...maximum 300 caractere"));
    ui->liver_duct_hepatic->setPlaceholderText(tr("...maximum 50 caractere"));
    ui->liver_porta->setPlaceholderText(tr("...maximum 5 caractere"));
    ui->liver_lienalis->setPlaceholderText(tr("...maximum 5 caractere"));

    //--- cholecist
    ui->cholecist_dimens->setMaxLength(15);
    ui->cholecist_form->setMaxLength(150);
    ui->cholecist_formations->setMaxLength(300);
    ui->cholecist_walls->setMaxLength(5);
    ui->cholecist_coledoc->setMaxLength(5);
    ui->cholecist_dimens->setPlaceholderText(tr("...maximum 15 caractere"));
    ui->cholecist_form->setPlaceholderText(tr("...maximum 150 caractere"));
    ui->cholecist_formations->setPlaceholderText(tr("...maximum 300 caractere"));
    ui->cholecist_walls->setPlaceholderText(tr("...maximum 5 caractere"));
    ui->cholecist_coledoc->setPlaceholderText(tr("...maximum 5 caractere"));

    //--- pancreas
    ui->pancreas_cefal->setMaxLength(5);
    ui->pancreas_corp->setMaxLength(5);
    ui->pancreas_tail->setMaxLength(5);
    ui->pancreas_parenchyma->setMaxLength(20);
    ui->pancreas_ecogenity->setMaxLength(30);
    ui->pancreas_formations->setMaxLength(300);
    ui->pancreas_cefal->setPlaceholderText(tr("...maximum 5 caractere"));
    ui->pancreas_corp->setPlaceholderText(tr("...maximum 5 caractere"));
    ui->pancreas_tail->setPlaceholderText(tr("...maximum 5 caractere"));
    ui->pancreas_parenchyma->setPlaceholderText(tr("...maximum 20 caractere"));
    ui->pancreas_ecogenity->setPlaceholderText(tr("...maximum 30 caractere"));
    ui->pancreas_formations->setPlaceholderText(tr("...maximum 300 caractere"));

    //--- spleen
    ui->spleen_dimens->setMaxLength(15);
    ui->spleen_contour->setMaxLength(20);
    ui->spleen_parenchyma->setMaxLength(30);
    ui->spleen_formations->setMaxLength(300);
    ui->spleen_dimens->setPlaceholderText(tr("...maximum 15 caractere"));
    ui->spleen_contour->setPlaceholderText(tr("...maximum 20 caractere"));
    ui->spleen_parenchyma->setPlaceholderText(tr("...maximum 30 caractere"));
    ui->spleen_formations->setPlaceholderText(tr("...maximum 300 caractere"));

    //--- intestinal loop
    ui->intestinalHandles->setPlaceholderText(tr("...maximum 300 caractere"));

    //--- concluzion
    ui->organsInternal_concluzion->setPlaceholderText(tr("... maximum 500 caractere"));

    //--- recommandation
    ui->organsInternal_recommendation->setPlaceholderText(tr("...maximum 255 caractere"));
    ui->organsInternal_recommendation->setMaxLength(255);
}

void ReportPageOrgansInternal::initConnections()
{
    //--- QLineEdit
    const QList<QLineEdit*> items = this->findChildren<QLineEdit*>();
    for (QLineEdit *item : items)
        connect(item, &QLineEdit::textChanged,
                this, &ReportPageOrgansInternal::dataWasModified, Qt::UniqueConnection);

    //---- QPlainTextEdit
    const QList<QPlainTextEdit*> tes = this->findChildren<QPlainTextEdit*>();
    for (QPlainTextEdit *te : tes)
        connect(te, &QPlainTextEdit::textChanged,
                this, &ReportPageOrgansInternal::dataWasModified, Qt::UniqueConnection);

    //--- stil
    const QList<QToolButton*> btns = this->findChildren<QToolButton*>();
    for (QToolButton *btn : btns)
        btn->setStyleSheet(toolButtonStyleForText);

    //--- conclusion (QToolButton) - concluzia
    connect(ui->btnAddTemplatesConclusion, &QAbstractButton::clicked,
            this, &ReportPageOrgansInternal::handleAddTemplate, Qt::UniqueConnection);
    connect(ui->btnSelectTemplatesConclusion, &QAbstractButton::clicked,
            this, &ReportPageOrgansInternal::handleSelectTemplate, Qt::UniqueConnection);

    //--- findings (QToolButton) - a.intestinale
    connect(ui->btnAddTemplatesIntestin, &QAbstractButton::clicked,
            this, &ReportPageOrgansInternal::handleAddFindingsTemplates, Qt::UniqueConnection);
    connect(ui->btnSelectTemplatesIntestin, &QAbstractButton::clicked,
            this, &ReportPageOrgansInternal::handleSelectFindingsTemplates, Qt::UniqueConnection);

    //--- findings (QAction + LineEditCustom) - descrierea formatiunilor, calculelor etc.
    for (auto &r : rows_action_findings) {
        connect(r.action_add, &QAction::triggered,
                this, &ReportPageOrgansInternal::handleAddFindingsTemplates, Qt::UniqueConnection);
        connect(r.action_select, &QAction::triggered,
                this, &ReportPageOrgansInternal::handleSelectFindingsTemplates, Qt::UniqueConnection);
    }

    connect(ui->organsInternal_concluzion, &QPlainTextEdit::textChanged,
            this, [this](){
                emit concluzionTextChanged();
            });
}

bool ReportPageOrgansInternal::existDocument(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(R"(
        SELECT EXISTS(
            SELECT 1
            FROM tableLiver
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

void ReportPageOrgansInternal::bindLiverFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":left", ui->liver_left->text());
    q.bindValue(":right", ui->liver_right->text());
    q.bindValue(":contur", ui->liver_contour->text());
    q.bindValue(":parenchim", ui->liver_parenchyma->text());
    q.bindValue(":ecogenity", ui->liver_ecogenity->text());
    q.bindValue(":formations", ui->liver_formations->text());
    q.bindValue(":ductsIntrahepatic", ui->liver_duct_hepatic->text());
    q.bindValue(":porta", ui->liver_porta->text());
    q.bindValue(":lienalis", ui->liver_lienalis->text());
    q.bindValue(":concluzion", ui->organsInternal_concluzion->toPlainText());
    q.bindValue(":recommendation", ui->organsInternal_recommendation->text().isEmpty()
                        ? QVariant()
                        : ui->organsInternal_recommendation->text());
}

void ReportPageOrgansInternal::bindCholecistFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":form", ui->cholecist_form->text());
    q.bindValue(":dimens", ui->cholecist_dimens->text().isEmpty()
                                ? QVariant()
                                : ui->cholecist_dimens->text());
    q.bindValue(":walls", ui->cholecist_walls->text().isEmpty()
                                ? QVariant()
                                : ui->cholecist_walls->text());
    q.bindValue(":choledoc", ui->cholecist_coledoc->text().isEmpty()
                                ? QVariant()
                                : ui->cholecist_coledoc->text());
    q.bindValue(":formations", ui->cholecist_formations->text().isEmpty()
                                ? QVariant()
                                : ui->cholecist_formations->text());
}

void ReportPageOrgansInternal::bindPancreasFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":cefal",         ui->pancreas_cefal->text());
    q.bindValue(":corp",          ui->pancreas_corp->text());
    q.bindValue(":tail",          ui->pancreas_tail->text());
    q.bindValue(":texture",       ui->pancreas_parenchyma->text());
    q.bindValue(":ecogency",      ui->pancreas_ecogenity->text());
    q.bindValue(":formations",    ui->pancreas_formations->text());
}

void ReportPageOrgansInternal::bindSpleenFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":dimens", ui->spleen_dimens->text());
    q.bindValue(":contur", ui->spleen_contour->text());
    q.bindValue(":parenchim", ui->spleen_parenchyma->text());
    q.bindValue(":formations", ui->spleen_formations->text());
}

void ReportPageOrgansInternal::bindIntestineFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":formations", ui->intestinalHandles->toPlainText());
}

bool ReportPageOrgansInternal::insertData(const int idReport)
{
    QSqlQuery q(m_currentDB);

    // liver
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_liver_insert.sql"));
    bindLiverFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Insert tableLiver error:"
                               << q.lastError().text();
        return false;
    }

    // cholecist
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_cholecist_insert.sql"));
    bindCholecistFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Insert tableCholecist error:"
                               << q.lastError().text();
        return false;
    }

    // pancreas
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_pancreas_insert.sql"));
    bindPancreasFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Insert tablePancreas error:"
                               << q.lastError().text();
        return false;
    }

    // spleen
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_spleen_insert.sql"));
    bindSpleenFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Insert tableSpleen error:"
                               << q.lastError().text();
        return false;
    }

    // intestine
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_intestine_insert.sql"));
    bindSpleenFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Insert tableIntestinalLoop error:"
                               << q.lastError().text();
        return false;
    }

    qInfo(logInfo()) << "ReportPageOrgansInternal: INSERT executat în tranzacția raportului; id=" << idReport;
    return true;
}

bool ReportPageOrgansInternal::updateData(const int idReport)
{
    QSqlQuery q(m_currentDB);

    // liver
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_liver_update.sql"));
    bindLiverFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Update tableLiver error:"
                               << q.lastError().text();
        return false;
    }

    // cholecist
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_cholecist_update.sql"));
    bindCholecistFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Update tableCholecist error:"
                               << q.lastError().text();
        return false;
    }

    // pancreas
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_pancreas_update.sql"));
    bindPancreasFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Update tablePancreas error:"
                               << q.lastError().text();
        return false;
    }

    // spleen
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_spleen_update.sql"));
    bindSpleenFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Update tableSpleen error:"
                               << q.lastError().text();
        return false;
    }

    // intestine
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_intestine_update.sql"));
    bindSpleenFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Update tableIntestinalLoop error:"
                               << q.lastError().text();
        return false;
    }

    qInfo(logInfo()) << "ReportPageOrgansInternal: UPDATE executat în tranzacția raportului; id=" << idReport;
    return true;
}

void ReportPageOrgansInternal::insertConclusionTemplate(const QString m_conclusion, const QString m_system)
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

void ReportPageOrgansInternal::insertFindigsTemplate(const QString m_description, const QString m_typeSystem)
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

bool ReportPageOrgansInternal::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        // Verificăm dacă tasta apăsată este Enter
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            QWidget *currentWidget = QApplication::focusWidget();
            if (!currentWidget)
                return false; // Dacă nu există widget activ, continuăm propagarea

            // Dacă este apăsat Ctrl+Enter sau Shift+Enter, permite introducerea de rând nou
            if (keyEvent->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier)) {
                return false; // Nu preluăm evenimentul, îl lăsăm să ajungă la QPlainTextEdit
            }

            if (currentWidget == ui->cholecist_formations)
                ui->scrollArea_organs_internal->ensureWidgetVisible(ui->spleen_formations);
            else if (currentWidget == ui->spleen_dimens)
                ui->scrollArea_organs_internal->ensureWidgetVisible(ui->organsInternal_recommendation);

            // Trecem la următorul widget în lanțul de focus
            focusNextChild();
            return true; // Marchează evenimentul ca procesat
        }
    }

    return ReportPageBase::eventFilter(obj, event); // Continuăm cu filtrarea normală
}

void ReportPageOrgansInternal::changeEvent(QEvent *event)
{
    ReportPageBase::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
}
