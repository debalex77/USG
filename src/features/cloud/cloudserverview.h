#ifndef CLOUDSERVERVIEW_H
#define CLOUDSERVERVIEW_H

#include <QDialog>
#include <QMap>

#include <database/database.h>
#include <settings/layoutsettingsmanager.h>

class PopUp;
class QSqlQueryModel;
class TableColumnsController;
class ToolBarCustom;

namespace Ui {
class CloudServerView;
}

class CloudServerView final : public QDialog
{
    Q_OBJECT

public:
    explicit CloudServerView(DataBase &db, QWidget *parent = nullptr);
    ~CloudServerView() override;

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void updateTableView();
    void onShowHideColumn();
    void onColumnsChanged();
    void onDoubleClicked(const QModelIndex &index);
    void showContextMenu(const QPoint &position);

private:
    struct ViewSettings
    {
        QMap<int, int> sectionSizes;
        QMap<int, bool> hiddenSections;
    };

    enum Column
    {
        Id = 0,
        OrganizationId,
        UserId,
        Organization,
        User,
        Host,
        DatabaseName,
        Port,
        ConnectionOptions,
        DatabaseUser
    };

    void initToolBar();
    void initTableView();
    void loadSettings();
    void restoreColumns();
    void saveSettings();
    int lastVisibleSection() const;
    int selectedId() const;
    bool selectRecord(int id);
    void clearActiveCloudContextIfNeeded(int organizationId, int userId);
    void reject() override;

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    Ui::CloudServerView *ui = nullptr;
    DataBase &m_db;
    LayoutSettingsManager m_settings;
    ViewSettings m_viewSettings;

    QSqlQueryModel *m_model = nullptr;
    ToolBarCustom *m_toolBar = nullptr;
    TableColumnsController *m_columnsController = nullptr;
    PopUp *m_popUp = nullptr;

    const QString m_settingsKey = QStringLiteral("CloudServerView");
};

#endif // CLOUDSERVERVIEW_H
