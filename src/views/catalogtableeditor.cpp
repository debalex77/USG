#include "catalogtableeditor.h"
#include "ui_catalogtableeditor.h"

CatalogTableEditor::CatalogTableEditor(DataBase &db,
                                       CatalogType::FormType formType,
                                       CatalogType::Type catalogType,
                                       QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CatalogTableEditor)
    , m_settings(globals().pathSettingsCommon)
    , m_formType(formType)
    , m_catalogType(catalogType)
    , m_db(db)
    , popUp(new PopUp(this))
    , toolButtonStyleForIcon(m_db.toolButtonStyleForIcon())
    , toolButtonStyleForText(m_db.toolButtonStyleForText())
    , model(new BaseAbstractModel(this))
{
    ui->setupUi(this);

    if (m_formType == CatalogType::FormType::List) {
        setWindowTitle(tr("Catalog: %1")
                           .arg(CatalogType::enumToStringRo(m_catalogType)));
        toolBar = new ToolBarCustom(this,
                                     ToolBarCustom::AddEditDelete);
        editableColumn = true;
    } else {
        setWindowTitle(tr("Catalog: %1 (selectare)")
                           .arg(CatalogType::enumToStringRo(m_catalogType)));
        toolBar = new ToolBarCustom(this,
                                     ToolBarCustom::ButtonSelect |
                                     ToolBarCustom::AddEditDelete);
        editableColumn = false;
    }

    loadFilterBySettings();

    initToolBar();
    initTableView();
    updateTableView();

    loadSizeSection();
}

CatalogTableEditor::~CatalogTableEditor()
{
    delete ui;
}

void CatalogTableEditor::setFilterQuery(const QString nameSystem)
{
    m_filterQuery = nameSystem;
    updateTableView();
}

void CatalogTableEditor::onAdd()
{
    if (m_formType == CatalogType::FormType::Selection) {
        QMessageBox::warning(this, tr("Control"),
                             tr("În regim de selectarea nu puteți adauga/redacta randuri !!!"));
        return;
    }

    if (!model)
        return;

    QVariantMap row = getDataByCatalogType();

    if (!model->addRow(row)) {
        QMessageBox::warning(this, tr("Eroare"),
                             tr("Nu s-a putut adăuga rând nou."));
        return;
    }

    const int rowIndex = model->rowCount() - 1;
    if (rowIndex < 0)
        return;

    const int codColumn = InvestigationsSections::Cod;
    const QModelIndex idx = model->index(rowIndex, codColumn);
    if (!idx.isValid())
        return;

    ui->tableView->clearSelection();
    ui->tableView->setCurrentIndex(idx);
    ui->tableView->scrollTo(idx, QAbstractItemView::PositionAtBottom);
    ui->tableView->setFocus();
    ui->tableView->edit(idx);
}

void CatalogTableEditor::onEdit()
{
    if (m_formType == CatalogType::FormType::Selection) {
        QMessageBox::warning(this, tr("Control"),
                             tr("În regim de selectarea nu puteți adauga/redacta randuri !!!"));
        return;
    }

    if (!model)
        return;

    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::information(this,
                                 tr("Editare"),
                                 tr("Nu este selectat niciun rând pentru editare."));
        return;
    }

    if (!(model->flags(currentIndex) & Qt::ItemIsEditable)) {
        QMessageBox::warning(this,
                             tr("Editare"),
                             tr("Celula selectată nu poate fi editată."));
        return;
    }

    ui->tableView->setFocus();
    ui->tableView->scrollTo(currentIndex);
    ui->tableView->selectionModel()->setCurrentIndex(currentIndex,
                                                     QItemSelectionModel::ClearAndSelect
                                                         | QItemSelectionModel::Rows
                                                         | QItemSelectionModel::Current);
    ui->tableView->edit(currentIndex);
}

