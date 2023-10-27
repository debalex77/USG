#include "catinvestigations.h"
#include "ui_catinvestigations.h"

CatInvestigations::CatInvestigations(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CatInvestigations)
{
    ui->setupUi(this);

    setWindowTitle(tr("Clasificatorul investigațiilor"));

    ui->btnAdd->setIcon(QIcon(":/img/add_x32.png"));
    ui->btnAdd->setStyleSheet("padding-left: 3px; text-align: left;");
    ui->btnAdd->setShortcut(QKeySequence(Qt::Key_Insert));
    ui->btnAdd->setLayout(new QGridLayout);

    ui->btnMarkDeletion->setIcon(QIcon(":/img/clear_x32.png"));
    ui->btnMarkDeletion->setStyleSheet("padding-left: 3px; text-align: left;");
    ui->btnMarkDeletion->setShortcut(QKeySequence(Qt::Key_Delete));
    ui->btnMarkDeletion->setLayout(new QGridLayout);

    ui->btnClose->setIcon(QIcon(":/img/close_x32.png"));
    ui->btnClose->setStyleSheet("padding-left: 3px; text-align: left;");
    ui->btnClose->setShortcut(QKeySequence(Qt::Key_Escape));
    ui->btnClose->setLayout(new QGridLayout);

    QStringList _headers;
    _headers << tr("id")
             << tr("") // deletionMark
             << tr("Cod MS")
             << tr("Denumirea investigației")
             << tr("Actual");

    db = new DataBase(this);
    menu = new QMenu(this);

    model = new BaseSqlTableModel(this);
    model->setTable("investigations");

    for(int i = 0, j = 0; i < model->columnCount(); i++, j++){
        model->setHeaderData(i, Qt::Horizontal, _headers[j]);
    }

    model->setEditStrategy(QSqlTableModel::OnFieldChange); // Înregistrarea modificărilor va fi executată
                                                           // automat după editarea celule

    model->setSort(0,Qt::AscendingOrder);        // sortarea datelor de la sectia 0
    ui->tableView->setModel(model);

    model->select();

    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);     // initializam meniu contextual
    ui->tableView->setColumnHidden(0, true);                        // ascundem id
//    ui->tableView->resizeColumnsToContents();                     // dimensiunea colonitelor dupa conținut
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    ui->tableView->setColumnWidth(1, 5);
    ui->tableView->setColumnWidth(2, 70);
    ui->tableView->setColumnWidth(3, 1200);

    ui->tableView->selectRow(0);

    connect(ui->btnAdd, &QAbstractButton::clicked, this, &CatInvestigations::onAddRowTable);
    connect(ui->btnMarkDeletion, &QAbstractButton::clicked, this, &CatInvestigations::onMarkDeletion);
    connect(ui->btnClose, &QAbstractButton::clicked, this, &CatInvestigations::onClose);

    connect(ui->tableView, &QWidget::customContextMenuRequested, this, &CatInvestigations::slotContextMenuRequested);
}

CatInvestigations::~CatInvestigations()
{
    delete db;
    delete menu;
    delete model;
    delete ui;
}

void CatInvestigations::onAddRowTable()
{
    int row = model->rowCount();
    model->insertRow(row);
    model->setData(model->index(row, 0), row + 1); // index(row, 0) - id
    model->setData(model->index(row, 1), 0);       // index(row, 1) - markDeletion
    model->setData(model->index(row, 4), 1);       // index(row, 4) - use
    model->submitAll();                            // acceptam transferarea in SQL

    ui->tableView->edit(model->index(row, 2));
    ui->tableView->selectRow(row);
}

void CatInvestigations::onMarkDeletion()
{
    if (ui->tableView->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Atenție"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    int _row     = ui->tableView->currentIndex().row();
    int _id      = model->index(_row, 0).data(Qt::DisplayRole).toInt();
    int _delMark = model->index(_row, 1).data(Qt::DisplayRole).toInt();
    QString _cod = model->index(_row, 2).data(Qt::DisplayRole).toString();

    if (!db->deletionMarkObject("investigations", _id)){
        QMessageBox::warning(this, tr("Marcarea/demarcarea obiectului"),
                             tr("Marcarea/demarcarea obiectului cu codul \"<b>%1</b>\" nu este reusita.").arg(_cod), QMessageBox::Ok);
        return;
    }
    if (_delMark == 0)
        model->setData(model->index(_row, 4), 1); // setam ca este actual
    else
        model->setData(model->index(_row, 4), 0);

    model->select();
}

void CatInvestigations::onClose()
{
    this->close();
    emit mCloseThisForm();
}

void CatInvestigations::slotContextMenuRequested(QPoint pos)
{
    int _row = ui->tableView->currentIndex().row();
    int _id = model->index(_row, 0).data(Qt::DisplayRole).toInt();
    QString _cod = model->index(_row, 2).data(Qt::DisplayRole).toString();

    QString strActionDelMark;
    int statusDeletionMark = db->statusDeletionMarkObject("investigations", _id);
    if (statusDeletionMark == DataBase::REQUIRED_NUMBER::DELETION_MARK){
        strActionDelMark = tr("Demarchează obiectul cu codul '%1'").arg(_cod);
    } else if (statusDeletionMark == DataBase::REQUIRED_NUMBER::DELETION_UNMARK){
        strActionDelMark = tr("Marchează pentru eliminare obiectul co codul '%1'").arg(_cod);
    } else {
     qDebug() << tr("%1 - slotContextMenuRequested():").arg(metaObject()->className())
              << tr("Status 'deletionMark' a obiectului cu ID=%1 nu este determinat !!!").arg(QString::number(_id));
    }

    QAction* actionAddObject  = new QAction(QIcon(":/img/add_x32.png"), tr("Adaugă obiect nou"), this);
    QAction* actionMarkObject = new QAction(QIcon(":/img/clear_x32.png"), strActionDelMark, this);

    connect(actionAddObject, &QAction::triggered, this, &CatInvestigations::onAddRowTable);
    connect(actionMarkObject, &QAction::triggered, this, &CatInvestigations::onMarkDeletion);

    menu->clear();
    menu->addAction(actionAddObject);
    menu->addAction(actionMarkObject);
    menu->popup(ui->tableView->viewport()->mapToGlobal(pos)); // prezentarea meniului
}

void CatInvestigations::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        // traducem  titlu
        setWindowTitle(tr("Clasificatorul investigațiilor"));
    }
}
