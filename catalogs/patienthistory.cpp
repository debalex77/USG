#include "patienthistory.h"
#include "ui_patienthistory.h"

#include <docs/docreportecho.h>

PatientHistory::PatientHistory(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PatientHistory)
{
    ui->setupUi(this);

    setWindowTitle(tr("Istoria pacientului"));

    db = new DataBase(this);
    completer = new QCompleter(this);
    model_patients = new QStandardItemModel(completer);
    QString str_qry;
    model_table = new BaseSqlQueryModel(str_qry, ui->tableView);
    proxy = new BaseSortFilterProxyModel(this);

    ui->btnClearPatient->setStyleSheet("border: 1px solid #8f8f91; "
                                          "border-radius: 4px;");
    ui->btnOpenCatPatient->setStyleSheet("border: 1px solid #8f8f91; "
                                          "border-radius: 4px;");

    initSetCompleter();
    connect(ui->comboPatient, &QComboBox::currentTextChanged, this, &PatientHistory::filterRegExpChangedPacient);
    connect(this, &PatientHistory::IdPatientChanged, this, &PatientHistory::slot_IdPatientChanged);
    connect(ui->btnClearPatient, &QAbstractButton::clicked, this, &PatientHistory::clearDataComboPatient);
    connect(ui->btnOpenCatPatient, &QAbstractButton::clicked, this, &PatientHistory::openCatPatient);

    connect(ui->tableView, QOverload<const QModelIndex&>::of(&QTableView::clicked), this, &PatientHistory::onClickedTable);
    connect(ui->tableView, QOverload<const QModelIndex&>::of(&QTableView::doubleClicked), this, &PatientHistory::onDoubleClickedTable);
}

PatientHistory::~PatientHistory()
{
    delete model_table;
    delete model_patients;
    delete completer;
    delete proxy;
    delete db;
    delete ui;
}

void PatientHistory::slot_IdPatientChanged()
{
    if (m_id_patient == -1)
        return;

    disconnect(ui->comboPatient, &QComboBox::currentTextChanged, this, &PatientHistory::filterRegExpChangedPacient);
    disconnect(completer, QOverload<const QModelIndex&>::of(&QCompleter::activated),
            this, QOverload<const QModelIndex&>::of(&PatientHistory::activatedItemCompleter));

    auto indexPacient = model_patients->match(model_patients->index(0,0), Qt::UserRole, m_id_patient, 1, Qt::MatchExactly);
    if (!indexPacient.isEmpty()){
        ui->comboPatient->setCurrentIndex(indexPacient.first().row());
        completer->setCurrentRow(indexPacient.first().row());
        ui->comboPatient->setCurrentText(completer->currentCompletion());
        setWindowTitle(tr("Istoria pacientului - %1").arg(ui->comboPatient->currentText()));
    }

    connect(ui->comboPatient, &QComboBox::currentTextChanged, this, &PatientHistory::filterRegExpChangedPacient);
    connect(completer, QOverload<const QModelIndex&>::of(&QCompleter::activated),
            this, QOverload<const QModelIndex&>::of(&PatientHistory::activatedItemCompleter));

    updateTableDoc();
    loadImagesPatients();
}

void PatientHistory::filterRegExpChangedPacient()
{
    proxy->setFilterKeyColumn(1);
    QRegularExpression regExp(ui->comboPatient->currentText(), QRegularExpression::CaseInsensitiveOption);
    proxy->setFilterRegularExpression(regExp);
}

void PatientHistory::activatedItemCompleter(const QModelIndex &index)
{
    if (! index.isValid())
        return;

    const int id = index.data(Qt::UserRole).toInt();

    if (id <= 0)
        return;

    setIdPatient(id); // setam id
    updateTableDoc(); // actualizam tabela
}

void PatientHistory::onClickedTable(const QModelIndex &index)
{
    if (! index.isValid())
        return;

    // imagine
    loadImagesPatients();
}

void PatientHistory::onDoubleClickedTable(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    const int row = index.row();
    const int id_doc = model_table->data(model_table->index(row, 0), Qt::DisplayRole).toInt();
    DocReportEcho* report = new DocReportEcho(this);
    report->setAttribute(Qt::WA_DeleteOnClose);
    report->setProperty("ItNew", false);
    report->setProperty("Id", id_doc);
    report->show();
}