void CatalogTableEditor::onDelete()
{
    if (m_formType == CatalogType::FormType::Selection) {
        QMessageBox::warning(this, tr("Control"),
                             tr("În regim de selectarea nu puteți marca rânduri pentru eliminare !!!"));
        return;
    }

    if (!model)
        return;

    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::information(this,
                                 tr("Editare"),
                                 tr("Nu este selectat niciun rând pentru editare."));
        return;
    }

    const int row = currentIndex.row();

    QVariantMap items = model->rowDataByRow(row);
    bool isMark = items["deletionMark"] == StatusObject::DeletionMark;

    if (!model->setDeletionMark(row,
                                isMark
                                    ? StatusObject::ZeroWrite
                                    : StatusObject::DeletionMark)) {
        QMessageBox::warning(this,
                             tr("Eroare"),
                             tr("Nu s-a putut marca rândul pentru ștergere."));
    }

    if (!saveCurrentRowToDatabase(row)) {
        QMessageBox::warning(this,
                             tr("Eroare"),
                             tr("Nu s-au putut salva modificările în baza de date."));
        return;
    }

    const int targetRow = (row > 0) ? row - 1 : 0;
    if (model->rowCount() > 0) {
        const QModelIndex targetIndex = model->index(targetRow, currentIndex.column());
        if (targetIndex.isValid()) {
            ui->tableView->setCurrentIndex(targetIndex);
            ui->tableView->selectionModel()->setCurrentIndex(targetIndex,
                                                             QItemSelectionModel::ClearAndSelect |
                                                             QItemSelectionModel::Rows |
                                                             QItemSelectionModel::Current);
            ui->tableView->scrollTo(targetIndex, QAbstractItemView::PositionAtCenter);
        }
    }

    popUp->setPopupText(tr("Obiectul %1 marcat pentru<br>"
                           "eliminare.")
                            .arg(isMark
                                     ? tr("nu mai este")
                                     : tr("este")));
    popUp->show();
}

void CatalogTableEditor::onOpenTreeInvestigations()
{
    GroupInvestigationList *tree = new GroupInvestigationList(m_db, this);
    tree->setAttribute(Qt::WA_DeleteOnClose);
    tree->show();
}

void CatalogTableEditor::onOpenUltrasoundTariffClassifier()
{
    QFile file(":/xmls/investig_2024.xml");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning(logWarning()) << "Nu se poate deschide fișierul XML:"
                               << file.errorString();
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning(logWarning()) << "Eroare la parsarea fișierului XML.";
        return;
    }

    QDomElement root = doc.documentElement();
    if (root.tagName() != "list_investigation") {
        qWarning(logWarning()) << "Tagul rădăcină al fișierului XML este incorect.";
        return;
    }

    auto *printModel = new QStandardItemModel(this);

    QDomNodeList entries = root.elementsByTagName("entry");
    for (int i = 0; i < entries.count(); ++i) {
        QDomElement entry = entries.at(i).toElement();
        if (entry.isNull())
            continue;

        const QString cod  = entry.attribute("cod");
        const QString name = entry.attribute("name");
        const QString cost = entry.attribute("cost");

        auto *itemCod  = new QStandardItem(cod);
        auto *itemName = new QStandardItem(name);
        auto *itemCost = new QStandardItem(QString::number(cost.toDouble(), 'f', 2));

        printModel->appendRow({itemCod, itemName, itemCost});
    }

    auto *report = new LimeReport::ReportEngine(this);
    report->dataManager()->clearUserVariables();
    report->dataManager()->addModel("print_model", printModel, false);
    report->setShowProgressDialog(true);
    report->setPreviewWindowTitle(tr("Ultrasound Tariff Classifier (updated 2024)"));

    QDir dir;
    if (!report->loadFromFile(dir.toNativeSeparators(globals().docsTemplatesPath + "/CatalogCost.lrxml"))) {
        QMessageBox::warning(this,
                             tr("Verificarea șablonului"),
                             tr("Nu a fost găsit formular de tipar !!!<br>"
                                "Probabil în setările aplicației nu este setat corect "
                                "drumul spre formularele de tipar."),
                             QMessageBox::Ok);
        report->deleteLater();
        printModel->deleteLater();
        return;
    }

    report->previewReport();

    report->deleteLater();
    printModel->deleteLater();
}

void CatalogTableEditor::onDataChangedItemModel(const QModelIndex &topLeft,
                                                const QModelIndex &bottomRight,
                                                const QVector<int> &/*roles*/)
{
    Q_UNUSED(bottomRight);

    if (!model ||
        !topLeft.isValid())
        return;

    const int row = topLeft.row();
    const int col = topLeft.column();

    auto focusEditAt = [&](int targetCol) {
        if (targetCol < 0)
            return;

        const QModelIndex idx = model->index(row, targetCol);
        if (!idx.isValid())
            return;

        ui->tableView->setCurrentIndex(idx);
        ui->tableView->scrollTo(idx, QAbstractItemView::PositionAtBottom);
        ui->tableView->edit(idx);

        m_modifiedRows.insert(row); // vezi: closeEvent()
    };

    switch (m_catalogType) {
    case CatalogType::Type::Investigations: {

        if (col == InvestigationsSections::Cod) {
            focusEditAt(InvestigationsSections::Name);
        } else if (col == InvestigationsSections::Name) {
            focusEditAt(InvestigationsSections::Use);
        } else if (col == InvestigationsSections::Use) {
            focusEditAt(InvestigationsSections::Owner);
        } else if (col == InvestigationsSections::Owner) {
            saveCurrentRowToDatabase(row);
        }
        break;
    }

    case CatalogType::Type::TypesPrices:
        if (col == TypePricesSections::Name) {
            focusEditAt(TypePricesSections::Discount);
        } else if (col == TypePricesSections::Discount) {
            focusEditAt(TypePricesSections::Noncomercial);
        } else if (col == TypePricesSections::Noncomercial) {
            saveCurrentRowToDatabase(row);
        }
        break;

    case CatalogType::Type::ConclusionTemplates:
        if (col == ConclusionTemplatesSections::Code)
            focusEditAt(ConclusionTemplatesSections::Name);
        else if (col == ConclusionTemplatesSections::Name)
            focusEditAt(ConclusionTemplatesSections::System);
        else if (col == ConclusionTemplatesSections::System)
            saveCurrentRowToDatabase(row);
        break;

    case CatalogType::Type::SystemTemplates:
        if (col == SystemFormationsTemplatesSections::Name)
            focusEditAt(SystemFormationsTemplatesSections::System);
        else if (col == SystemFormationsTemplatesSections::System)
            saveCurrentRowToDatabase(row);
        break;

    default:
        break;
    }
}

