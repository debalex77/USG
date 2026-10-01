#ifndef ONLINEACCOUNTVIEW_H
#define ONLINEACCOUNTVIEW_H

#include <QDialog>
#include <QMdiSubWindow>

#include <features/catalogs/onlineaccountdialog.h>

#include <common/appmetatypes.h>
#include <common/globals.h>
#include <settings/layoutsettingsmanager.h>
#include <ui/services/tablecolumnscontroller.h>

#include <ui/widgets/toolbarcustom.h>

#include <database/database.h>
#include <app/popup.h>

#include <features/catalogs/onlineaccountmodel.h>

namespace Ui {
class OnlineAccountView;
}

class OnlineAccountView : public QDialog
{
    Q_OBJECT

public:
    explicit OnlineAccountView(DataBase &db, QWidget *parent = nullptr);
    ~OnlineAccountView();

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onUpdate();
    void onShowHideColumn();

    void onColumnsChanged();

    void onDoubleClickedTableView(const QModelIndex &index);
    void slotContextMenuRequested(const QPoint &pos);

private:
    void loadFilterJournalBySettings();
    void initToolBar();
    void initTableView();
    void updateTableView();

    void loadSizeSection();
    int lastVisibleSection() const;
    void saveSettingsJournal();

    void reject(); // pu inchiderea subferestrei MDI la apasarea ESC

private:
    Ui::OnlineAccountView *ui;
    LayoutSettingsManager m_settings;

    struct Filter
    {
        QMap<int, int> sectionSizes;
        QMap<int, bool> hiddenSections;
    };
    Filter m_filter;

    const QString m_className = "OnlineAccountView";

    DataBase &m_db;
    PopUp    *popUp;
    ToolBarCustom *toolBar;

    OnlineAccountModel *model = nullptr;

    TableColumnsController *m_columnsController = nullptr;

    QString toolButtonStyleForIcon;

protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);
    void keyReleaseEvent(QKeyEvent *event); // 'Key_Up', 'Key_Down' etc
};

#endif // ONLINEACCOUNTVIEW_H
