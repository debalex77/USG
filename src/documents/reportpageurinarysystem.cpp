#include "reportpageurinarysystem.h"
#include "ui_reportpageurinarysystem.h"

#include <customs/custommessage.h>

ReportPageUrinarySystem::ReportPageUrinarySystem(DataBase &db,
                                                 QSqlDatabase &currentDB,
                                                 QWidget *parent)
    : ReportPageBase(parent)
    , ui(new Ui::ReportPageUrinarySystem)
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

ReportPageUrinarySystem::~ReportPageUrinarySystem()
{
    delete ui;
}

bool ReportPageUrinarySystem::loadData(int idReport)
{
    if (idReport <= 0)
        return false;

    QSqlQuery q(m_currentDB);
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_urinarysystem_select.sql"));
    q.bindValue(":id_reports", idReport);
    if(!q.exec()) {
        qWarning(logWarning()).noquote()
        << "ReportPageUrinarySystem error:" << q.lastError().text()
        << "\nLast query:" << q.lastQuery();
        return false;
    }

    if (!q.next()) {
        qWarning(logWarning()) << "Nu s-au gasit datele pentru sistema 'urinarySystem' !!!";
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

    // kidney
    ui->kidney_contur_right->setCurrentText(q.value("kidney_contour_right").toString());
    ui->kidney_contur_left->setCurrentText(q.value("kidney_contour_left").toString());
    ui->kidney_right->setText(q.value("kidney_dimens_right").toString());
    ui->kidney_left->setText(q.value("kidney_dimens_left").toString());
    ui->kidney_corticomed_right->setText(q.value("kidney_corticomed_right").toString());
    ui->kidney_corticomed_left->setText(q.value("kidney_corticomed_left").toString());
    ui->kidney_pielocaliceal_right->setText(q.value("kidney_pielocaliceal_right").toString());
    ui->kidney_pielocaliceal_left->setText(q.value("kidney_pielocaliceal_left").toString());
    ui->kidney_formations->setText(q.value("kidney_formations").toString());
    ui->adrenalGlands->setPlainText(q.value("kidney_suprarenal_formations").toString());
    ui->urinary_system_concluzion->setPlainText(q.value("kidney_concluzion").toString());
    ui->urinary_system_recommendation->setText(q.value("kidney_recommendation").toString());
    // bladder
    ui->bladder_volum->setText(q.value("bladder_volum").toString());
    ui->bladder_walls->setText(q.value("bladder_walls").toString());
    ui->bladder_formations->setText(q.value("bladder_formations").toString());

    return true;
}

bool ReportPageUrinarySystem::saveData(int idReport)
{
    if (idReport <= 0)
        return false;

    if (existDocument(idReport))
        return updateData(idReport);
    else
        return insertData(idReport);

    return false;
}

QString ReportPageUrinarySystem::concluzionText() const
{
    return ui->urinary_system_concluzion
               ? ui->urinary_system_concluzion->toPlainText().trimmed()
               : QString{};
}

ReportSections::ReportSystem ReportPageUrinarySystem::system() const
{
    return ReportSections::ReportSystem::UrinarySystem;
}

void ReportPageUrinarySystem::handleSelectTemplate()
{
    CatalogTableEditor *dlg = new CatalogTableEditor(m_db,
                                                     CatalogType::FormType::Selection,
                                                     CatalogType::Type::ConclusionTemplates,
                                                     this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setFilterQuery("Sistemul urinar");
    connect(dlg, &CatalogTableEditor::dataSelected, this, [this](const QVariantMap &data){
        if (data.isEmpty())
            return;
        ui->urinary_system_concluzion->appendPlainText(data["name"].toString());
    });
    dlg->show();
}

void ReportPageUrinarySystem::handleAddTemplate()
{
    /** pregatim variabile locale */
    const QString m_concluzion = ui->urinary_system_concluzion->toPlainText().trimmed();
    const QString m_system     = "Sistemul urinar";

    /** verificam daca sunt valorile */
    if (m_concluzion.isEmpty() || m_system.isEmpty())
        return;

    /** inseram datele */
    insertConclusionTemplate(m_concluzion, m_system);
}

void ReportPageUrinarySystem::handleSelectFindingsTemplates()
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
     ** doar 'ui->adrenalGlands'                       */
    if (auto btn = qobject_cast<QAbstractButton*>(s)) {

        Q_UNUSED(btn);

        dlg->setFilterQuery("Gl.suprarenale");
        connect(dlg, &CatalogTableEditor::dataSelected, this, [dlg, this](const QVariantMap &data){
            if (data.isEmpty() || !dlg)
                return;
            ui->adrenalGlands->setPlainText(data["name"].toString());
            dlg->close();
        });
        dlg->show();
    }
}

void ReportPageUrinarySystem::handleAddFindingsTemplates()
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
     ** doar 'ui->adrenalGlands'                       */
    if (auto btn = qobject_cast<QAbstractButton*>(s)) {
        Q_UNUSED(btn);
        insertFindigsTemplate(ui->adrenalGlands->toPlainText().trimmed(), "Gl.suprarenale");
    }
}