void CatalogTableEditor::onValidationFailed(const QPersistentModelIndex &index,
                                            const QString &errorText)
{
    if (!index.isValid() || errorText.isEmpty())
        return;

    const bool isComboColumn =
        qobject_cast<ComboDelegate *>(ui->tableView->itemDelegateForColumn(index.column())) != nullptr;

    if (!errorText.isEmpty()) {
        const QRect cellRect = ui->tableView->visualRect(index);
        const QPoint globalPos = ui->tableView->viewport()->mapToGlobal(
            QPoint(cellRect.center().x(), cellRect.top()));

        BalloonTip::showBalloon(QMessageBox::Information,
                                tr("Verificarea datelor"),
                                errorText,
                                globalPos,
                                4000,
                                true,
                                BalloonTip::TopCenter);

        ui->tableView->setCurrentIndex(index);
        ui->tableView->scrollTo(index, QAbstractItemView::PositionAtCenter);

        if (!isComboColumn) {
            QTimer::singleShot(0, this, [this, index]() {
                if (!index.isValid())
                    return;

                ui->tableView->setCurrentIndex(index);
                ui->tableView->scrollTo(index, QAbstractItemView::PositionAtCenter);
                ui->tableView->edit(index);
            });
        }
    }
}

void CatalogTableEditor::onCloseEditor(QWidget *editor,
                                       QAbstractItemDelegate::EndEditHint hint)
{
    Q_UNUSED(editor)

    if (!model)
        return;

    if (hint != QAbstractItemDelegate::RevertModelCache)
        return;

    const QModelIndex currentIndex = ui->tableView->currentIndex();
    if (!currentIndex.isValid())
        return;

    const int row = currentIndex.row();
    const QVariantMap rowData = model->rowDataByRow(row);
    if (rowData.isEmpty())
        return;

    // doar pentru rânduri noi
    if (rowData.value("id").toInt() != 0)
        return;

    bool removeRow = false;

    switch (m_catalogType) {

    case CatalogType::Type::Investigations: {
        const QString cod = rowData.value("cod").toString().trimmed();
        const QString name = rowData.value("name").toString().trimmed();
        const QVariant owner = rowData.value("owner");

        removeRow = cod.isEmpty()
                    || name.isEmpty()
                    || !owner.isValid()
                    || owner.isNull()
                    || owner.toInt() == 0;
        break;
    }

    case CatalogType::Type::TypesPrices:
        removeRow = rowData.value("name").toString().trimmed().isEmpty();
        break;

    case CatalogType::Type::ConclusionTemplates:
        removeRow = rowData.value("cod").toString().trimmed().isEmpty()
                    || rowData.value("name").toString().trimmed().isEmpty();
        break;

    default:
        break;
    }

    if (!removeRow)
        return;

    m_modifiedRows.remove(row);
    model->removeRowAt(row);
}

void CatalogTableEditor::onRowSelected(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    QVariantMap data = model->rowDataByRow(index.row());
    emit dataSelected(data);
}

void CatalogTableEditor::loadFilterBySettings()
{
    const QString catalog = CatalogType::enumToString(m_catalogType);

    const QJsonObject rootObj = m_settings.getJsonObject(m_class);
    if (rootObj.isEmpty()) {
        return;
    }

    const QJsonObject catalogObj = rootObj.value(catalog).toObject();
    if (catalogObj.isEmpty()) {
        return;
    }

    // --- sortarea sectiilor
    m_filter.sortSection = catalogObj.value("sort").toObject().value("section").toInt(0);
    m_filter.sortOrder = catalogObj.value("sort").toObject().value("direction").toInt(0) == 0
                             ? Qt::AscendingOrder
                             : Qt::DescendingOrder;

    // --- size section
    const QJsonObject sectionsObj = catalogObj.value("sections").toObject();
    for (auto it = sectionsObj.begin(); it != sectionsObj.end(); ++it) {
        bool ok = false;
        const int index = it.key().toInt(&ok);
        if (ok)
            m_filter.sectionSizes[index] = it.value().toInt();
    }

    // --- hide/show section
    const QJsonObject hiddenObj = catalogObj.value("hide_show_sections").toObject();
    for (auto it = hiddenObj.begin(); it != hiddenObj.end(); ++it) {
        bool ok = false;
        const int index = it.key().toInt(&ok);
        if (ok)
            m_filter.hiddenSections[index] = (it.value().toInt() != 0);
    }
}

