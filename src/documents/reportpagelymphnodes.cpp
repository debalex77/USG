#include "reportpagelymphnodes.h"
#include "ui_reportpagelymphnodes.h"

#include <QPushButton>
#include <customs/custommessage.h>

ReportPageLymphNodes::ReportPageLymphNodes(DataBase &db, QSqlDatabase &currentDB, QWidget *parent)
    : ReportPageBase(parent)
    , ui(new Ui::ReportPageLymphNodes)
    , m_db(db)
    , m_currentDB(currentDB)
    , toolButtonStyleForText(m_db.toolButtonStyleForText())
{
    ui->setupUi(this);
    initInstallEventFilter();
    setDefaultContext();
    setPropertyMaxLengthText();
    connect(ui->ln_typeInvestig, &QComboBox::currentIndexChanged,
            this, &ReportPageLymphNodes::updateInvestigationTabs);
    updateInvestigationTabs();

    // QLineEdit
    const QList<QLineEdit*> items = this->findChildren<QLineEdit*>();
    for (QLineEdit *item : items)
        connect(item, &QLineEdit::textChanged,
                this, &ReportPageLymphNodes::dataWasModified, Qt::UniqueConnection);

    // QComboBox
    const QList<QComboBox*> combos = this->findChildren<QComboBox*>();
    for (QComboBox *combo: combos)
        connect(combo, &QComboBox::currentTextChanged,
                this, &ReportPageLymphNodes::dataWasModified, Qt::UniqueConnection);

    // QPlainTextEdit
    const QList<QPlainTextEdit*> tes = this->findChildren<QPlainTextEdit*>();
    for (QPlainTextEdit *te : tes)
        connect(te, &QPlainTextEdit::textChanged,
                this, &ReportPageLymphNodes::dataWasModified, Qt::UniqueConnection);

    const QList<QToolButton*> btns = this->findChildren<QToolButton*>();
    for (QToolButton *btn : btns)
        btn->setStyleSheet(toolButtonStyleForText);

    // selectarea sabloanelor concluziilor
    connect(ui->btnSelectTemplate, &QAbstractButton::clicked,
            this, &ReportPageLymphNodes::handleSelectConclusionTemplate, Qt::UniqueConnection);
    connect(ui->btnAddTemplate, &QAbstractButton::clicked,
            this, &ReportPageLymphNodes::handleAddConclusionTemplate, Qt::UniqueConnection);

    // selectarea sabloanelor recomandarilor
    connect(ui->ln_recommand->actionOpenList(), &QAction::triggered,
            this, &ReportPageLymphNodes::handleSelectRecommendationTemplate, Qt::UniqueConnection);
    connect(ui->ln_recommand->actionAddItem(), &QAction::triggered,
            this, &ReportPageLymphNodes::handleAddRecommendationTemplate, Qt::UniqueConnection);

    connect(ui->ln_conluzion, &QPlainTextEdit::textChanged,
            this, [this](){
                emit concluzionTextChanged();
            });
}

ReportPageLymphNodes::~ReportPageLymphNodes()
{
    delete ui;
}

