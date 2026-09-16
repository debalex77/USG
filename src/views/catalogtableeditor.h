#ifndef CATALOGTABLEEDITOR_H
#define CATALOGTABLEEDITOR_H

#include <QDialog>
#include <QKeyEvent>
#include <QScrollBar>
#include <QDomDocument>
#include <QMessageBox>
#include <QMdiSubWindow>
#include <QStandardItemModel>
#include <LimeReport>
#include <QAbstractItemDelegate>

#include <catalogs/groupinvestigationlist.h>

#include <common/appmetatypes.h>
#include <common/reportsettingsmanager.h>
#include <common/table_sections.h>
#include <common/balloontip.h>

#include <data/database.h>
#include <common/globals.h>
#include <data/popup.h>

#include <delegates/checkboxdelegate.h>
#include <delegates/combodelegate.h>

#include <customs/toolbarcustom.h>

#include <models/baseabstractmodel.h>

namespace Ui {
class CatalogTableEditor;
}

class CatalogTableEditor : public QDialog
{
    Q_OBJECT

public:
    explicit CatalogTableEditor(DataBase &db,
                                CatalogType::FormType formType,
                                CatalogType::Type catalogType,
                                QWidget *parent = nullptr);
    ~CatalogTableEditor();

    void setFilterQuery(const QString nameSystem);

signals:
    void dataSelected(const QVariantMap &data);

private slots:
    void onAdd();
    void onEdit();
    void onDelete();

    void onOpenTreeInvestigations();
    void onOpenUltrasoundTariffClassifier();

    void onDataChangedItemModel(const QModelIndex &topLeft,
                                const QModelIndex &bottomRight,
                                const QVector<int> &);
    void onValidationFailed(const QPersistentModelIndex &index,
                            const QString &errorText);
    void onCloseEditor(QWidget *editor,
                       QAbstractItemDelegate::EndEditHint hint);

    /** functia pu selectare a randului -> vezi 'pricing'
     *  adaugarea investigatiei in tabel cu emiterea signalului
     *  dataSelected(const QVariantMap &data);  */
    void onRowSelected(const QModelIndex &index);

private:
    void loadFilterBySettings();
    void loadSizeSection();
    void saveSizeSection();

    void initToolBar();

    QString getTextQueryByCatalogType() const;

    void registerColumnTableByCatalogType();
    void setDelegatesFromTable();

    void initTableView();
    void updateTableView();

    QString getNameTableByCatalogType();

    QVariantMap getDataByCatalogType(bool insert = true);

    bool saveCurrentRowToDatabase(int row);

    void reject(); // pu inchiderea subferestrei MDI la apasarea ESC

private:
    Ui::CatalogTableEditor *ui;
    ReportSettingsManager m_settings;
    CatalogViewFilter m_filter;

    CatalogType::FormType m_formType;
    CatalogType::Type m_catalogType;

    const QString m_class = "CatalogTableEditor";

    QString m_filterQuery; // pu clasa 'ReportPageOrgansInternal' sau 'ReportPageUrinarySystem' etc.

    DataBase &m_db;
    PopUp    *popUp;

    QString toolButtonStyleForIcon;
    QString toolButtonStyleForText;

    ToolBarCustom *toolBar;

    BaseAbstractModel *model;
    bool editableColumn; // daca m_formType == Select -> false

    QSet<int> m_modifiedRows;

protected:
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void changeEvent(QEvent *event);
    void keyReleaseEvent(QKeyEvent *event); // 'Key_Up', 'Key_Down' etc
};

#endif // CATALOGTABLEEDITOR_H