void CatalogTableEditor::loadSizeSection()
{
    auto *header = ui->tableView->horizontalHeader();
    if (!header)
        return;

    ui->tableView->setUpdatesEnabled(false);

    const int colCount = header->count();

    for (int col = 0; col < colCount; ++col) {
        const int width = m_filter.sectionSizes.value(col, header->defaultSectionSize());
        header->resizeSection(col, width);

        // const bool hidden = m_filter.hiddenSections.value(col, false);
        // ui->tableView->setColumnHidden(col, hidden);
    }

    // const int sortSection = m_filter.sortSection;
    // const Qt::SortOrder sortOrder = m_filter.sortOrder;

    // if (sortSection >= 0 && sortSection < colCount) {
    //     header->setSortIndicator(sortSection, sortOrder);
    //     ui->tableView->sortByColumn(sortSection, sortOrder);
    // }

    if (ui->tableView->model() && ui->tableView->model()->rowCount() > 0)
        ui->tableView->selectRow(0);

    ui->tableView->setUpdatesEnabled(true);
}

void CatalogTableEditor::saveSizeSection()
{
    QString m_cat = CatalogType::enumToString(m_catalogType);

    for (int numSection = 0; numSection < ui->tableView->horizontalHeader()->count(); ++numSection) {

        // size sections
        int w = ui->tableView->horizontalHeader()->sectionSize(numSection);
        m_settings.setValue(metaObject()->className(), m_cat + QString("/sections/%1").arg(numSection), w);

        // sortarea
        m_settings.setValue(metaObject()->className(), m_cat + "/sort/section",
                            ui->tableView->horizontalHeader()->sortIndicatorSection());

        m_settings.setValue(metaObject()->className(), m_cat + "/sort/direction",
                            static_cast<int>(ui->tableView->horizontalHeader()->sortIndicatorOrder()));

        // show/hide section
        m_settings.setValue(metaObject()->className(),
                            m_cat + QString("/hide_show_sections/%1").arg(numSection),
                            ui->tableView->horizontalHeader()->isSectionHidden(numSection) ? 1 : 0);
    }

    m_settings.save();
}

void CatalogTableEditor::initToolBar()
{
    if (m_formType == CatalogType::FormType::List)
        toolBar->setStyles(toolButtonStyleForIcon);
    else
        toolBar->setStyles(toolButtonStyleForIcon,
                           toolButtonStyleForText);

    ui->layoutToolBar->addWidget(toolBar);

    if (m_formType == CatalogType::FormType::Selection) {
        connect(toolBar->getBtnSelect(), &QToolButton::clicked,
                this, [this]() {
                    QModelIndex index = ui->tableView->currentIndex();
                    if (!index.isValid())
                        return;

                    onRowSelected(index);
                });
    }

    if (m_catalogType == CatalogType::Type::Investigations &&
        m_formType == CatalogType::FormType::List) {

        QToolButton *btnTreeInvestig = new QToolButton(this);
        btnTreeInvestig->setText(tr("Arbore"));
        btnTreeInvestig->setIcon(QIcon(":/img/catalogs/tree_yellow.png"));
        btnTreeInvestig->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        btnTreeInvestig->setStyleSheet(toolButtonStyleForText);
        connect(btnTreeInvestig, &QToolButton::clicked,
                this, &CatalogTableEditor::onOpenTreeInvestigations, Qt::UniqueConnection);
        ui->layoutToolBar->addSpacing(10);
        ui->layoutToolBar->addWidget(btnTreeInvestig);

        QToolButton *btnUltrasoundTariff = new QToolButton(this);
        btnUltrasoundTariff->setText(tr("Catalogul tarife unice 2024a."));
        btnUltrasoundTariff->setStyleSheet(toolButtonStyleForText);
        connect(btnUltrasoundTariff, &QToolButton::clicked,
                this, &CatalogTableEditor::onOpenUltrasoundTariffClassifier, Qt::UniqueConnection);
        ui->layoutToolBar->addSpacing(10);
        ui->layoutToolBar->addWidget(btnUltrasoundTariff);
    }
    ui->layoutToolBar->addStretch();

    connect(toolBar, &ToolBarCustom::addDoc,
            this, &CatalogTableEditor::onAdd, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::editDoc,
            this, &CatalogTableEditor::onEdit, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::deleteDoc,
            this, &CatalogTableEditor::onDelete, Qt::UniqueConnection);
}

