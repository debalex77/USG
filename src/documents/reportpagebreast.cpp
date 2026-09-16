#include "reportpagebreast.h"
#include "ui_reportpagebreast.h"

#include <customs/custommessage.h>

ReportPageBreast::ReportPageBreast(DataBase &db,
                                   QSqlDatabase &currentDB,
                                   QWidget *parent)
    : ReportPageBase(parent)
    , ui(new Ui::ReportPageBreast)
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

ReportPageBreast::~ReportPageBreast()
{
    delete ui;
}

bool ReportPageBreast::loadData(int idReport)
{
    if (idReport <= 0)
        return false;

    QSqlQuery q(m_currentDB);
    q.prepare(R"(SELECT * FROM tableBreast WHERE id_reportEcho = :id_reports)");
    q.bindValue(":id_reports", idReport);
    if(!q.exec()) {
        qWarning(logWarning()).noquote()
        << "ReportPageBreast error:" << q.lastError().text()
        << "\nLast query:" << q.lastQuery();
        return false;
    }

    if (!q.next()) {
        qWarning(logWarning()) << "Nu s-au gasit datele pentru sistema 'breast' !!!";
        return false;
    }

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

    ui->breast_right_ecostructure->setText(q.value("breast_right_ecostrcture").toString());
    ui->breast_right_duct->setText(q.value("breast_right_duct").toString());
    ui->breast_right_ligament->setText(q.value("breast_right_ligament").toString());
    ui->breast_right_formations->setPlainText(q.value("breast_right_formations").toString());
    ui->breast_right_ganglions->setText(q.value("breast_right_ganglions").toString());

    ui->breast_left_ecostructure->setText(q.value("breast_left_ecostrcture").toString());
    ui->breast_left_duct->setText(q.value("breast_left_duct").toString());
    ui->breast_left_ligament->setText(q.value("breast_left_ligament").toString());
    ui->breast_left_formations->setPlainText(q.value("breast_left_formations").toString());
    ui->breast_left_ganglions->setText(q.value("breast_left_ganglions").toString());

    ui->breast_concluzion->setPlainText(q.value("concluzion").toString());

    return true;
}

bool ReportPageBreast::saveData(int idReport)
{
    if (idReport <= 0)
        return false;

    if (existDocument(idReport))
        return updateData(idReport);

    return insertData(idReport);
}

QString ReportPageBreast::concluzionText() const
{
    return ui->breast_concluzion
               ? ui->breast_concluzion->toPlainText().trimmed()
               : QString{};
}

ReportSections::ReportSystem ReportPageBreast::system() const
{
    return ReportSections::ReportSystem::Breast;
}

void ReportPageBreast::handleSelectTemplate()
{
    CatalogTableEditor *dlg = new CatalogTableEditor(m_db,
                                                     CatalogType::FormType::Selection,
                                                     CatalogType::Type::ConclusionTemplates,
                                                     this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setFilterQuery("Gl.mamare");
    connect(dlg, &CatalogTableEditor::dataSelected, this, [this](const QVariantMap &data){
        if (data.isEmpty())
            return;
        ui->breast_concluzion->appendPlainText(data["name"].toString());
    });
    dlg->show();
}

void ReportPageBreast::handleAddTemplate()
{
    /** pregatim variabile locale */
    const QString m_concluzion = ui->breast_concluzion->toPlainText().trimmed();
    const QString m_system     = "Gl.mamare";

    /** verificam daca sunt valorile */
    if (m_concluzion.isEmpty() || m_system.isEmpty())
        return;

    /** inseram datele */
    insertConclusionTemplate(m_concluzion, m_system);
}

void ReportPageBreast::handleSelectFindingsTemplates()
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

        dlg->setFilterQuery("Recomandari (gl.mamare)");
        connect(dlg, &CatalogTableEditor::dataSelected, this, [dlg, this](const QVariantMap &data){
            if (data.isEmpty() || !dlg)
                return;
            ui->breast_recommendation->setText(data["name"].toString());
            dlg->close();
        });
        dlg->show();
    }
}

void ReportPageBreast::handleAddFindingsTemplates()
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
        insertFindigsTemplate(ui->breast_recommendation->text().trimmed(), "Recomandari (gl.mamare)");
    }
}

