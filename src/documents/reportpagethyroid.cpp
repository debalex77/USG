#include "reportpagethyroid.h"
#include "ui_reportpagethyroid.h"

#include <QMessageBox>
#include <QTimer>

#include <customs/custommessage.h>

#include <views/catalogtableeditor.h>

ReportPageThyroid::ReportPageThyroid(DataBase &db, QSqlDatabase &currentDB, QWidget *parent)
    : ReportPageBase(parent)
    , ui(new Ui::ReportPageThyroid)
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

ReportPageThyroid::~ReportPageThyroid()
{
    delete ui;
}

bool ReportPageThyroid::loadData(int idReport)
{
    if (idReport <= 0)
        return false;

    QSqlQuery q(m_currentDB);
    q.prepare(R"(SELECT * FROM tableThyroid WHERE id_reportEcho = :id_reports)");
    q.bindValue(":id_reports", idReport);
    if(!q.exec()) {
        qWarning(logWarning()).noquote()
        << "ReportPageThyroid error:" << q.lastError().text()
        << "\nLast query:" << q.lastQuery();
        return false;
    }

    if (!q.next()) {
        qWarning(logWarning()) << "Nu s-au gasit datele pentru sistema 'thyroid' !!!";
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

    ui->thyroid_right_dimens->setText(q.value("thyroid_right_dimens").toString());
    ui->thyroid_right_volum->setText(q.value("thyroid_right_volum").toString());
    ui->thyroid_left_dimens->setText(q.value("thyroid_left_dimens").toString());
    ui->thyroid_left_volum->setText(q.value("thyroid_left_volum").toString());
    ui->thyroid_istm->setText(q.value("thyroid_istm").toString());
    ui->thyroid_ecostructure->setText(q.value("thyroid_ecostructure").toString());
    ui->thyroid_formations->setPlainText(q.value("thyroid_formations").toString());
    ui->thyroid_ganglions->setText(q.value("thyroid_ganglions").toString());
    ui->thyroid_concluzion->setPlainText(q.value("concluzion").toString());
    ui->thyroid_recommendation->setText(q.value("recommendation").toString());

    return true;
}

bool ReportPageThyroid::saveData(int idReport)
{
    if (idReport <= 0)
        return false;

    if (existDocument(idReport))
        return updateData(idReport);
    else
        return insertData(idReport);

    return false;
}

QString ReportPageThyroid::concluzionText() const
{
    return ui->thyroid_concluzion
               ? ui->thyroid_concluzion->toPlainText().trimmed()
               : QString{};
}

ReportSections::ReportSystem ReportPageThyroid::system() const
{
    return ReportSections::ReportSystem::Thyroid;
}

void ReportPageThyroid::handleSelectTemplate()
{
    CatalogTableEditor *dlg = new CatalogTableEditor(m_db,
                                                     CatalogType::FormType::Selection,
                                                     CatalogType::Type::ConclusionTemplates,
                                                     this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setFilterQuery("Tiroida");
    connect(dlg, &CatalogTableEditor::dataSelected, this, [this](const QVariantMap &data){
        if (data.isEmpty())
            return;
        ui->thyroid_concluzion->appendPlainText(data["name"].toString());
    });
    dlg->show();
}

void ReportPageThyroid::handleAddTemplate()
{
    /** pregatim variabile locale */
    const QString m_concluzion = ui->thyroid_concluzion->toPlainText().trimmed();
    const QString m_system     = "Tiroida";

    /** verificam daca sunt valorile */
    if (m_concluzion.isEmpty() || m_system.isEmpty())
        return;

    /** inseram datele */
    insertConclusionTemplate(m_concluzion, m_system);
}

void ReportPageThyroid::handleSelectFindingsTemplates()
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

        dlg->setFilterQuery("Recomandari (tiroida)");
        connect(dlg, &CatalogTableEditor::dataSelected, this, [dlg, this](const QVariantMap &data){
            if (data.isEmpty() || !dlg)
                return;
            ui->thyroid_recommendation->setText(data["name"].toString());
            dlg->close();
        });
        dlg->show();
    }
}