QString CatalogTableEditor::getTextQueryByCatalogType() const
{
    QString sql;
    switch (m_catalogType) {
    case CatalogType::Type::Investigations:
        sql = "SELECT * FROM investigations ORDER BY cod"; break;
    case CatalogType::Type::TypesPrices:
        sql = "SELECT * FROM typesPrices ORDER BY name"; break;
    case CatalogType::Type::ConclusionTemplates:
        sql = "SELECT * FROM conclusionTemplates";
        if (!m_filterQuery.isEmpty())
            sql += QString(" WHERE system = '%1'").arg(m_filterQuery);
        sql += " ORDER BY system, name";
        break;
    case CatalogType::Type::SystemTemplates:
        sql = "SELECT * FROM formationsSystemTemplates";
        if (!m_filterQuery.isEmpty())
            sql += QString(" WHERE typeSystem = '%1'").arg(m_filterQuery);
        sql += " ORDER BY typeSystem, name";
        break;
    default:
        break;
    }
    return sql;
}

void CatalogTableEditor::registerColumnTableByCatalogType()
{
    auto FontRule = [](const QVariantMap &row) -> QVariant {
        QFont font;
        bool changed = false;

        if (row.value("deletionMark").toInt() == 1) {
            font.setItalic(true);
            font.setStrikeOut(true);
            changed = true;
        }

        return changed ? QVariant(font) : QVariant();
    };

    auto ForengroundRule = [](const QVariantMap &row) ->QVariant {
        const bool isDeleted = row.value("deletionMark").toInt() == 1;

        // deleted
        if (isDeleted) {
            if (globals().isSystemThemeDark) {
                return QBrush(QColor(200, 140, 180));
            } else {
                return QBrush(QColor(255, 230, 255));
            }
        }

        return {};
    };


    // ID
    auto idCol = std::make_unique<FieldColumn>("ID", "id", false, Qt::AlignCenter);
    idCol->setFontRule(FontRule);
    idCol->setForegroundRule(ForengroundRule);
    model->registerColumn(std::move(idCol));

    // deletionMark
    auto markCol = std::make_unique<FieldColumn>("", "deletionMark", false, Qt::AlignCenter);

    markCol->setDecorationRule([](const QVariantMap &row) -> QVariant {
        const int mark = row.value("deletionMark").toInt();

        if (mark == 0)
            return QIcon(":/img/catalogs/item.png");
        if (mark == 1)
            return QIcon(":/img/catalogs/item_delete.png");

        return {};
    });

    // nu afisam DisplayRole pu 0 si 1 deletionMark
    markCol->setDisplayFormatter([](const QVariant &) -> QVariant {
        return {};
    });

    markCol->setFontRule(FontRule);
    markCol->setForegroundRule(ForengroundRule);
    model->registerColumn(std::move(markCol));

    if (m_catalogType == CatalogType::Type::Investigations) {

        // Cod
        auto codCol = std::make_unique<FieldColumn>("Cod MS", "cod", editableColumn, Qt::AlignCenter);
        codCol->setFontRule(FontRule);
        codCol->setForegroundRule(ForengroundRule);

        codCol->setNormalizer([](const QVariant &v) -> QVariant {
            return v.toString().trimmed();
        });

        codCol->setValidator([](const QVariant &v, QString *err) -> bool {
            const QString s = v.toString().trimmed();
            if (s.isEmpty()) {
                if (err) *err = QObject::tr("Câmpul 'Cod' este obligatoriu.");
                return false;
            }
            return true;
        });
        model->registerColumn(std::move(codCol));

        // Name
        auto nameCol = std::make_unique<FieldColumn>("Denumirea investigatiei", "name", editableColumn, Qt::AlignLeft | Qt::AlignVCenter);
        nameCol->setFontRule(FontRule);
        nameCol->setForegroundRule(ForengroundRule);

        nameCol->setNormalizer([](const QVariant &v) -> QVariant {
            return v.toString().trimmed();
        });

        nameCol->setValidator([](const QVariant &v, QString *err) -> bool {
            const QString s = v.toString().trimmed();
            if (s.isEmpty()) {
                if (err) *err = QObject::tr("Câmpul 'Denumirea investigatiei' este obligatoriu.");
                return false;
            }
            return true;
        });
        model->registerColumn(std::move(nameCol));

        // Use
        auto useCol = std::make_unique<CheckColumn>("Utilizare", "use", editableColumn);
        useCol->setFontRule(FontRule);
        useCol->setForegroundRule(ForengroundRule);
        model->registerColumn(std::move(useCol));

        // Owner
        auto ownerCol = std::make_unique<ComboColumn>("Sistema", "owner", editableColumn, Qt::AlignLeft | Qt::AlignVCenter);
        ownerCol->setFontRule(FontRule);
        ownerCol->setForegroundRule(ForengroundRule);

        ownerCol->setNullValue(0);
        ownerCol->setAllowNull(false);
        ownerCol->setStoreNullInsteadOfNullValue(false);
        ownerCol->setRequiredErrorText(tr("Trebuie selectată sistema investigației."));

        model->registerColumn(std::move(ownerCol));

    } else if (m_catalogType == CatalogType::Type::TypesPrices) {

        // Name
        auto nameCol = std::make_unique<FieldColumn>("Denumirea pretului", "name", editableColumn, Qt::AlignLeft | Qt::AlignVCenter);
        nameCol->setFontRule(FontRule);
        nameCol->setForegroundRule(ForengroundRule);

        nameCol->setNormalizer([](const QVariant &v) -> QVariant {
            return v.toString().trimmed();
        });

        nameCol->setValidator([](const QVariant &v, QString *err) -> bool {
            const QString s = v.toString().trimmed();
            if (s.isEmpty()) {
                if (err) *err = QObject::tr("Câmpul 'Denumirea pretului' este obligatoriu.");
                return false;
            }
            return true;
        });
        model->registerColumn(std::move(nameCol));

        // Discount
        auto discountCol = std::make_unique<NumberColumn>(
            "Discount", "discount", 2, true, Qt::AlignLeft | Qt::AlignVCenter);
        discountCol->setFontRule(FontRule);
        discountCol->setForegroundRule(ForengroundRule);
        model->registerColumn(std::move(discountCol));

        // NonComecrial
        auto noncomercialCol = std::make_unique<CheckColumn>("Non Comercial", "noncomercial", editableColumn);
        noncomercialCol->setFontRule(FontRule);
        noncomercialCol->setForegroundRule(ForengroundRule);
        model->registerColumn(std::move(noncomercialCol));

    } else if (m_catalogType == CatalogType::Type::ConclusionTemplates) {

        // Cod
        auto codCol = std::make_unique<FieldColumn>("Cod", "cod", editableColumn, Qt::AlignCenter);
        codCol->setFontRule(FontRule);
        codCol->setForegroundRule(ForengroundRule);

        codCol->setNormalizer([](const QVariant &v) -> QVariant {
            return v.toString().trimmed();
        });

        codCol->setValidator([](const QVariant &v, QString *err) -> bool {
            const QString s = v.toString().trimmed();
            if (s.isEmpty()) {
                if (err) *err = QObject::tr("Câmpul 'Cod' este obligatoriu.");
                return false;
            }
            return true;
        });
        model->registerColumn(std::move(codCol));

        // Name
        auto nameCol = std::make_unique<FieldColumn>("Denumirea investigatiei", "name", editableColumn, Qt::AlignLeft | Qt::AlignVCenter);
        nameCol->setFontRule(FontRule);
        nameCol->setForegroundRule(ForengroundRule);

        nameCol->setNormalizer([](const QVariant &v) -> QVariant {
            return v.toString().trimmed();
        });

        nameCol->setValidator([](const QVariant &v, QString *err) -> bool {
            const QString s = v.toString().trimmed();
            if (s.isEmpty()) {
                if (err) *err = QObject::tr("Câmpul 'Denumirea investigatiei' este obligatoriu.");
                return false;
            }
            return true;
        });
        model->registerColumn(std::move(nameCol));

        // System
        auto systemCol = std::make_unique<FieldColumn>("Sistema", "system", editableColumn, Qt::AlignLeft | Qt::AlignVCenter);
        systemCol->setFontRule(FontRule);
        systemCol->setForegroundRule(ForengroundRule);
        model->registerColumn(std::move(systemCol));

    } else if (m_catalogType == CatalogType::Type::SystemTemplates) {

        // Name
        auto nameCol = std::make_unique<FieldColumn>(
            "Name", "name", editableColumn, Qt::AlignLeft | Qt::AlignVCenter);
        nameCol->setFontRule(FontRule);
        nameCol->setForegroundRule(ForengroundRule);

        nameCol->setNormalizer([](const QVariant &v) -> QVariant {
            return v.toString().trimmed();
        });

        nameCol->setValidator([](const QVariant &v, QString *err) -> bool {
            const QString s = v.toString().trimmed();
            if (s.isEmpty()) {
                if (err) *err = QObject::tr("Câmpul 'Name' este obligatoriu.");
                return false;
            }
            return true;
        });
        model->registerColumn(std::move(nameCol));

        // System
        auto systemCol = std::make_unique<FieldColumn>(
            "System", "typeSystem", editableColumn, Qt::AlignLeft | Qt::AlignVCenter);
        systemCol->setFontRule(FontRule);
        systemCol->setForegroundRule(ForengroundRule);
        model->registerColumn(std::move(systemCol));

    }
}