void ReportPageBreast::initRequiredStructure()
{
    rows_btn_findings = {
        {ui->btnSelectTempletsBreastLeft , ui->btnAddTempletsBreastLeft , ui->breast_left_formations , "Gl.mamara (stanga)" },
        {ui->btnSelectTempletsBreastRight, ui->btnAddTempletsBreastRight, ui->breast_right_formations, "Gl.mamara (dreapta)"}
    };
}

void ReportPageBreast::initInstallEventFilter()
{
    this->installEventFilter(this);

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

void ReportPageBreast::setDefaultContext()
{
    if (ui->breast_right_ecostructure->text().isEmpty())
        ui->breast_right_ecostructure->setText("glandulară, omogenă");
    if (ui->breast_right_duct->text().isEmpty())
        ui->breast_right_duct->setText("norm.");
    if (ui->breast_right_ligament->text().isEmpty())
        ui->breast_right_ligament->setText("norm");
    if (ui->breast_right_formations->toPlainText().isEmpty())
        ui->breast_right_formations->setPlainText("lichidiene, solide abs.");
    if (ui->breast_right_ganglions->text().isEmpty())
        ui->breast_right_ganglions->setText("fără modificări patologice");

    if (ui->breast_left_ecostructure->text().isEmpty())
        ui->breast_left_ecostructure->setText("glandulară, omogenă");
    if (ui->breast_left_duct->text().isEmpty())
        ui->breast_left_duct->setText("norm.");
    if (ui->breast_left_ligament->text().isEmpty())
        ui->breast_left_ligament->setText("norm");
    if (ui->breast_left_formations->toPlainText().isEmpty())
        ui->breast_left_formations->setPlainText("lichidiene, solide abs.");
    if (ui->breast_left_ganglions->text().isEmpty())
        ui->breast_left_ganglions->setText("fără modificări patologice");
}

void ReportPageBreast::setPropertyMaxLengthText()
{
    //--- left
    ui->breast_left_ecostructure->setMaxLength(255);
    ui->breast_left_ecostructure->setPlaceholderText(tr("... maximum 255 caractere"));
    ui->breast_left_duct->setMaxLength(20);
    ui->breast_left_duct->setPlaceholderText(tr("... maximum 20 caractere"));
    ui->breast_left_ligament->setMaxLength(20);
    ui->breast_left_ligament->setPlaceholderText(tr("... maximum 20 caractere"));
    ui->breast_left_ganglions->setMaxLength(300);
    ui->breast_left_ganglions->setPlaceholderText(tr("... maximum 300 caractere"));
    ui->breast_left_formations->setPlaceholderText(tr("... maximum 500 caractere"));

    //--- right
    ui->breast_right_ecostructure->setMaxLength(255);
    ui->breast_right_ecostructure->setPlaceholderText(tr("... maximum 255 caractere"));
    ui->breast_right_duct->setMaxLength(20);
    ui->breast_right_duct->setPlaceholderText(tr("... maximum 20 caractere"));
    ui->breast_right_ligament->setMaxLength(20);
    ui->breast_right_ligament->setPlaceholderText(tr("... maximum 20 caractere"));
    ui->breast_right_ganglions->setMaxLength(300);
    ui->breast_right_ganglions->setPlaceholderText(tr("... maximum 300 caractere"));
    ui->breast_right_formations->setPlaceholderText(tr("... maximum 500 caractere"));

    //--- concluzion
    ui->breast_concluzion->setPlaceholderText(tr("... maximum 500 caractere"));

    //--- recommandation
    ui->breast_recommendation->setPlaceholderText(tr("... maximum 255 caractere"));
    ui->breast_recommendation->setMaxLength(255);
}

void ReportPageBreast::initConnections()
{
    const QList<QLineEdit*> items = this->findChildren<QLineEdit*>();
    for (QLineEdit *item : items)
        connect(item, &QLineEdit::textChanged,
                this, &ReportPageBreast::dataWasModified, Qt::UniqueConnection);

    const QList<QPlainTextEdit*> tes = this->findChildren<QPlainTextEdit*>();
    for (QPlainTextEdit *te : tes)
        connect(te, &QPlainTextEdit::textChanged,
                this, &ReportPageBreast::dataWasModified, Qt::UniqueConnection);

    const QList<QToolButton*> btns = this->findChildren<QToolButton*>();
    for (QToolButton *btn : btns)
        btn->setStyleSheet(toolButtonStyleForText);

    // conclusion (QToolButton) - concluzia
    connect(ui->btn_add_template_breast, &QAbstractButton::clicked,
            this, &ReportPageBreast::handleAddTemplate, Qt::UniqueConnection);
    connect(ui->btn_template_breast, &QAbstractButton::clicked,
            this, &ReportPageBreast::handleSelectTemplate, Qt::UniqueConnection);

    // recomandari
    connect(ui->breast_recommendation->actionAddItem(), &QAction::triggered,
            this, &ReportPageBreast::handleAddFindingsTemplates, Qt::UniqueConnection);
    connect(ui->breast_recommendation->actionOpenList(), &QAction::triggered,
            this, &ReportPageBreast::handleSelectFindingsTemplates, Qt::UniqueConnection);

    // findings (QToolButton + QPlainTextEdit) - descrierea formatiunilor, calculelor etc.
    for (auto &r : rows_btn_findings) {
        connect(r.btn_add, &QToolButton::clicked,
                this, &ReportPageBreast::handleAddFindingsTemplates, Qt::UniqueConnection);
        connect(r.btn_select, &QToolButton::clicked,
                this, &ReportPageBreast::handleSelectFindingsTemplates, Qt::UniqueConnection);
    }

    connect(ui->breast_concluzion, &QPlainTextEdit::textChanged,
            this, [this](){
                emit concluzionTextChanged();
            });
}

bool ReportPageBreast::existDocument(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(R"(
        SELECT EXISTS(
            SELECT 1
            FROM tableBreast
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

void ReportPageBreast::bindFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":breast_right_ecostrcture", ui->breast_right_ecostructure->text());
    q.bindValue(":breast_right_duct", ui->breast_right_duct->text());
    q.bindValue(":breast_right_ligament", ui->breast_right_ligament->text());
    q.bindValue(":breast_right_formations", ui->breast_right_formations->toPlainText());
    q.bindValue(":breast_right_ganglions", ui->breast_right_ganglions->text());
    q.bindValue(":breast_left_ecostrcture", ui->breast_left_ecostructure->text());
    q.bindValue(":breast_left_duct", ui->breast_left_duct->text());
    q.bindValue(":breast_left_ligament", ui->breast_left_ligament->text());
    q.bindValue(":breast_left_formations", ui->breast_left_formations->toPlainText());
    q.bindValue(":breast_left_ganglions", ui->breast_left_ganglions->text());
    q.bindValue(":concluzion", ui->breast_concluzion->toPlainText());
    q.bindValue(":recommendation", ui->breast_recommendation->text().isEmpty()
                        ? QVariant()
                        : ui->breast_recommendation->text());
}

bool ReportPageBreast::insertData(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_breast_insert.sql"));
    bindFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Insert tableBreast error:"
                               << q.lastError().text();
        return false;
    }
    qInfo(logInfo()) << "ReportPageBreast: INSERT executat în tranzacția raportului; id=" << idReport;
    return true;
}

bool ReportPageBreast::updateData(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_breast_update.sql"));
    bindFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Update tableBreast error:"
                               << q.lastError().text();
        return false;
    }
    qInfo(logInfo()) << "ReportPageBreast: UPDATE executat în tranzacția raportului; id=" << idReport;
    return true;
}

void ReportPageBreast::insertConclusionTemplate(const QString m_conclusion, const QString m_system)
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

void ReportPageBreast::insertFindigsTemplate(const QString m_description, const QString m_typeSystem)
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

bool ReportPageBreast::eventFilter(QObject *obj, QEvent *event)
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
        QWidget *contents = ui->scrollArea_breast->widget();
        while (target && target->parentWidget() && target->parentWidget() != contents) {
            target = target->parentWidget();
        }

        if (target && contents->isAncestorOf(target)) {
            QTimer::singleShot(0, this, [this, target]() {
                ui->scrollArea_breast->ensureWidgetVisible(target, 20, 20);
            });
        }
    }

    return ReportPageBase::eventFilter(obj, event);
}

void ReportPageBreast::changeEvent(QEvent *event)
{
    ReportPageBase::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
}