void ReportPageUrinarySystem::initRequiredStructure()
{
    rows_action_findings = {
        {ui->kidney_formations->actionOpenList(), ui->kidney_formations->actionAddItem(), ui->kidney_formations, "Rinichi"},
        {ui->bladder_formations->actionOpenList(), ui->bladder_formations->actionAddItem(), ui->bladder_formations, "V.urinara"},
        {ui->urinary_system_recommendation->actionOpenList(), ui->urinary_system_recommendation->actionAddItem(), ui->urinary_system_recommendation, "Recomandari (s.urinar)"}
    };
}

void ReportPageUrinarySystem::initInstallEventFilter()
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

void ReportPageUrinarySystem::setDefaultContext()
{
    //--- kidney
    if (ui->kidney_formations->text().isEmpty())
        ui->kidney_formations->setText("solide, lichide abs., sectoare hiperecogene 2-3 mm fără umbră acustică");
    if (ui->kidney_pielocaliceal_left->text().isEmpty())
        ui->kidney_pielocaliceal_left->setText("nu este dilatat");
    if (ui->kidney_pielocaliceal_right->text().isEmpty())
        ui->kidney_pielocaliceal_right->setText("nu este dilatat");
    if (ui->adrenalGlands->toPlainText().isEmpty())
        ui->adrenalGlands->setPlainText("nu sunt vizibile ecografic");

    //--- bladder
    if (ui->bladder_formations->text().isEmpty())
        ui->bladder_formations->setText("diverticuli, calculi abs.");
}

void ReportPageUrinarySystem::setPropertyMaxLengthText()
{
    //--- kidney
    ui->kidney_right->setMaxLength(15);
    ui->kidney_left->setMaxLength(15);
    ui->kidney_corticomed_left->setMaxLength(5);
    ui->kidney_corticomed_right->setMaxLength(5);
    ui->kidney_pielocaliceal_left->setMaxLength(30);
    ui->kidney_pielocaliceal_right->setMaxLength(30);
    ui->kidney_formations->setMaxLength(500);
    ui->kidney_right->setPlaceholderText(tr("...maximum 15 caractere"));
    ui->kidney_left->setPlaceholderText(tr("...maximum 15 caractere"));
    ui->kidney_corticomed_left->setPlaceholderText(tr("...maximum 5 caractere"));
    ui->kidney_corticomed_right->setPlaceholderText(tr("...maximum 5 caractere"));
    ui->kidney_pielocaliceal_left->setPlaceholderText(tr("...maximum 30 caractere"));
    ui->kidney_pielocaliceal_right->setPlaceholderText(tr("...maximum 30 caractere"));
    ui->kidney_formations->setPlaceholderText(tr("...maximum 500 caractere"));

    //--- adrenal glands
    ui->adrenalGlands->setPlaceholderText(tr("... maximum 500 caractere"));

    //--- bladder
    ui->bladder_volum->setMaxLength(5);
    ui->bladder_walls->setMaxLength(5);
    ui->bladder_formations->setMaxLength(300);
    ui->bladder_volum->setPlaceholderText(tr("...maximum 5 caractere"));
    ui->bladder_walls->setPlaceholderText(tr("...maximum 5 caractere"));
    ui->bladder_formations->setPlaceholderText(tr("...maximum 300 caractere"));

    //--- concluzion
    ui->urinary_system_concluzion->setPlaceholderText(tr("... maximum 500 caractere"));

    //--- recommandation
    ui->urinary_system_recommendation->setPlaceholderText(tr("...maximum 255 caractere"));
    ui->urinary_system_recommendation->setMaxLength(255);
}

