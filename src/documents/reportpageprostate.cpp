#include "reportpageprostate.h"
#include "ui_reportpageprostate.h"

#include <customs/custommessage.h>

ReportPageProstate::ReportPageProstate(DataBase &db,
                                       QSqlDatabase &currentDB,
                                       QWidget *parent)
    : ReportPageBase(parent)
    , ui(new Ui::ReportPageProstate)
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

ReportPageProstate::~ReportPageProstate()
{
    delete ui;
}

bool ReportPageProstate::loadData(int idReport)
{
    if (idReport <= 0)
        return false;

    QSqlQuery q(m_currentDB);
    q.prepare(R"(SELECT * FROM tableProstate WHERE id_reportEcho = :id_reports)");
    q.bindValue(":id_reports", idReport);
    if(!q.exec()) {
        qWarning(logWarning()).noquote()
        << "ReportPageProstate error:" << q.lastError().text()
        << "\nLast query:" << q.lastQuery();
        return false;
    }

    if (!q.next()) {
        qWarning(logWarning()) << "Nu s-au gasit datele pentru sistema 'prostate' !!!";
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

    ui->prostate_radioBtn_transrectal->setChecked(q.value("transrectal").toInt());
    ui->prostate_radioBtn_transabdom->setChecked(!q.value("transrectal").toInt());
    ui->prostate_dimens->setText(q.value("dimens").toString());
    ui->prostate_volum->setText(q.value("volume").toString());
    ui->prostate_contur->setText(q.value("contour").toString());
    ui->prostate_ecostructure->setText(q.value("ecostructure").toString());
    ui->prostate_ecogency->setText(q.value("ecogency").toString());
    ui->prostate_formations->setText(q.value("formations").toString());
    ui->prostate_concluzion->setPlainText(q.value("concluzion").toString());
    ui->prostate_recommendation->setText(q.value("recommendation").toString());

    return true;
}

bool ReportPageProstate::saveData(int idReport)
{
    if (idReport <= 0)
        return false;

    if (existDocument(idReport))
        return updateData(idReport);
    else
        return insertData(idReport);

    return false;
}

QString ReportPageProstate::concluzionText() const
{
    return ui->prostate_concluzion
               ? ui->prostate_concluzion->toPlainText().trimmed()
               : QString{};
}

ReportSections::ReportSystem ReportPageProstate::system() const
{
    return ReportSections::ReportSystem::Prostate;
}

bool ReportPageProstate::isTransrectal() const
{
    return ui->prostate_radioBtn_transrectal->isChecked();
}

void ReportPageProstate::handleSelectTemplate()
{
    CatalogTableEditor *dlg = new CatalogTableEditor(m_db,
                                                     CatalogType::FormType::Selection,
                                                     CatalogType::Type::ConclusionTemplates,
                                                     this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setFilterQuery("Prostata");
    connect(dlg, &CatalogTableEditor::dataSelected, this, [this](const QVariantMap &data){
        if (data.isEmpty())
            return;
        ui->prostate_concluzion->appendPlainText(data["name"].toString());
    });
    dlg->show();
}

void ReportPageProstate::handleAddTemplate()
{
    /** pregatim variabile locale */
    const QString m_concluzion = ui->prostate_concluzion->toPlainText().trimmed();
    const QString m_system     = "Prostata";

    /** verificam daca sunt valorile */
    if (m_concluzion.isEmpty() || m_system.isEmpty())
        return;

    /** inseram datele */
    insertConclusionTemplate(m_concluzion, m_system);
}

void ReportPageProstate::handleSelectFindingsTemplates()
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

void ReportPageProstate::handleAddFindingsTemplates()
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

void ReportPageProstate::initRequiredStructure()
{
    rows_action_findings = {
        {ui->prostate_formations->actionOpenList(), ui->prostate_formations->actionAddItem(), ui->prostate_formations, "Prostata"},
        {ui->prostate_recommendation->actionOpenList(), ui->prostate_recommendation->actionAddItem(), ui->prostate_recommendation, "Recomandari (prostata)"}
    };
}

void ReportPageProstate::initInstallEventFilter()
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

void ReportPageProstate::setDefaultContext()
{
    ui->prostate_radioBtn_transabdom->setChecked(true);

    if (ui->prostate_contur->text().isEmpty())
        ui->prostate_contur->setText("clar");
    if (ui->prostate_ecogency->text().isEmpty())
        ui->prostate_ecogency->setText("scăzută");
    if (ui->prostate_ecostructure->text().isEmpty())
        ui->prostate_ecostructure->setText("omogenă");
    if (ui->prostate_formations->text().isEmpty())
        ui->prostate_formations->setText("abs.");
    if (ui->prostate_recommendation->text().isEmpty())
        ui->prostate_recommendation->setText("consultația urologului");
}

void ReportPageProstate::setPropertyMaxLengthText()
{
    ui->prostate_dimens->setMaxLength(25);
    ui->prostate_volum->setMaxLength(5);
    ui->prostate_ecostructure->setMaxLength(30);
    ui->prostate_contur->setMaxLength(20);
    ui->prostate_ecogency->setMaxLength(30);
    ui->prostate_formations->setMaxLength(300);
    ui->prostate_dimens->setPlaceholderText(tr("...maximum 25 caractere"));
    ui->prostate_volum->setPlaceholderText(tr("...maximum 5 caractere"));
    ui->prostate_ecostructure->setPlaceholderText(tr("...maximum 30 caractere"));
    ui->prostate_contur->setPlaceholderText(tr("...maximum 20 caractere"));
    ui->prostate_ecogency->setPlaceholderText(tr("...maximum 30 caractere"));
    ui->prostate_formations->setPlaceholderText(tr("...maximum 300 caractere"));

    //--- concluzion
    ui->prostate_concluzion->setPlaceholderText(tr("... maximum 500 caractere"));

    //--- recommandation
    ui->prostate_recommendation->setPlaceholderText(tr("... maximum 255 caractere"));
    ui->prostate_recommendation->setMaxLength(255);
}

void ReportPageProstate::initConnections()
{
    const QList<QLineEdit*> items = this->findChildren<QLineEdit*>();
    for (QLineEdit *item : items)
        connect(item, &QLineEdit::textChanged,
                this, &ReportPageProstate::dataWasModified, Qt::UniqueConnection);

    const QList<QPlainTextEdit*> tes = this->findChildren<QPlainTextEdit*>();
    for (QPlainTextEdit *te : tes)
        connect(te, &QPlainTextEdit::textChanged,
                this, &ReportPageProstate::dataWasModified, Qt::UniqueConnection);

    const QList<QToolButton*> btns = this->findChildren<QToolButton*>();
    for (QToolButton *btn : btns)
        btn->setStyleSheet(toolButtonStyleForText);

    //--- conclusion (QToolButton) - concluzia
    connect(ui->btn_add_template_prostate, &QAbstractButton::clicked,
            this, &ReportPageProstate::handleAddTemplate, Qt::UniqueConnection);
    connect(ui->btn_template_prostate, &QAbstractButton::clicked,
            this, &ReportPageProstate::handleSelectTemplate, Qt::UniqueConnection);

    //--- findings (QAction + LineEditCustom) - descrierea formatiunilor, calculelor etc.
    for (auto &r : rows_action_findings) {
        connect(r.action_add, &QAction::triggered,
                this, &ReportPageProstate::handleAddFindingsTemplates, Qt::UniqueConnection);
        connect(r.action_select, &QAction::triggered,
                this, &ReportPageProstate::handleSelectFindingsTemplates, Qt::UniqueConnection);
    }

    connect(ui->prostate_concluzion, &QPlainTextEdit::textChanged,
            this, [this](){
                emit concluzionTextChanged();
            });
}

bool ReportPageProstate::existDocument(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(R"(
        SELECT EXISTS(
            SELECT 1
            FROM tableProstate
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

void ReportPageProstate::bindFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":dimens",        ui->prostate_dimens->text());
    q.bindValue(":volume",        ui->prostate_volum->text());
    q.bindValue(":ecostructure",  ui->prostate_ecostructure->text());
    q.bindValue(":contour",       ui->prostate_contur->text());
    q.bindValue(":ecogency",      ui->prostate_ecogency->text());
    q.bindValue(":formations",    ui->prostate_formations->text());
    q.bindValue(":transrectal", globals().thisMySQL
                         ? QVariant(ui->prostate_radioBtn_transrectal->isChecked())
                         : QVariant(int(ui->prostate_radioBtn_transrectal->isChecked())));
    q.bindValue(":concluzion",     ui->prostate_concluzion->toPlainText());
    q.bindValue(":recommendation", ui->prostate_recommendation->text().isEmpty()
                                    ? QVariant()
                                    : ui->prostate_recommendation->text());
}

bool ReportPageProstate::insertData(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_prostate_insert.sql"));
    bindFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Insert tableProstate error:"
                               << q.lastError().text();
        return false;
    }
    qInfo(logInfo()) << "ReportPageProstate: INSERT executat în tranzacția raportului; id=" << idReport;
    return true;
}

bool ReportPageProstate::updateData(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_prostate_update.sql"));
    bindFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Update tableProstate error:"
                               << q.lastError().text();
        return false;
    }
    qInfo(logInfo()) << "ReportPageProstate: UPDATE executat în tranzacția raportului; id=" << idReport;
    return true;
}

void ReportPageProstate::insertConclusionTemplate(const QString m_conclusion, const QString m_system)
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

void ReportPageProstate::insertFindigsTemplate(const QString m_description, const QString m_typeSystem)
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

bool ReportPageProstate::eventFilter(QObject *obj, QEvent *event)
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

            // Trecem la următorul widget în lanțul de focus
            focusNextChild();
            return true; // Marchează evenimentul ca procesat
        }
    }

    return ReportPageBase::eventFilter(obj, event); // Continuăm cu filtrarea normală
}

void ReportPageProstate::changeEvent(QEvent *event)
{
    ReportPageBase::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
}
