#include "reportpagegestation0.h"
#include "ui_reportpagegestation0.h"

#include <QMessageBox>

#include <customs/custommessage.h>

#include <views/catalogtableeditor.h>

ReportPageGestation0::ReportPageGestation0(DataBase &db,
                                           QSqlDatabase &currentDB,
                                           QWidget *parent)
    : ReportPageBase(parent)
    , ui(new Ui::ReportPageGestation0)
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

ReportPageGestation0::~ReportPageGestation0()
{
    delete ui;
}

bool ReportPageGestation0::loadData(int idReport)
{
    if (idReport <= 0)
        return false;

    QSqlQuery q(m_currentDB);
    q.prepare(R"(SELECT * FROM tableGestation0 WHERE id_reportEcho = :id_reports)");
    q.bindValue(":id_reports", idReport);
    if(!q.exec()) {
        qWarning(logWarning()).noquote()
        << "ReportPageGestation0 error:" << q.lastError().text()
        << "\nLast query:" << q.lastQuery();
        return false;
    }

    if (!q.next()) {
        qWarning(logWarning()) << "Nu s-au gasit datele pentru sistema 'gestation0' !!!";
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

    switch (q.value("view_examination").toInt()) {
    case 1:
        ui->gestation0_view_good->setChecked(true);
        break;
    case 2:
        ui->gestation0_view_medium->setChecked(true);
        break;
    case 3:
        ui->gestation0_view_difficult->setChecked(true);
        break;
    }
    ui->gestation0_antecedent->setText(q.value("antecedent").toString());
    ui->gestation0_LMP->setDate(QDate::fromString(q.value("lmp").toString(), "yyyy-MM-dd"));
    ui->gestation0_gestation->setText(q.value("gestation_age").toString());
    ui->gestation0_GS_dimens->setText(q.value("GS").toString());
    ui->gestation0_GS_age->setText(q.value("GS_age").toString());
    ui->gestation0_CRL_dimens->setText(q.value("CRL").toString());
    ui->gestation0_CRL_age->setText(q.value("CRL_age").toString());
    ui->gestation0_BCF->setText(q.value("BCF").toString());
    ui->gestation0_liquid_amniotic->setText(q.value("liquid_amniotic").toString());
    ui->gestation0_miometer->setText(q.value("miometer").toString());
    ui->gestation0_cervix->setText(q.value("cervix").toString());
    ui->gestation0_ovary->setText(q.value("ovary").toString());
    ui->gestation0_concluzion->setPlainText(q.value("concluzion").toString());
    ui->gestation0_recommendation->setText(q.value("recommendation").toString());

    calculateGestationalAge(ui->gestation0_LMP->date());
    calculateDueDate(ui->gestation0_LMP->date());

    return true;
}

bool ReportPageGestation0::saveData(int idReport)
{
    if (idReport <= 0)
        return false;

    if (existDocument(idReport))
        return updateData(idReport);
    else
        return insertData(idReport);
}

QString ReportPageGestation0::concluzionText() const
{
    return ui->gestation0_concluzion
               ? ui->gestation0_concluzion->toPlainText().trimmed()
               : QString{};
}

ReportSections::ReportSystem ReportPageGestation0::system() const
{
    return ReportSections::ReportSystem::Gestation0;
}

QDate ReportPageGestation0::LMP() const
{
    return ui->gestation0_LMP->date();
}

QDate ReportPageGestation0::probableDateBirth() const
{
    return ui->gestation0_probableDateBirth->date();
}

ReportSections::ViewExamination ReportPageGestation0::getViewExamination() const
{
    if (ui->gestation0_view_good->isChecked())
        return ReportSections::ViewExamination::Good;
    if (ui->gestation0_view_medium->isChecked())
        return ReportSections::ViewExamination::Medium;
    if (ui->gestation0_view_difficult->isChecked())
        return ReportSections::ViewExamination::Difficult;
    return ReportSections::ViewExamination::Unknown;
}

void ReportPageGestation0::onDateLMPChanged()
{
    QString str_vg = calculateGestationalAge(ui->gestation0_LMP->date());
    if (str_vg == nullptr)
        return;
    ui->gestation0_gestation->setText(str_vg);
    ui->gestation0_probableDateBirth->setDate(calculateDueDate(ui->gestation0_LMP->date()));
}

void ReportPageGestation0::handleSelectTemplate()
{
    CatalogTableEditor *dlg = new CatalogTableEditor(m_db,
                                                     CatalogType::FormType::Selection,
                                                     CatalogType::Type::ConclusionTemplates,
                                                     this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setFilterQuery("Sarcina până la 11 săptămâni");
    connect(dlg, &CatalogTableEditor::dataSelected, this, [this](const QVariantMap &data){
        if (data.isEmpty())
            return;
        ui->gestation0_concluzion->appendPlainText(data["name"].toString());
    });
    dlg->show();
}

void ReportPageGestation0::handleAddTemplate()
{
    /** pregatim variabile locale */
    const QString m_concluzion = ui->gestation0_concluzion->toPlainText().trimmed();
    const QString m_system     = "Sarcina până la 11 săptămâni";

    /** verificam daca sunt valorile */
    if (m_concluzion.isEmpty() || m_system.isEmpty())
        return;

    /** inseram datele */
    insertConclusionTemplate(m_concluzion, m_system);
}

void ReportPageGestation0::handleSelectFindingsTemplates()
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

void ReportPageGestation0::handleAddFindingsTemplates()
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

void ReportPageGestation0::initRequiredStructure()
{
    rows_action_findings = {
        {ui->gestation0_recommendation->actionOpenList(), ui->gestation0_recommendation->actionAddItem(), ui->gestation0_recommendation, "Recomandari (gestatation0)"}
    };
}

void ReportPageGestation0::initInstallEventFilter()
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

void ReportPageGestation0::setDefaultContext()
{
    ui->gestation0_view_medium->setChecked(true);
    ui->gestation0_LMP->setDate(QDate::currentDate());

    if (ui->gestation0_antecedent->text().isEmpty())
        ui->gestation0_antecedent->setText("abs.");
    if (ui->gestation0_BCF->text().isEmpty())
        ui->gestation0_BCF->setText("prezenți, ritmici");
    if (ui->gestation0_liquid_amniotic->text().isEmpty())
        ui->gestation0_liquid_amniotic->setText("omogen, transparent");
    if (ui->gestation0_miometer->text().isEmpty())
        ui->gestation0_miometer->setText("omogen; formațiuni solide, lichidiene abs.");
    if (ui->gestation0_cervix->text().isEmpty())
        ui->gestation0_cervix->setText("omogen; formațiuni solide, lichidiene abs.; închis, lungimea 32,9 mm");
    if (ui->gestation0_ovary->text().isEmpty())
        ui->gestation0_ovary->setText("aspect ecografic normal");
    if (ui->gestation0_recommendation->text().isEmpty())
        ui->gestation0_recommendation->setText("consultația ginecologului, examen ecografic la 11-14 săptămâni a sarcinei");
}

void ReportPageGestation0::setPropertyMaxLengthText()
{
    ui->gestation0_gestation->setInputMask("99s. 9z.");
    ui->gestation0_GS_age->setInputMask("99s. 9z.");
    ui->gestation0_CRL_age->setInputMask("99s. 9z.");

    ui->gestation0_antecedent->setMaxLength(150);
    ui->gestation0_antecedent->setPlaceholderText(tr("... maximum 150 caractere"));
    ui->gestation0_gestation->setMaxLength(20);
    ui->gestation0_GS_dimens->setMaxLength(5);
    ui->gestation0_GS_age->setMaxLength(20);
    ui->gestation0_CRL_dimens->setMaxLength(5);
    ui->gestation0_CRL_age->setMaxLength(20);
    ui->gestation0_BCF->setMaxLength(30);
    ui->gestation0_liquid_amniotic->setMaxLength(40);
    ui->gestation0_liquid_amniotic->setPlaceholderText(tr("... maximum 40 caractere"));
    ui->gestation0_miometer->setMaxLength(200);
    ui->gestation0_miometer->setPlaceholderText(tr("... maximum 200 caractere"));
    ui->gestation0_cervix->setMaxLength(200);
    ui->gestation0_cervix->setPlaceholderText(tr("... maximum 200 caractere"));
    ui->gestation0_ovary->setMaxLength(200);
    ui->gestation0_ovary->setPlaceholderText(tr("... maximum 200 caractere"));

    //--- concluzion
    ui->gestation0_concluzion->setPlaceholderText(tr("... maximum 500 caractere"));

    //--- recommandation
    ui->gestation0_recommendation->setPlaceholderText(tr("... maximum 255 caractere"));
    ui->gestation0_recommendation->setMaxLength(255);
}

void ReportPageGestation0::initConnections()
{
    // QDateEdit
    const QList<QDateEdit*> des = this->findChildren<QDateEdit*>();
    for (QDateEdit *de : des)
        connect(de, &QDateEdit::dateChanged,
                this, &ReportPageGestation0::dataWasModified, Qt::UniqueConnection);

    // QLineEdit
    const QList<QLineEdit*> items = this->findChildren<QLineEdit*>();
    for (QLineEdit *item : items)
        connect(item, &QLineEdit::textChanged,
                this, &ReportPageGestation0::dataWasModified, Qt::UniqueConnection);

    // QPlainTextEdit
    const QList<QPlainTextEdit*> tes = this->findChildren<QPlainTextEdit*>();
    for (QPlainTextEdit *te : tes)
        connect(te, &QPlainTextEdit::textChanged,
                this, &ReportPageGestation0::dataWasModified, Qt::UniqueConnection);

    // QToolButton
    const QList<QToolButton*> btns = this->findChildren<QToolButton*>();
    for (QToolButton *btn : btns)
        btn->setStyleSheet(toolButtonStyleForText);

    // calcularea varstei gestationale
    connect(ui->gestation0_LMP, &QDateEdit::dateChanged,
            this, &ReportPageGestation0::onDateLMPChanged, Qt::UniqueConnection);

    //--- conclusion (QToolButton) - concluzia
    connect(ui->btn_add_template_gestation0, &QAbstractButton::clicked,
            this, &ReportPageGestation0::handleAddTemplate, Qt::UniqueConnection);
    connect(ui->btn_template_gestation0, &QAbstractButton::clicked,
            this, &ReportPageGestation0::handleSelectTemplate, Qt::UniqueConnection);

    //--- findings (QAction + LineEditCustom) - descrierea formatiunilor, calculelor etc.
    for (auto &r : rows_action_findings) {
        connect(r.action_add, &QAction::triggered,
                this, &ReportPageGestation0::handleAddFindingsTemplates, Qt::UniqueConnection);
        connect(r.action_select, &QAction::triggered,
                this, &ReportPageGestation0::handleSelectFindingsTemplates, Qt::UniqueConnection);
    }

    connect(ui->gestation0_concluzion, &QPlainTextEdit::textChanged,
            this, [this](){
                emit concluzionTextChanged();
            });
}

QString ReportPageGestation0::calculateGestationalAge(const QDate &lmp)
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

QDate ReportPageGestation0::calculateDueDate(const QDate &lmp)
{
    return lmp.addDays(280);  // Adăugăm 280 de zile (40 săptămâni)
}

bool ReportPageGestation0::existDocument(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(R"(
        SELECT EXISTS(
            SELECT 1
            FROM tableGestation0
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

void ReportPageGestation0::bindFields(QSqlQuery &q, const int idReport)
{
    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":view_examination", static_cast<int>(getViewExamination()));
    q.bindValue(":antecedent", ui->gestation0_antecedent->text().isEmpty()
                                    ? QVariant()
                                    : ui->gestation0_antecedent->text());
    q.bindValue(":gestation_age", ui->gestation0_gestation->text().isEmpty()
                                    ? QVariant()
                                    : ui->gestation0_gestation->text());
    q.bindValue(":GS",              ui->gestation0_GS_dimens->text());
    q.bindValue(":GS_age",          ui->gestation0_GS_age->text());
    q.bindValue(":CRL",             ui->gestation0_CRL_dimens->text());
    q.bindValue(":CRL_age",         ui->gestation0_CRL_age->text());
    q.bindValue(":BCF",             ui->gestation0_BCF->text());
    q.bindValue(":liquid_amniotic", ui->gestation0_liquid_amniotic->text());
    q.bindValue(":miometer",        ui->gestation0_miometer->text());
    q.bindValue(":cervix",          ui->gestation0_cervix->text());
    q.bindValue(":ovary",           ui->gestation0_ovary->text());
    q.bindValue(":concluzion",      ui->gestation0_concluzion->toPlainText());
    q.bindValue(":recommendation", ui->gestation0_recommendation->text().isEmpty()
                                       ? QVariant()
                                       : ui->gestation0_recommendation->text());
    q.bindValue(":lmp", ui->gestation0_LMP->date().toString("yyyy-MM-dd"));
}

bool ReportPageGestation0::insertData(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_ges0_insert.sql"));
    bindFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Insert tableGestation0 error:"
                               << q.lastError().text();
        return false;
    }
    qInfo(logInfo()) << "ReportPageGestation0: INSERT executat în tranzacția raportului; id=" << idReport;
    return true;
}

bool ReportPageGestation0::updateData(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_ges0_update.sql"));
    bindFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Update tableGestation0 error:"
                               << q.lastError().text();
        return false;
    }
    qInfo(logInfo()) << "ReportPageGestation0: UPDATE executat în tranzacția raportului; id=" << idReport;
    return true;
}

void ReportPageGestation0::insertConclusionTemplate(const QString m_conclusion, const QString m_system)
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

void ReportPageGestation0::insertFindigsTemplate(const QString m_description, const QString m_typeSystem)
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

bool ReportPageGestation0::eventFilter(QObject *obj, QEvent *event)
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
        QWidget *contents = ui->scrollArea_gestation0->widget();
        while (target && target->parentWidget() && target->parentWidget() != contents) {
            target = target->parentWidget();
        }

        if (target && contents->isAncestorOf(target)) {
            QTimer::singleShot(0, this, [this, target]() {
                ui->scrollArea_gestation0->ensureWidgetVisible(target, 20, 20);
            });
        }
    }

    return ReportPageBase::eventFilter(obj, event);
}

void ReportPageGestation0::changeEvent(QEvent *event)
{
    ReportPageBase::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
}