bool ReportPageLymphNodes::loadData(int idReport)
{
    if (idReport <= 0)
        return false;

    QSqlQuery q(m_currentDB);
    q.prepare(R"(SELECT * FROM tableSofTissuesLymphNodes WHERE id_reportEcho = :id_reports)");
    q.bindValue(":id_reports", idReport);
    if(!q.exec()) {
        qWarning(logWarning()).noquote()
        << "ReportPageLymphNodes error:" << q.lastError().text()
        << "\nLast query:" << q.lastQuery();
        return false;
    }

    if (!q.next()) {
        qWarning(logWarning()) << "Nu s-au gasit datele pentru sistema 'LymphNodes' !!!";
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

    ui->ln_zoneArea->setText(q.value("examinedArea").toString());
    ui->ln_indications->setText(q.value("clinicalIndications").toString());
    ui->ln_recommand->setText(q.value("recommendation").toString());
    ui->ln_conluzion->setPlainText(q.value("concluzion").toString());

    const QString name_investigation = q.value("section_type").toString();
    if (name_investigation == "soft_tissues") {
        ui->ln_typeInvestig->setCurrentIndex(1);
        ui->tabWidget_LymphNodes->setCurrentIndex(0);

        ui->ln_skinStructureTissue->setCurrentText(q.value("skin_structure").toString());
        ui->ln_subcutanTissue->setCurrentText(q.value("subcutaneous_tissue").toString());
        ui->ln_localizationTissue->setText(q.value("lesion_location").toString());
        ui->ln_sizeTissue->setText(q.value("lesion_size").toString());
        ui->ln_ecogenityTissue->setCurrentText(q.value("lesion_echogenicity").toString());
        ui->ln_conturTissue->setCurrentText(q.value("lesion_contour").toString());
        ui->ln_DopplerTissue->setCurrentText(q.value("lesion_vascularization").toString());
        ui->ln_otherChangeTissue->setPlainText(q.value("other_changes").toString());
    } else if (name_investigation == "lymph_nodes") {
        ui->ln_typeInvestig->setCurrentIndex(2);
        ui->tabWidget_LymphNodes->setCurrentIndex(1);

        ui->ln_nrNodes->setText(q.value("ln_number").toString());
        ui->ln_sizeNode->setText(q.value("ln_size_nodes").toString());
        ui->ln_formNode->setCurrentText(q.value("ln_shape").toString());
        ui->ln_hillNode->setCurrentText(q.value("ln_echogenic_hilum").toString());
        ui->ln_corticalNode->setCurrentText(q.value("ln_cortex").toString());
        ui->ln_structureNode->setCurrentText(q.value("ln_structure").toString());
        ui->ln_contourNode->setCurrentText(q.value("ln_contour").toString());
        ui->ln_dopplerNode->setCurrentText(q.value("ln_vascularization").toString());
        ui->ln_otherChangeNode->setPlainText(q.value("ln_associated_changes").toString());
    } else {
        ui->ln_typeInvestig->setCurrentIndex(0);
        ui->ln_skinStructureTissue->setCurrentText(q.value("skin_structure").toString());
        ui->ln_subcutanTissue->setCurrentText(q.value("subcutaneous_tissue").toString());
        ui->ln_localizationTissue->setText(q.value("lesion_location").toString());
        ui->ln_sizeTissue->setText(q.value("lesion_size").toString());
        ui->ln_ecogenityTissue->setCurrentText(q.value("lesion_echogenicity").toString());
        ui->ln_conturTissue->setCurrentText(q.value("lesion_contour").toString());
        ui->ln_DopplerTissue->setCurrentText(q.value("lesion_vascularization").toString());
        ui->ln_otherChangeTissue->setPlainText(q.value("other_changes").toString());
        ui->ln_nrNodes->setText(q.value("ln_number").toString());
        ui->ln_sizeNode->setText(q.value("ln_size_nodes").toString());
        ui->ln_formNode->setCurrentText(q.value("ln_shape").toString());
        ui->ln_hillNode->setCurrentText(q.value("ln_echogenic_hilum").toString());
        ui->ln_corticalNode->setCurrentText(q.value("ln_cortex").toString());
        ui->ln_structureNode->setCurrentText(q.value("ln_structure").toString());
        ui->ln_contourNode->setCurrentText(q.value("ln_contour").toString());
        ui->ln_dopplerNode->setCurrentText(q.value("ln_vascularization").toString());
        ui->ln_otherChangeNode->setPlainText(q.value("ln_associated_changes").toString());
    }

    updateInvestigationTabs();
    return true;
}

bool ReportPageLymphNodes::saveData(int idReport)
{
    if (idReport <= 0)
        return false;

    if (existDocument(idReport))
        return updateData(idReport);
    else
        return insertData(idReport);

    return false;
}

QString ReportPageLymphNodes::concluzionText() const
{
    return ui->ln_conluzion
               ? ui->ln_conluzion->toPlainText().trimmed()
               : QString{};
}

ReportSections::ReportSystem ReportPageLymphNodes::system() const
{
    return ReportSections::ReportSystem::LymphNodes;
}

QString ReportPageLymphNodes::typeInvestigation() const
{
    if (ui->ln_typeInvestig->currentIndex() == 1)
        return "soft_tissues";
    else if (ui->ln_typeInvestig->currentIndex() == 2)
        return "lymph_nodes";
    else
        return "tissues_nodes";
}

void ReportPageLymphNodes::handleSelectConclusionTemplate()
{
    auto *dlg = new CatalogTableEditor(m_db,
                                       CatalogType::FormType::Selection,
                                       CatalogType::Type::ConclusionTemplates,
                                       this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setFilterQuery(QStringLiteral("Gangl.limfatici"));
    connect(dlg, &CatalogTableEditor::dataSelected, this, [this, dlg](const QVariantMap &data){
        if (data.isEmpty())
            return;
        ui->ln_conluzion->appendPlainText(data.value("name").toString());
        dlg->close();
    });
    dlg->show();
}

void ReportPageLymphNodes::handleAddConclusionTemplate()
{
    const QString conclusion = ui->ln_conluzion->toPlainText().trimmed();
    if (!conclusion.isEmpty())
        insertConclusionTemplate(conclusion, QStringLiteral("Gangl.limfatici"));
}

void ReportPageLymphNodes::handleSelectRecommendationTemplate()
{
    auto *dlg = new CatalogTableEditor(m_db,
                                       CatalogType::FormType::Selection,
                                       CatalogType::Type::SystemTemplates,
                                       this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setFilterQuery(QStringLiteral("Recomandari (gangl.limfatici)"));
    connect(dlg, &CatalogTableEditor::dataSelected, this, [this, dlg](const QVariantMap &data){
        if (data.isEmpty())
            return;
        ui->ln_recommand->setText(data.value("name").toString());
        dlg->close();
    });
    dlg->show();
}

void ReportPageLymphNodes::handleAddRecommendationTemplate()
{
    const QString recommendation = ui->ln_recommand->text().trimmed();
    if (!recommendation.isEmpty())
        insertRecommendationTemplate(recommendation,
                                     QStringLiteral("Recomandari (gangl.limfatici)"));
}

void ReportPageLymphNodes::initInstallEventFilter()
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

void ReportPageLymphNodes::setDefaultContext()
{

}

void ReportPageLymphNodes::setPropertyMaxLengthText()
{
    //--- items
    ui->ln_zoneArea->setMaxLength(50);
    ui->ln_zoneArea->setPlaceholderText(tr("reg.cervicală, axilară, inghinală ... max.50 caractere"));
    //--- tissues
    ui->ln_indications->setMaxLength(100);
    ui->ln_indications->setPlaceholderText(tr("tumefacție, durere, susp. de adenopatie, control postoper. ... max.100 caractere"));
    ui->ln_localizationTissue->setMaxLength(100);
    ui->ln_localizationTissue->setPlaceholderText(tr("... max. 100 caractere"));
    ui->ln_sizeTissue->setMaxLength(50);
    ui->ln_sizeTissue->setPlaceholderText(tr("... max.50 caractere"));
    ui->ln_otherChangeTissue->setPlaceholderText(tr("colecții, hematom, abces, lipom etc. ... maximum 250 caractere"));
    //--- nodes
    ui->ln_sizeNode->setMaxLength(50);
    ui->ln_sizeNode->setPlaceholderText(tr("... max.50 caractere"));
    ui->ln_otherChangeNode->setPlaceholderText(tr("necroză, calcificări, adenopatie suspectă ... max.250 carcatere"));

    //--- concluzion
    ui->ln_conluzion->setPlaceholderText(tr("... maximum 500 caractere"));

    //--- recommandation
    ui->ln_recommand->setMaxLength(250);
    ui->ln_recommand->setPlaceholderText(tr("... maximum 250 caractere"));
}

bool ReportPageLymphNodes::existDocument(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(R"(
        SELECT EXISTS(
            SELECT 1
            FROM tableSofTissuesLymphNodes
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

void ReportPageLymphNodes::bindFields(QSqlQuery &q, const int idReport)
{
    QString type_investigation;
    if (ui->ln_typeInvestig->currentIndex() == 0)
        type_investigation = "tissues_nodes";
    else if (ui->ln_typeInvestig->currentIndex() == 1)
        type_investigation = "soft_tissues";
    else
        type_investigation = "lymph_nodes";

    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":section_type", type_investigation);
    q.bindValue(":examinedArea", ui->ln_zoneArea->text());
    q.bindValue(":clinicalIndications", ui->ln_indications->text());
    // soft tissues
    q.bindValue(":skin_structure", (ui->ln_typeInvestig->currentIndex() == 2)
                                        ? QVariant()
                                        : ui->ln_skinStructureTissue->currentText());
    q.bindValue(":subcutaneous_tissue", (ui->ln_typeInvestig->currentIndex() == 2)
                                            ? QVariant()
                                            : ui->ln_subcutanTissue->currentText());
    q.bindValue(":lesion_location", (ui->ln_typeInvestig->currentIndex() == 2)
                                        ? QVariant()
                                        : ui->ln_localizationTissue->text());
    q.bindValue(":lesion_size", (ui->ln_typeInvestig->currentIndex() == 2)
                                    ? QVariant()
                                    : ui->ln_sizeTissue->text());
    q.bindValue(":lesion_echogenicity", (ui->ln_typeInvestig->currentIndex() == 2)
                                            ? QVariant()
                                            : ui->ln_ecogenityTissue->currentText());
    q.bindValue(":lesion_contour", (ui->ln_typeInvestig->currentIndex() == 2)
                                        ? QVariant()
                                        : ui->ln_conturTissue->currentText());
    q.bindValue(":lesion_vascularization", (ui->ln_typeInvestig->currentIndex() == 2)
                                                ? QVariant()
                                                : ui->ln_DopplerTissue->currentText());
    // lymph nodes
    q.bindValue(":ln_number", (ui->ln_typeInvestig->currentIndex() == 1)
                                    ? QVariant()
                                    : ui->ln_nrNodes->text().toInt());
    q.bindValue(":ln_size_nodes", (ui->ln_typeInvestig->currentIndex() == 1)
                                        ? QVariant()
                                        : ui->ln_sizeNode->text());
    q.bindValue(":ln_shape", (ui->ln_typeInvestig->currentIndex() == 1)
                                ? QVariant()
                                : ui->ln_formNode->currentText());
    q.bindValue(":ln_echogenic_hilum", (ui->ln_typeInvestig->currentIndex() == 1)
                                            ? QVariant()
                                            : ui->ln_hillNode->currentText());
    q.bindValue(":ln_cortex", (ui->ln_typeInvestig->currentIndex() == 1)
                                  ? QVariant()
                                  : ui->ln_corticalNode->currentText());
    q.bindValue(":ln_structure", (ui->ln_typeInvestig->currentIndex() == 1)
                                     ? QVariant()
                                     : ui->ln_structureNode->currentText());
    q.bindValue(":ln_contour", (ui->ln_typeInvestig->currentIndex() == 1)
                                   ? QVariant()
                                   : ui->ln_contourNode->currentText());
    q.bindValue(":ln_vascularization", (ui->ln_typeInvestig->currentIndex() == 1)
                                            ? QVariant()
                                            : ui->ln_dopplerNode->currentText());
    q.bindValue(":ln_associated_changes", (ui->ln_typeInvestig->currentIndex() == 1)
                                            ? QVariant()
                                            : ui->ln_otherChangeNode->toPlainText());

    if (ui->ln_typeInvestig->currentIndex() == 0 ||
        ui->ln_typeInvestig->currentIndex() == 1 ||
        ui->ln_typeInvestig->currentIndex() == 2)
        q.bindValue(":other_changes", ui->ln_otherChangeTissue->toPlainText().isEmpty()
                                        ? QVariant()
                                        : ui->ln_otherChangeTissue->toPlainText());

    q.bindValue(":concluzion", ui->ln_conluzion->toPlainText());
    q.bindValue(":recommendation", ui->ln_recommand->text().isEmpty() ? QVariant() : ui->ln_recommand->text());
}

bool ReportPageLymphNodes::insertData(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_lymphnodes_insert.sql"));
    bindFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Insert tableSofTissuesLymphNodes error:"
                               << q.lastError().text();
        return false;
    }
    qInfo(logInfo()) << "ReportPageLymphNodes: INSERT executat în tranzacția raportului; id=" << idReport;
    return true;
}

bool ReportPageLymphNodes::updateData(const int idReport)
{
    QSqlQuery q(m_currentDB);
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/report_lymphnodes_update.sql"));
    bindFields(q, idReport);
    if (! q.exec()) {
        qWarning(logWarning()) << "Update tableSofTissuesLymphNodes error:"
                               << q.lastError().text();
        return false;
    }
    qInfo(logInfo()) << "ReportPageLymphNodes: UPDATE executat în tranzacția raportului; id=" << idReport;
    return true;
}

void ReportPageLymphNodes::insertConclusionTemplate(const QString &conclusion,
                                                     const QString &system)
{
    QSqlQuery q(m_currentDB);
    q.prepare("SELECT COUNT(name) FROM conclusionTemplates WHERE name = :name");
    q.bindValue(":name", conclusion);
    if (q.exec() && q.next() && q.value(0).toInt() > 0) {
        QMessageBox msgBox(QMessageBox::Question,
                           tr("Verificarea dublajului"),
                           tr("Concluzia <b>%1</b> există ca șablon.<br>"
                              "Doriți să prelungiți validarea ?").arg(conclusion),
                           QMessageBox::Yes | QMessageBox::No, this);
        if (msgBox.exec() == QMessageBox::No)
            return;
    }

    QString sql = "INSERT INTO conclusionTemplates (deletionMark, cod, name, %1%, uuid) VALUES (?,?,?,?,?)";
    sql.replace("%1%", globals().thisMySQL ? "`system`" : "system");
    q.prepare(sql);
    q.addBindValue(0);
    q.addBindValue(m_db.getLastIdForTable("conclusionTemplates") + 1);
    q.addBindValue(conclusion);
    q.addBindValue(system);
    q.addBindValue(QUuid::createUuid().toRfc4122());
    if (!q.exec()) {
        qWarning(logWarning()) << "ReportPageLymphNodes add conclusion template error:"
                               << q.lastError().text();
        CustomMessage msg(this);
        msg.setWindowTitle(QGuiApplication::applicationDisplayName());
        msg.setTextTitle(tr("Inserarea șablonului în baza de date nu s-a efectuat."));
        msg.setDetailedText(q.lastError().text());
        msg.exec();
        return;
    }

    emit showPopUp(tr("Șablonul adăugat cu succes<br>în baza de date."));
}

void ReportPageLymphNodes::insertRecommendationTemplate(const QString &recommendation,
                                                         const QString &system)
{
    if (!ensureRecommendationTemplateType()) {
        CustomMessage msg(this);
        msg.setWindowTitle(QGuiApplication::applicationDisplayName());
        msg.setTextTitle(tr("Schema șabloanelor de recomandări nu a putut fi actualizată."));
        msg.setDetailedText(m_currentDB.lastError().text());
        msg.exec();
        return;
    }

    QSqlQuery q(m_currentDB);
    q.prepare(R"(SELECT name FROM formationsSystemTemplates
                 WHERE name = :name AND typeSystem = :typeSystem)");
    q.bindValue(":name", recommendation);
    q.bindValue(":typeSystem", system);
    if (q.exec() && q.next()) {
        QMessageBox msgBox(QMessageBox::Question,
                           tr("Verificarea dublajului"),
                           tr("Recomandarea <b><u>'%1'</u></b> există ca șablon.<br>"
                              "Doriți să prelungiți validarea ?").arg(recommendation),
                           QMessageBox::Yes | QMessageBox::No, this);
        if (msgBox.exec() == QMessageBox::No)
            return;
    }

    q.prepare(R"(INSERT INTO formationsSystemTemplates
                 (id, deletionMark, name, typeSystem, uuid) VALUES (?, ?, ?, ?, ?))");
    q.addBindValue(m_db.getLastIdForTable("formationsSystemTemplates") + 1);
    q.addBindValue(0);
    q.addBindValue(recommendation);
    q.addBindValue(system);
    q.addBindValue(QUuid::createUuid().toRfc4122());
    if (!q.exec()) {
        qWarning(logWarning()) << "ReportPageLymphNodes add recommendation template error:"
                               << q.lastError().text();
        CustomMessage msg(this);
        msg.setWindowTitle(QGuiApplication::applicationDisplayName());
        msg.setTextTitle(tr("Inserarea recomandării în baza de date nu s-a efectuat."));
        msg.setDetailedText(q.lastError().text());
        msg.exec();
        return;
    }

    emit showPopUp(tr("Șablonul adăugat cu succes<br>în baza de date."));
}

bool ReportPageLymphNodes::ensureRecommendationTemplateType()
{
    const QString type = QStringLiteral("Recomandari (gangl.limfatici)");
    const bool sqlite = m_currentDB.driverName().compare(QStringLiteral("QSQLITE"),
                                                          Qt::CaseInsensitive) == 0;

    if (sqlite) {
        QSqlQuery schema(m_currentDB);
        if (!schema.exec(QStringLiteral(
                "SELECT sql FROM sqlite_master WHERE type='table' "
                "AND name='formationsSystemTemplates'"))
            || !schema.next()) {
            qWarning(logWarning()) << "ReportPageLymphNodes schema check error:"
                                   << schema.lastError().text();
            return false;
        }

        QString createSql = schema.value(0).toString();
        schema.finish();
        if (createSql.contains(type))
            return true;

        const QString oldTail = QStringLiteral("'Recomandari (gestatation2)')");
        createSql.replace(oldTail, QStringLiteral(
            "'Recomandari (gestatation2)', 'Recomandari (gangl.limfatici)')"));
        if (!createSql.contains(type))
            return false;

        QStringList indexes;
        QSqlQuery indexQuery(m_currentDB);
        if (!indexQuery.exec(QStringLiteral(
                "SELECT sql FROM sqlite_master WHERE type='index' "
                "AND tbl_name='formationsSystemTemplates' AND sql IS NOT NULL")))
            return false;
        while (indexQuery.next())
            indexes.append(indexQuery.value(0).toString());
        indexQuery.finish();

        if (!m_currentDB.transaction())
            return false;

        QSqlQuery alter(m_currentDB);
        bool ok = alter.exec(QStringLiteral(
                      "ALTER TABLE formationsSystemTemplates "
                      "RENAME TO formationsSystemTemplates_old"))
                  && alter.exec(createSql)
                  && alter.exec(QStringLiteral(
                      "INSERT INTO formationsSystemTemplates "
                      "(id, deletionMark, name, typeSystem, uuid) "
                      "SELECT id, deletionMark, name, typeSystem, uuid "
                      "FROM formationsSystemTemplates_old"))
                  && alter.exec(QStringLiteral(
                      "DROP TABLE formationsSystemTemplates_old"));
        for (const QString &index : indexes) {
            if (ok && !alter.exec(index))
                ok = false;
        }

        if (!ok || !m_currentDB.commit()) {
            qWarning(logWarning()) << "ReportPageLymphNodes SQLite schema update error:"
                                   << alter.lastError().text();
            m_currentDB.rollback();
            return false;
        }

        qInfo(logInfo())
            << "Schema formationsSystemTemplates a fost extinsă pe conexiunea ReportDialog.";
        return true;
    }

    QSqlQuery schema(m_currentDB);
    schema.prepare(QStringLiteral(
        "SELECT COLUMN_TYPE FROM INFORMATION_SCHEMA.COLUMNS "
        "WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='formationsSystemTemplates' "
        "AND COLUMN_NAME='typeSystem'"));
    if (!schema.exec() || !schema.next())
        return false;
    if (schema.value(0).toString().contains(type))
        return true;

    QSqlQuery alter(m_currentDB);
    const bool ok = alter.exec(QStringLiteral(R"(
        ALTER TABLE formationsSystemTemplates
        MODIFY COLUMN typeSystem ENUM(
            'Unknow', 'Ficat', 'Colecist', 'Pancreas', 'Splina', 'Intestine', 'Recomandari (org.interne)',
            'Rinichi', 'V.urinara', 'Gl.suprarenale', 'Recomandari (s.urinar)',
            'Prostata', 'Recomandari (prostata)', 'Tiroida', 'Recomandari (tiroida)',
            'Gl.mamara (stanga)', 'Gl.mamara (dreapta)', 'Recomandari (gl.mamare)',
            'Ginecologia (uter)', 'Ginecologia (ovar stang)', 'Ginecologia (ovar drept)', 'Recomandari (ginecologia)',
            'Recomandari (gestatation0)', 'Recomandari (gestatation1)', 'Recomandari (gestatation2)',
            'Recomandari (gangl.limfatici)'
        ) NOT NULL DEFAULT 'Unknow'
    )"));
    if (!ok)
        qWarning(logWarning()) << "ReportPageLymphNodes MariaDB schema update error:"
                               << alter.lastError().text();
    return ok;
}

bool ReportPageLymphNodes::eventFilter(QObject *obj, QEvent *event)
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

    return ReportPageBase::eventFilter(obj, event);
}

void ReportPageLymphNodes::changeEvent(QEvent *event)
{
    ReportPageBase::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
}

void ReportPageLymphNodes::updateInvestigationTabs()
{
    const int type = ui->ln_typeInvestig->currentIndex();
    ui->tabWidget_LymphNodes->setTabEnabled(0, type != 2);
    ui->tabWidget_LymphNodes->setTabEnabled(1, type != 1);
    if (type == 1)
        ui->tabWidget_LymphNodes->setCurrentIndex(0);
    else if (type == 2)
        ui->tabWidget_LymphNodes->setCurrentIndex(1);
}
