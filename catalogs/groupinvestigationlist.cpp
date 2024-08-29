#include "groupinvestigationlist.h"
#include "ui_groupinvestigationlist.h"

#include <QStringListModel>

GroupInvestigationList::GroupInvestigationList(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GroupInvestigationList)
{
    ui->setupUi(this);

    setWindowTitle(tr("Arbore investigațiilor"));
    setWindowIcon(QIcon(":/img/folder_yellow.png"));

    updateTable();

    connect(ui->btnPrint, &QAbstractButton::clicked, this, &GroupInvestigationList::onPrint);
    connect(ui->hideNotGroup, QOverload<int>::of(&QCheckBox::stateChanged), this, QOverload<int>::of(&GroupInvestigationList::onHideNotGroupStateChanged));
    connect(ui->btnAdd, &QAbstractButton::clicked, this, &GroupInvestigationList::onCreateNewGroup);
    connect(ui->btnClose, &QAbstractButton::clicked, this, &GroupInvestigationList::close);
}

GroupInvestigationList::~GroupInvestigationList()
{
    delete model_tree;
    delete db;
    delete ui;
}

void GroupInvestigationList::onCreateNewGroup()
{
    item_group = new GroupInvestigation(this);
    item_group->setAttribute(Qt::WA_DeleteOnClose);
    item_group->show();
}

void GroupInvestigationList::onHideNotGroupStateChanged(int state)
{
    Q_UNUSED(state)
    updateTable();
}

void GroupInvestigationList::updateTable()
{
    QString result;

    db = new DataBase(this);
    QSqlQuery qry;
    QSqlQuery qry_items;
    qry.prepare("SELECT owner FROM tmp_investigations WHERE owner NOT NULL OR owner <> '' GROUP BY owner;");
    if (qry.exec()){
        while (qry.next()) {

            result = result + "gr_" + qry.value(0).toString() + "\n";

            qry_items.prepare(QString("SELECT cod, name FROM tmp_investigations WHERE `use` = 1 AND owner = '%1' ORDER BY cod;").arg(qry.value(0).toString()));
            if (qry_items.exec()){
                while (qry_items.next()) {
                    result = result + " it_" + qry_items.value(0).toString() + " - " + qry_items.value(1).toString() + "\n";
                }
            }
        }
    }

    if (! ui->hideNotGroup->isChecked()){
        qry_items.prepare("SELECT cod, name FROM tmp_investigations WHERE `use` = 0 ORDER BY cod;");
        if (qry_items.exec()){
            while (qry_items.next()) {
                result = result + "it_" + qry_items.value(0).toString() + " - " + qry_items.value(1).toString() + "\n";
            }
        }
    }

    model_tree = new TreeModel(false, QStringList(), result, this);
    ui->treeView->setModel(model_tree);
    ui->treeView->expandAll();
}

void GroupInvestigationList::onPrint()
{
    this->hide();

    QDir dir;
    print_model = new QSqlQueryModel(this);
    print_model->setQuery("SELECT owner FROM tmp_investigations WHERE owner NOT NULL AND owner <> '' GROUP BY owner;");

    m_report = new LimeReport::ReportEngine(this);
    m_report->dataManager()->addModel("list_group", print_model, true);

    LimeReport::ICallbackDatasource *callbackDatasource = m_report->dataManager()->createCallbackDatasource("test");

    connect(callbackDatasource, QOverload<const LimeReport::CallbackInfo &, QVariant &>::of(&LimeReport::ICallbackDatasource::getCallbackData),
            this, QOverload<LimeReport::CallbackInfo, QVariant &>::of(&GroupInvestigationList::slotGetCallbackChildData));

    connect(callbackDatasource, QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&LimeReport::ICallbackDatasource::changePos),
            this, QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&GroupInvestigationList::slotChangeChildPos));

    m_report->dataManager()->clearUserVariables();
    m_report->setShowProgressDialog(true);
    m_report->setPreviewWindowTitle(tr("Arbore investigațiilor"));
    m_report->loadFromFile(dir.toNativeSeparators(globals::pathTemplatesDocs + "/Tree_investigations.lrxml"));
    m_report->designReport();

    delete print_model;
    delete m_report;

    this->show();
}

void GroupInvestigationList::slotGetCallbackChildData(LimeReport::CallbackInfo info, QVariant &data)
{
    QSqlQuery *qry = new QSqlQuery(db->getDatabase());
    qry->prepare("SELECT * FROM tmp_investigations WHERE owner = 'Screening';");
    qry->exec();
    prepareData(qry, info, data);
}

void GroupInvestigationList::slotChangeChildPos(const LimeReport::CallbackInfo::ChangePosType &type, bool &result)
{
    QSqlQuery *ds = new QSqlQuery(db->getDatabase());
    ds->prepare("SELECT * FROM tmp_investigations WHERE owner = 'Screening';");
    ds->exec();
    if (! ds)
        return;

    if (type == LimeReport::CallbackInfo::First)
        result = ds->first();
    else
        result = ds->next();
}

void GroupInvestigationList::prepareData(QSqlQuery *qry, LimeReport::CallbackInfo info, QVariant &data)
{
    switch (info.dataType) {
    case LimeReport::CallbackInfo::ColumnCount:
        data = qry->record().count();
        break;
    case LimeReport::CallbackInfo::IsEmpty:
        data = ! qry->first();
        break;
    case LimeReport::CallbackInfo::HasNext:
        data = qry->next();
        qry->previous();
        break;
    case LimeReport::CallbackInfo::ColumnHeaderData:
        if (info.index < qry->record().count())
            data = qry->record().fieldName(info.index);
        break;
    case LimeReport::CallbackInfo::ColumnData:
        data = qry->value(qry->record().indexOf(info.columnName));
        break;
    default:
        break;
    }
}
