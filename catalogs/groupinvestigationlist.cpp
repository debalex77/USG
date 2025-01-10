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
    qry.prepare("SELECT owner FROM investigations WHERE owner NOT NULL OR owner <> '' GROUP BY owner ORDER BY cod;");
    if (qry.exec()){
        while (qry.next()) {

            result = result + "gr_" + qry.value(0).toString() + "\n";

            qry_items.prepare(QString("SELECT cod, name FROM investigations WHERE `use` = 1 AND owner = '%1' ORDER BY cod;").arg(qry.value(0).toString()));
            if (qry_items.exec()){
                while (qry_items.next()) {
                    result = result + " it_" + qry_items.value(0).toString() + " - " + qry_items.value(1).toString() + "\n";
                }
            }
        }
    }

    if (! ui->hideNotGroup->isChecked()){
        qry_items.prepare("SELECT cod, name FROM investigations WHERE `use` = 0 ORDER BY cod;");
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

    m_report = new LimeReport::ReportEngine(this);
    m_report->setPreviewWindowTitle(tr("Arbore investiga\310\233iilor"));

    // solicitarea pentru grupe
    m_owner = new QSqlQuery("SELECT "
                            " investigations.owner, "
                            " investigationsGroup.cod "
                            "FROM "
                            " investigations "
                            "INNER JOIN "
                            " investigationsGroup ON investigations.owner = investigationsGroup.name "
                            "WHERE "
                            " investigations.owner IS NOT NULL "
                            "GROUP BY "
                            " investigationsGroup.cod "
                            "ORDER BY "
                            " investigationsGroup.cod ASC;");
    if (! m_owner->first()) {
        qCritical() << "Error: Unable to fetch investigationsGroup data:" << m_owner->lastError().text();
        delete m_report;
        return;
    }

    // solicitarea elementelor investigatiilor
    m_investigations = new QSqlQuery("SELECT "
                                     " investigations.cod, "
                                     " investigations.name "
                                     "FROM "
                                     " investigations "
                                     "WHERE "
                                     " investigations.owner = ?;");
    m_investigations->bindValue(0, "Org.interne"); // initial
    if ( !m_investigations->exec()) {
        qCritical() << "Error: Unable to execute investigations query:" << m_investigations->lastError().text();
        delete m_report;
        return;
    }

#if defined(Q_OS_LINUX)

    // list_owner
    LimeReport::ICallbackDatasource *callbackDatasource = m_report->dataManager()->createCallbackDatasource("list_owner");
    connect(callbackDatasource, QOverload<const LimeReport::CallbackInfo &, QVariant &>::of(&LimeReport::ICallbackDatasource::getCallbackData),
            this, QOverload<LimeReport::CallbackInfo, QVariant &>::of(&GroupInvestigationList::slotGetCallbackChildData));

    connect(callbackDatasource, QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&LimeReport::ICallbackDatasource::changePos),
            this, QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&GroupInvestigationList::slotChangeChildPos));

    // list_items
    callbackDatasource = m_report->dataManager()->createCallbackDatasource("list_items");
    connect(callbackDatasource, QOverload<const LimeReport::CallbackInfo &, QVariant &>::of(&LimeReport::ICallbackDatasource::getCallbackData),
            this, QOverload<LimeReport::CallbackInfo, QVariant &>::of(&GroupInvestigationList::slotGetCallbackChildData_items));

    connect(callbackDatasource, QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&LimeReport::ICallbackDatasource::changePos),
            this, QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&GroupInvestigationList::slotChangeChildPos_items));

#elif defined(Q_OS_MACOS)

    // list_owner
    LimeReport::ICallbackDatasource *callbackDatasource = m_report->dataManager()->createCallbackDatasource("list_owner");
    connect(callbackDatasource, QOverload<const LimeReport::CallbackInfo &, QVariant &>::of(&LimeReport::ICallbackDatasource::getCallbackData),
            this, QOverload<LimeReport::CallbackInfo, QVariant &>::of(&GroupInvestigationList::slotGetCallbackChildData));

    connect(callbackDatasource, QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&LimeReport::ICallbackDatasource::changePos),
            this, QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&GroupInvestigationList::slotChangeChildPos));

    // list_items
    callbackDatasource = m_report->dataManager()->createCallbackDatasource("list_items");
    connect(callbackDatasource, QOverload<const LimeReport::CallbackInfo &, QVariant &>::of(&LimeReport::ICallbackDatasource::getCallbackData),
            this, QOverload<LimeReport::CallbackInfo, QVariant &>::of(&GroupInvestigationList::slotGetCallbackChildData_items));

    connect(callbackDatasource, QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&LimeReport::ICallbackDatasource::changePos),
            this, QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&GroupInvestigationList::slotChangeChildPos_items));

#elif defined(Q_OS_WIN)

    // list_owner
    LimeReport::ICallbackDatasource *callbackDatasource = m_report->dataManager()->createCallbackDatasource("list_owner");
    connect(callbackDatasource, SIGNAL(getCallbackData()), this, SLOT(slotGetCallbackChildData()));
    connect(callbackDatasource, SIGNAL(changePos()), this, SLOT(slotChangeChildPos()));

    // list_items
    callbackDatasource = m_report->dataManager()->createCallbackDatasource("list_items");
    connect(callbackDatasource, SIGNAL(getCallbackData()), this, SLOT(slotGetCallbackChildData_items()));
    connect(callbackDatasource, SIGNAL(changePos()), this, SLOT(slotChangeChildPos_items()));

#endif

    m_report->dataManager()->clearUserVariables();
    m_report->setShowProgressDialog(true);
    m_report->setPreviewWindowTitle(tr("Arbore investigațiilor"));
    m_report->loadFromFile(dir.toNativeSeparators(globals::pathTemplatesDocs + "/Tree_investigations.lrxml"));
    m_report->previewReport();

    delete m_owner;
    delete m_investigations;
    m_report->deleteLater();

    this->show();
}

void GroupInvestigationList::slotGetCallbackChildData(LimeReport::CallbackInfo info, QVariant &data)
{
    if (! m_owner)
        return;
    prepareData(m_owner, info, data);
}

void GroupInvestigationList::slotChangeChildPos(const LimeReport::CallbackInfo::ChangePosType &type, bool &result)
{
    QSqlQuery *ds = m_owner;
    if (!ds)
        return;
    if (type == LimeReport::CallbackInfo::First)
        result = ds->first();
    else
        result = ds->next();

    if (result){
        m_investigations->bindValue(0, m_owner->value(m_owner->record().indexOf("owner")));
        if (! m_investigations->exec()) {
            qCritical() << "Error: Unable to execute investigations query (slotChangePos):" << m_investigations->lastError().text();
            delete m_report;
            return;
        }
    }
}

void GroupInvestigationList::slotGetCallbackChildData_items(LimeReport::CallbackInfo info, QVariant &data)
{
    if (! m_investigations)
        return;
    prepareData(m_investigations, info, data);
}

void GroupInvestigationList::slotChangeChildPos_items(const LimeReport::CallbackInfo::ChangePosType &type, bool &result)
{
    QSqlQuery *ds = m_investigations;
    if (!ds)
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
