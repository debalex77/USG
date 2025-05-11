#ifndef GROUPINVESTIGATIONLIST_H
#define GROUPINVESTIGATIONLIST_H

#include <QDialog>
#include <QKeyEvent>
#include <LimeReport>
#include <QSqlQueryModel>

#include <common/reportsettingsmanager.h>

#include "catalogs/groupinvestigation.h"
#include "data/database.h"
#include "models/treemodel.h"

namespace Ui {
class GroupInvestigationList;
}

class GroupInvestigationList : public QDialog
{
    Q_OBJECT

public:
    explicit GroupInvestigationList(QWidget *parent = nullptr);
    ~GroupInvestigationList();

private:
    void saveSizeWindows();
    void loadSizeWindows();

private slots:
    void onCreateNewGroup();
    void onHideNotGroupStateChanged(int state);
    void updateTable();

    void onPrint();
    void slotGetCallbackChildData(LimeReport::CallbackInfo info, QVariant &data);
    void slotChangeChildPos(const LimeReport::CallbackInfo::ChangePosType &type, bool &result);
    void slotGetCallbackChildData_items(LimeReport::CallbackInfo info, QVariant &data);
    void slotChangeChildPos_items(const LimeReport::CallbackInfo::ChangePosType &type, bool &result);
    void prepareData(QSqlQuery *qry, LimeReport::CallbackInfo info, QVariant &data);

private:
    Ui::GroupInvestigationList *ui;
    ReportSettingsManager settings;
    const QString name_class = "GroupInvestigationList";

    DataBase  *db;
    TreeModel *model_tree;
    GroupInvestigation *item_group;

    QSqlQuery *qry_group;

    QSqlQuery *m_owner;
    QSqlQuery *m_investigations;

    LimeReport::ReportEngine *m_report;

protected:
    void closeEvent(QCloseEvent *event);
};

#endif // GROUPINVESTIGATIONLIST_H