void PatientHistory::clearDataComboPatient()
{
    setIdPatient(-1);
    ui->comboPatient->setCurrentIndex(0);
    ui->comboPatient->setCurrentText("");
    updateTableDoc();
    ui->diagnoza->setPlainText("");
    ui->image1->setText(tr("Imagine lipsește"));
    ui->image_2->setText(tr("Imagine lipsește"));
    ui->image_3->setText(tr("Imagine lipsește"));
    ui->image_4->setText(tr("Imagine lipsește"));
    ui->image_5->setText(tr("Imagine lipsește"));
}

void PatientHistory::openCatPatient()
{
    if (m_id_patient == -1)
        return;

    cat_patients = new CatGeneral(this);
    cat_patients->setAttribute(Qt::WA_DeleteOnClose);
    cat_patients->setProperty("itNew", false);
    cat_patients->setProperty("typeCatalog", CatGeneral::TypeCatalog::Pacients);
    cat_patients->setProperty("Id", m_id_patient);
    cat_patients->show();
}

void PatientHistory::initSetCompleter()
{
    updateModelPatients();
    proxy->setSourceModel(model_patients);
    completer->setModel(proxy);                                  // setam model
    completer->setCaseSensitivity(Qt::CaseInsensitive);                 // cautare insenseibila la registru
    completer->setModelSorting(QCompleter::CaseSensitivelySortedModel); // modelul este sortat în mod sensibil.
    completer->setFilterMode(Qt::MatchContains);                        // contine elementul
    completer->setCompletionMode(QCompleter::PopupCompletion);

    ui->comboPatient->setEditable(true);
    ui->comboPatient->setCompleter(completer);

    connect(completer, QOverload<const QModelIndex&>::of(&QCompleter::activated),
            this, QOverload<const QModelIndex&>::of(&PatientHistory::activatedItemCompleter));
}

void PatientHistory::updateModelPatients()
{
    if (model_patients->rowCount() > 0)
        model_patients->clear();

    const QString strQuery =
        globals().thisMySQL ?
            QStringLiteral(
                "SELECT "
                "  pacients.id, "
                "  CONCAT(pacients.name, ' ', pacients.fName, ', ', "
                "  DATE_FORMAT(pacients.birthday, '%d.%m.%Y'), ', idnp: ', "
                "  IFNULL(pacients.IDNP, '')) AS FullName "
                "FROM "
                "  pacients "
                "WHERE "
                "  pacients.deletionMark = 0 "
                "ORDER BY "
                "  FullName ASC;"
            ) :
            QStringLiteral(
                "SELECT "
                "  pacients.id,"
                "  pacients.name ||' '|| pacients.fName "
                "  || ', ' || strftime('%d.%m.%Y', pacients.birthday) || ', idnp: ' || IFNULL(pacients.IDNP, '') AS FullName "
                "FROM "
                "  pacients "
                "WHERE "
                "  pacients.deletionMark = 0 "
                "ORDER BY "
                "  FullName ASC;"
            );
    QSqlQuery qry;
    if (! qry.exec(strQuery)) {
        qWarning() << "Eroare exec query:" << qry.lastError().text();
        return;
    }

    // pu performanta cream container
    QList<QStandardItem*> items;

    // prelucrarea solicitarii si completarea containerului 'items'
    while (qry.next()) {
        int     _id   = qry.value(0).toInt();
        QString _name = qry.value(1).toString();

        auto *item = new QStandardItem;
        item->setData(_id, Qt::UserRole);       // pentru identificator logic
        item->setData(_name, Qt::DisplayRole);  // ce se afișează în combobox

        items.append(item); // introducem in container
    }

    // adaugam toate randurile printr-o tranzactie/simultan (eficient si rapid)
    model_patients->invisibleRootItem()->appendRows(items);
}