void ReportPageThyroid::handleAddFindingsTemplates()
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
        insertFindigsTemplate(ui->thyroid_recommendation->text().trimmed(), "Recomandari (tiroida)");
    }
}

void ReportPageThyroid::initRequiredStructure()
{
    rows_btn_findings = {
        {ui->btnSelectTempletsThyroid, ui->btnAddTempletsThyroid, ui->thyroid_formations, "Tiroida"}
    };
}

void ReportPageThyroid::initInstallEventFilter()
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

void ReportPageThyroid::setDefaultContext()
{
    if (ui->thyroid_ecostructure->text().isEmpty())
        ui->thyroid_ecostructure->setText("omogenă");
    if (ui->thyroid_formations->toPlainText().isEmpty())
        ui->thyroid_formations->setPlainText("l.drept - formațiuni lichidiene, solide abs.\nl.stâng - formațiuni lichidiene, solide abs.");
    if (ui->thyroid_ganglions->text().isEmpty())
        ui->thyroid_ganglions->setText("fara modificări patologice");
    if (ui->thyroid_recommendation->text().isEmpty())
        ui->thyroid_recommendation->setText("consultația endocrinologului");
}

void ReportPageThyroid::setPropertyMaxLengthText()
{
    //--- items
    ui->thyroid_left_dimens->setMaxLength(20);
    ui->thyroid_left_dimens->setPlaceholderText(tr("... maximum 20 caractere"));
    ui->thyroid_left_volum->setMaxLength(5);
    ui->thyroid_left_volum->setPlaceholderText(tr("... maximum 5 caractere"));
    ui->thyroid_right_dimens->setMaxLength(20);
    ui->thyroid_right_dimens->setPlaceholderText(tr("... maximum 20 caractere"));
    ui->thyroid_right_volum->setMaxLength(5);
    ui->thyroid_right_volum->setPlaceholderText(tr("... maximum 5 caractere"));
    ui->thyroid_istm->setMaxLength(4);
    ui->thyroid_istm->setPlaceholderText(tr("... maximum 4 caractere"));
    ui->thyroid_ecostructure->setMaxLength(20);
    ui->thyroid_ecostructure->setPlaceholderText(tr("... maximum 20 caractere"));
    ui->thyroid_ganglions->setMaxLength(300);
    ui->thyroid_ganglions->setPlaceholderText(tr("... maximum 300 caractere"));
    ui->thyroid_formations->setPlaceholderText(tr("... maximum 500 caractere"));

    //--- concluzion
    ui->thyroid_concluzion->setPlaceholderText(tr("... maximum 500 caractere"));

    //--- recommandation
    ui->thyroid_recommendation->setPlaceholderText(tr("... maximum 255 caractere"));
    ui->thyroid_recommendation->setMaxLength(255);
}