void CatalogTableEditor::setDelegatesFromTable()
{
    switch (m_catalogType) {
    case CatalogType::Type::Investigations: {
        auto cbDelegat = new CheckBoxDelegate(this);
        connect(cbDelegat, &QAbstractItemDelegate::closeEditor,                  // daca apasam pe ESC close editor
                this, &CatalogTableEditor::onCloseEditor, Qt::UniqueConnection); // stergem randul
        ui->tableView->setItemDelegateForColumn(InvestigationsSections::Use,
                                                cbDelegat);

        auto *delegate = new ComboDelegate(ui->tableView);
        delegate->setQuery("SELECT id,name FROM investigationsGroup");
        delegate->setIdColumn(0);
        delegate->setTextColumn(1);
        delegate->setAllowNullItem(true);
        delegate->setNullText(tr("<<- selectează ->>"));
        delegate->setNullValue(0);
        connect(delegate, &QAbstractItemDelegate::closeEditor,                   // daca apasam pe ESC close editor/popup
                this, &CatalogTableEditor::onCloseEditor, Qt::UniqueConnection); // stergem randul
        ui->tableView->setItemDelegateForColumn(InvestigationsSections::Owner, delegate);
        break;
    }
    case CatalogType::Type::TypesPrices: {
        auto cbDelegat = new CheckBoxDelegate(this);
        connect(cbDelegat, &QAbstractItemDelegate::closeEditor,                  // daca apasam pe ESC close editor
                this, &CatalogTableEditor::onCloseEditor, Qt::UniqueConnection); // stergem randul
        ui->tableView->setItemDelegateForColumn(TypePricesSections::Noncomercial,
                                                cbDelegat);
        break;
    }
    default:
        break;
    }
}