void PatientHistory::updateTableDoc()
{
    QString str;
    str = QString("SELECT reportEcho.id,"
                  "reportEchoPresentation.docPresentation AS full_name,"
                  "reportEcho.concluzion "
                  "FROM reportEcho INNER JOIN reportEchoPresentation ON reportEchoPresentation.id_reportEcho = reportEcho.id WHERE id_pacients = '%1' AND reportEcho.deletionMark = 2;")
              .arg(QString::number(m_id_patient));

    model_table->setQuery(str);
    ui->tableView->setModel(model_table);
    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(2);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);  // setam alegerea randului
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection); // setam singura alegerea(nu multipla)
    ui->tableView->setSortingEnabled(true);                              // setam posibilitatea sortarii
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // permitem schimbarea size sectiilor
    ui->tableView->verticalHeader()->setDefaultSectionSize(30);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);  // extinderea ultimei sectiei
    ui->tableView->selectRow(0);
    model_table->setHeaderData(1, Qt::Horizontal, tr("Documente"));
}

void PatientHistory::loadImagesPatients()
{
    if (m_id_patient == -1)
        return;

    if (!ui->tableView->currentIndex().isValid())
        return;

    const int row = ui->tableView->currentIndex().row();
    const int id_doc = model_table->data(model_table->index(row, 0), Qt::DisplayRole).toInt();
    const QString concluzion = model_table->data(model_table->index(row, 2), Qt::DisplayRole).toString();
    if (!concluzion.isEmpty())
        ui->diagnoza->setPlainText(concluzion);

    // imagine
    QSqlQuery qry(db->getDatabaseImage());
    qry.prepare("SELECT * FROM imagesReports WHERE id_reportEcho = :id_reportEcho;");
    qry.bindValue(":id_reportEcho", id_doc);
    if (! qry.exec()){
        qWarning(logWarning()) << tr("Eroare de executare a solicitarii de extragere a imaginei: %1").arg(qry.lastError().text());
    } else {
        if (qry.next()){
            QSqlRecord rec = qry.record();
            QByteArray outByteArray1 = QByteArray::fromBase64(qry.value(rec.indexOf("image_1")).toByteArray());
            QByteArray outByteArray2 = QByteArray::fromBase64(qry.value(rec.indexOf("image_2")).toByteArray());
            QByteArray outByteArray3 = QByteArray::fromBase64(qry.value(rec.indexOf("image_3")).toByteArray());
            QByteArray outByteArray4 = QByteArray::fromBase64(qry.value(rec.indexOf("image_4")).toByteArray());
            QByteArray outByteArray5 = QByteArray::fromBase64(qry.value(rec.indexOf("image_5")).toByteArray());

            QPixmap outPixmap1 = QPixmap();
            if (! outByteArray1.isEmpty() && outPixmap1.loadFromData(outByteArray1))
                ui->image1->setPixmap(outPixmap1.scaled(600,344));
            else
                ui->image1->setText(tr("Imagine lipsește"));
            QPixmap outPixmap2 = QPixmap();
            if (! outByteArray2.isEmpty() && outPixmap2.loadFromData(outByteArray2))
                ui->image_2->setPixmap(outPixmap2.scaled(600,344));
            else
                ui->image_2->setText(tr("Imagine lipsește"));
            QPixmap outPixmap3 = QPixmap();
            if (! outByteArray3.isEmpty() && outPixmap3.loadFromData(outByteArray3))
                ui->image_3->setPixmap(outPixmap3.scaled(600,344));
            else
                ui->image_3->setText(tr("Imagine lipsește"));
            QPixmap outPixmap4 = QPixmap();
            if (! outByteArray4.isEmpty() && outPixmap4.loadFromData(outByteArray4))
                ui->image_4->setPixmap(outPixmap4.scaled(600,344));
            else
                ui->image_4->setText(tr("Imagine lipsește"));
            QPixmap outPixmap5 = QPixmap();
            if (! outByteArray5.isEmpty() && outPixmap5.loadFromData(outByteArray5))
                ui->image_5->setPixmap(outPixmap5.scaled(600,344));
            else
                ui->image_5->setText(tr("Imagine lipsește"));
        } else {
            ui->image1->setText(tr("Imagine lipsește"));
            ui->image_2->setText(tr("Imagine lipsește"));
            ui->image_3->setText(tr("Imagine lipsește"));
            ui->image_4->setText(tr("Imagine lipsește"));
            ui->image_5->setText(tr("Imagine lipsește"));
        }
    }
}