void ReportPageUrinarySystem::initConnections()
{
    // QLineEdit
    const QList<QLineEdit*> items = this->findChildren<QLineEdit*>();
    for (QLineEdit *item : items)
        connect(item, &QLineEdit::textChanged,
                this, &ReportPageUrinarySystem::dataWasModified, Qt::UniqueConnection);

    // QComboBox
    const QList<QComboBox*> combos = this->findChildren<QComboBox*>();
    for (QComboBox *combo: combos)
        connect(combo, &QComboBox::currentTextChanged,
                this, &ReportPageUrinarySystem::dataWasModified, Qt::UniqueConnection);

    // QPlainTextEdit
    const QList<QPlainTextEdit*> tes = this->findChildren<QPlainTextEdit*>();
    for (QPlainTextEdit *te : tes)
        connect(te, &QPlainTextEdit::textChanged,
                this, &ReportPageUrinarySystem::dataWasModified, Qt::UniqueConnection);

    // Stil
    const QList<QToolButton*> btns = this->findChildren<QToolButton*>();
    for (QToolButton *btn : btns)
        btn->setStyleSheet(toolButtonStyleForText);

    //--- conclusion (QToolButton) - concluzia
    connect(ui->btnAddTemplatesConclusion, &QAbstractButton::clicked,
            this, &ReportPageUrinarySystem::handleAddTemplate, Qt::UniqueConnection);
    connect(ui->btnSelectTemplatesConclusion, &QAbstractButton::clicked,
            this, &ReportPageUrinarySystem::handleSelectTemplate, Qt::UniqueConnection);

    //--- findings (QToolButton) - adrenas gland
    connect(ui->btnAddTemplatesAdrenalGlands, &QAbstractButton::clicked,
            this, &ReportPageUrinarySystem::handleAddFindingsTemplates, Qt::UniqueConnection);
    connect(ui->btnSelectTemplatesAdrenalGlands, &QAbstractButton::clicked,
            this, &ReportPageUrinarySystem::handleSelectFindingsTemplates, Qt::UniqueConnection);

    //--- findings (QAction + LineEditCustom) - descrierea formatiunilor, calculelor etc.
    for (auto &r : rows_action_findings) {
        connect(r.action_add, &QAction::triggered,
                this, &ReportPageUrinarySystem::handleAddFindingsTemplates, Qt::UniqueConnection);
        connect(r.action_select, &QAction::triggered,
                this, &ReportPageUrinarySystem::handleSelectFindingsTemplates, Qt::UniqueConnection);
    }

    //
    connect(ui->urinary_system_concluzion, &QPlainTextEdit::textChanged,
            this, [this](){
                emit concluzionTextChanged();
            });
}

bool ReportPageUrinarySystem::existDocument(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(R"(
        SELECT EXISTS(
            SELECT 1
            FROM tableKidney
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

void ReportPageUrinarySystem::bindKidneyFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho",         idReport);
    q.bindValue(":contour_right",         ui->kidney_contur_right->currentText());
    q.bindValue(":contour_left",          ui->kidney_contur_left->currentText());
    q.bindValue(":dimens_right",          ui->kidney_right->text());
    q.bindValue(":dimens_left",           ui->kidney_left->text());
    q.bindValue(":corticomed_right",      ui->kidney_corticomed_right->text());
    q.bindValue(":corticomed_left",       ui->kidney_corticomed_left->text());
    q.bindValue(":pielocaliceal_right",   ui->kidney_pielocaliceal_right->text());
    q.bindValue(":pielocaliceal_left",    ui->kidney_pielocaliceal_left->text());
    q.bindValue(":formations",            ui->kidney_formations->text());
    q.bindValue(":suprarenal_formations", ui->adrenalGlands->toPlainText());
    q.bindValue(":concluzion",            ui->urinary_system_concluzion->toPlainText());
    q.bindValue(":recommendation", ui->urinary_system_recommendation->text().isEmpty()
                                       ? QVariant()
                                       : ui->urinary_system_recommendation->text());
}

void ReportPageUrinarySystem::bindBladderFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":volum",         ui->bladder_volum->text());
    q.bindValue(":walls",         ui->bladder_walls->text());
    q.bindValue(":formations",    ui->bladder_formations->text());
}

bool ReportPageUrinarySystem::insertData(const int idReport)
{
    QSqlQuery q(m_currentDB);

    // kidney
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_kidney_insert.sql"));
    bindKidneyFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Insert tableProstate error:"
                               << q.lastError().text();
        return false;
    }

    // bladder
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_bladder_insert.sql"));
    bindBladderFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Insert tableProstate error:"
                               << q.lastError().text();
        return false;
    }

    qInfo(logInfo()) << "ReportPageUrinarySystem: INSERT executat în tranzacția raportului; id=" << idReport;
    return true;
}

bool ReportPageUrinarySystem::updateData(const int idReport)
{
    QSqlQuery q(m_currentDB);

    // kidney
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_kidney_update.sql"));
    bindKidneyFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Update tableProstate error:"
                               << q.lastError().text();
        return false;
    }

    // bladder
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_bladder_update.sql"));
    bindBladderFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Update tableProstate error:"
                               << q.lastError().text();
        return false;
    }

    qInfo(logInfo()) << "ReportPageUrinarySystem: UPDATE executat în tranzacția raportului; id=" << idReport;
    return true;
}

void ReportPageUrinarySystem::insertConclusionTemplate(const QString m_conclusion, const QString m_system)
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

void ReportPageUrinarySystem::insertFindigsTemplate(const QString m_description, const QString m_typeSystem)
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

bool ReportPageUrinarySystem::eventFilter(QObject *obj, QEvent *event)
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
        QWidget *contents = ui->scrollArea_urinary_system->widget();
        while (target && target->parentWidget() && target->parentWidget() != contents) {
            target = target->parentWidget();
        }

        if (target && contents->isAncestorOf(target)) {
            QTimer::singleShot(0, this, [this, target]() {
                ui->scrollArea_urinary_system->ensureWidgetVisible(target, 20, 20);
            });
        }
    }

    return ReportPageBase::eventFilter(obj, event);
}

void ReportPageUrinarySystem::changeEvent(QEvent *event)
{
    ReportPageBase::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
}