void ReportPageThyroid::initConnections()
{
    const QList<QLineEdit*> items = this->findChildren<QLineEdit*>();
    for (QLineEdit *item : items)
        connect(item, &QLineEdit::textChanged,
                this, &ReportPageThyroid::dataWasModified, Qt::UniqueConnection);

    const QList<QPlainTextEdit*> tes = this->findChildren<QPlainTextEdit*>();
    for (QPlainTextEdit *te : tes)
        connect(te, &QPlainTextEdit::textChanged,
                this, &ReportPageThyroid::dataWasModified, Qt::UniqueConnection);

    const QList<QToolButton*> btns = this->findChildren<QToolButton*>();
    for (QToolButton *btn : btns)
        btn->setStyleSheet(toolButtonStyleForText);

    // conclusion (QToolButton) - concluzia
    connect(ui->btn_add_template_thyroid, &QAbstractButton::clicked,
            this, &ReportPageThyroid::handleAddTemplate, Qt::UniqueConnection);
    connect(ui->btn_template_thyroid, &QAbstractButton::clicked,
            this, &ReportPageThyroid::handleSelectTemplate, Qt::UniqueConnection);

    // recomandari
    connect(ui->thyroid_recommendation->actionAddItem(), &QAction::triggered,
            this, &ReportPageThyroid::handleAddFindingsTemplates, Qt::UniqueConnection);
    connect(ui->thyroid_recommendation->actionOpenList(), &QAction::triggered,
            this, &ReportPageThyroid::handleSelectFindingsTemplates, Qt::UniqueConnection);

    // findings (QToolButton + QPlainTextEdit) - descrierea formatiunilor, calculelor etc.
    for (auto &r : rows_btn_findings) {
        connect(r.btn_add, &QToolButton::clicked,
                this, &ReportPageThyroid::handleAddFindingsTemplates, Qt::UniqueConnection);
        connect(r.btn_select, &QToolButton::clicked,
                this, &ReportPageThyroid::handleSelectFindingsTemplates, Qt::UniqueConnection);
    }

    connect(ui->thyroid_concluzion, &QPlainTextEdit::textChanged,
            this, [this](){
                emit concluzionTextChanged();
            });
}

bool ReportPageThyroid::existDocument(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(R"(
        SELECT EXISTS(
            SELECT 1
            FROM tableThyroid
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

void ReportPageThyroid::bindFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho",        idReport);
    q.bindValue(":thyroid_right_dimens", ui->thyroid_right_dimens->text());
    q.bindValue(":thyroid_right_volum",  ui->thyroid_right_volum->text());
    q.bindValue(":thyroid_left_dimens",  ui->thyroid_left_dimens->text());
    q.bindValue(":thyroid_left_volum",   ui->thyroid_left_volum->text());
    q.bindValue(":thyroid_istm",         ui->thyroid_istm->text());
    q.bindValue(":thyroid_ecostructure", ui->thyroid_ecostructure->text());
    q.bindValue(":thyroid_formations",   ui->thyroid_formations->toPlainText());
    q.bindValue(":thyroid_ganglions",    ui->thyroid_ganglions->text());
    q.bindValue(":concluzion",           ui->thyroid_concluzion->toPlainText());
    q.bindValue(":recommendation", ui->thyroid_recommendation->text().isEmpty()
                                        ? QVariant()
                                        : ui->thyroid_recommendation->text());
}

bool ReportPageThyroid::insertData(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_thyroid_insert.sql"));
    bindFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Insert tableThyroid error:"
                               << q.lastError().text();
        return false;
    }
    qInfo(logInfo()) << "ReportPageThyroid: INSERT executat în tranzacția raportului; id=" << idReport;
    return true;
}

bool ReportPageThyroid::updateData(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_thyroid_update.sql"));
    bindFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Update tableThyroid error:"
                               << q.lastError().text();
        return false;
    }
    qInfo(logInfo()) << "ReportPageThyroid: UPDATE executat în tranzacția raportului; id=" << idReport;
    return true;
}

void ReportPageThyroid::insertConclusionTemplate(const QString m_conclusion, const QString m_system)
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

void ReportPageThyroid::insertFindigsTemplate(const QString m_description, const QString m_typeSystem)
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

bool ReportPageThyroid::eventFilter(QObject *obj, QEvent *event)
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
        QWidget *contents = ui->scrollArea_thyroid->widget();
        while (target && target->parentWidget() && target->parentWidget() != contents) {
            target = target->parentWidget();
        }

        if (target && contents->isAncestorOf(target)) {
            QTimer::singleShot(0, this, [this, target]() {
                ui->scrollArea_thyroid->ensureWidgetVisible(target, 20, 20);
            });
        }
    }

    return ReportPageBase::eventFilter(obj, event);
}

void ReportPageThyroid::changeEvent(QEvent *event)
{
    ReportPageBase::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
}