void CatalogTableEditor::initTableView()
{
    registerColumnTableByCatalogType();

    ui->tableView->setModel(model);
    ui->tableView->setColumnHidden(InvestigationsSections::Id, true);
    ui->tableView->setColumnHidden(InvestigationsSections::Uuid, true);

    setDelegatesFromTable();

    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    connect(model, &QAbstractItemModel::dataChanged,
            this, &CatalogTableEditor::onDataChangedItemModel, Qt::QueuedConnection);

    connect(model, &BaseAbstractModel::validationFailed,
            this, &CatalogTableEditor::onValidationFailed, Qt::QueuedConnection);

    connect(ui->tableView->itemDelegate(), &QAbstractItemDelegate::closeEditor,
            this, &CatalogTableEditor::onCloseEditor, Qt::UniqueConnection);

    if (m_formType == CatalogType::FormType::Selection) {
        connect(ui->tableView, QOverload<const QModelIndex&>::of(&QTableView::doubleClicked),
                this, &CatalogTableEditor::onRowSelected);
    }
}

void CatalogTableEditor::updateTableView()
{
    int currentId = -1;

    const QModelIndex currentIndex = ui->tableView->currentIndex();
    if (currentIndex.isValid())
        currentId = model->idByRow(currentIndex.row());

    QList<QVariantMap> rows;

    QSqlQuery q;
    q.prepare(getTextQueryByCatalogType());
    if (!q.exec()) {
        qWarning(logWarning()).noquote()
        << "CatalogTableEditor: updateTableView() error:"
        << q.lastError().text();
        model->clear();
        return;
    }

    while (q.next()) {
        QVariantMap row;
        row.insert("id",           q.value("id"));
        row.insert("deletionMark", q.value("deletionMark"));
        switch (m_catalogType) {
        case CatalogType::Type::Investigations:
            row.insert("cod",   q.value("cod"));
            row.insert("name",  q.value("name"));
            row.insert("use",   q.value("use"));
            row.insert("owner", q.value("owner"));
            break;
        case CatalogType::Type::TypesPrices:
            row.insert("name",         q.value("name"));
            row.insert("discount",     q.value("discount"));
            row.insert("noncomercial", q.value("noncomercial"));
            break;
        case CatalogType::Type::ConclusionTemplates:
            row.insert("cod",    q.value("cod"));
            row.insert("name",   q.value("name"));
            row.insert("system", q.value("system"));
            break;
        case CatalogType::Type::SystemTemplates:
            row.insert("name",       q.value("name"));
            row.insert("typeSystem", q.value("typeSystem"));
            break;
        default:
            break;
        }
        row.insert("uuid", q.value("uuid"));
        rows.append(row);
    }

    model->setRows(rows);

    if (currentId >= 0) {
        const int row = model->rowById(currentId);
        if (row >= 0) {
            ui->tableView->selectRow(row);
            ui->tableView->scrollTo(model->index(row, 0));
        }
    }
}

QString CatalogTableEditor::getNameTableByCatalogType()
{
    switch (m_catalogType) {
    case CatalogType::Type::Investigations:
        return "investigations";
    case CatalogType::Type::TypesPrices:
        return "typesPrices";
    case CatalogType::Type::ConclusionTemplates:
        return "conclusionTemplates";
    case CatalogType::Type::SystemTemplates:
        return "formationsSystemTemplates";
    default:
        break;
    }
    return "";
}

QVariantMap CatalogTableEditor::getDataByCatalogType(bool insert)
{
    QVariantMap data;
    data["id"] = 0;
    data["deletionMark"] = 0;

    switch (m_catalogType) {
    case CatalogType::Type::Investigations:
        data["cod"]   = "";
        data["name"]  = "";
        data["use"]   = 0;
        data["owner"] = 0;
        break;
    case CatalogType::Type::TypesPrices:
        data["name"]         = "";
        data["discount"]     = 0;
        data["noncomercial"] = 1;
        break;
    case CatalogType::Type::ConclusionTemplates:
        data["cod"]    = "";
        data["name"]   = "";
        data["system"] = "";
        break;
    case CatalogType::Type::SystemTemplates:
        data["name"]       = "";
        data["typeSystem"] = "";
        break;
    default:
        break;
    }

    if (insert)
        data["uuid"] = QUuid::createUuid().toRfc4122();

    return data;
}

bool CatalogTableEditor::saveCurrentRowToDatabase(int row)
{
    if (!model || row < 0)
        return false;

    QVariantMap rowData = model->rowDataByRow(row);
    if (rowData.isEmpty())
        return false;

    QString errorText;
    const int id = rowData.value("id").toInt();
    QSqlDatabase db = m_db.getDatabase();

    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::warning(this, tr("Eroare"), tr("Baza de date nu este deschisă."));
        return false;
    }

    if (!db.transaction()) {
        QMessageBox::warning(this,
                             tr("Eroare"),
                             tr("Nu s-a putut porni tranzacția:\n%1")
                                 .arg(db.lastError().text()));
        return false;
    }

    const auto restoreInsertedRow = [&]() {
        if (id == 0 && row < model->rowCount())
            model->updateRowByIndex(row, rowData);
    };

    bool ok = false;

    if (id == 0)
        ok = model->insertRowToDatabase(m_db, getNameTableByCatalogType(), row, &errorText);
    else
        ok = model->updateRowToDatabase(m_db, getNameTableByCatalogType(), row, &errorText);

    if (!ok) {
        const QString operationError = errorText;
        if (!db.rollback()) {
            errorText = tr("%1\nRollback eșuat: %2")
                            .arg(operationError, db.lastError().text());
        }
        restoreInsertedRow();
        QMessageBox::warning(this, tr("Eroare"), errorText);
        return false;
    }

    if (!db.commit()) {
        const QString commitError = db.lastError().text();
        QString rollbackError;
        if (!db.rollback())
            rollbackError = tr("\nRollback eșuat: %1").arg(db.lastError().text());

        restoreInsertedRow();
        QMessageBox::warning(this,
                             tr("Eroare"),
                             tr("Commit-ul salvării catalogului a eșuat: %1%2")
                                 .arg(commitError, rollbackError));
        return false;
    }

    m_modifiedRows.remove(row);

    return true;
}

void CatalogTableEditor::reject()
{
    if (auto *sub = qobject_cast<QMdiSubWindow *>(parentWidget())) {
        sub->close();      // inchide subfereastra MDI si dialogul intern
        return;
    }

    QDialog::reject();     // fallback normal
}

void CatalogTableEditor::closeEvent(QCloseEvent *event)
{
    saveSizeSection();

    // daca utilizatorul nu a trecut prin toate celulele
    // vezi functia onDataChangedItemModel()
    // si a apasat ESC save/update row nu s-a efectuat
    // atunci efectuam update la inchiderea widgetului
    if (!m_modifiedRows.isEmpty()) {
        const auto rows = m_modifiedRows.values();  // copiem
        for (int row : rows) {
            if (!saveCurrentRowToDatabase(row)) {
                event->ignore();
                return;
            }
        }
    }

    QWidget::closeEvent(event);
}

bool CatalogTableEditor::eventFilter(QObject *obj, QEvent *event)
{
    return QDialog::eventFilter(obj, event);
}

void CatalogTableEditor::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        setWindowTitle(tr("Catalog: %1")
                           .arg(CatalogType::enumToStringRo(m_catalogType)));
    }
}

void CatalogTableEditor::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_End)
        ui->tableView->selectRow(model->rowCount() - 1);
    if (event->key() == Qt::Key_Home)
        ui->tableView->selectRow(0);
}
