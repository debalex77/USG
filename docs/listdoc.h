#ifndef LISTDOC_H
#define LISTDOC_H

#include <QDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QToolButton>
#include <QMenu>

#include "data/database.h"
#include <data/appsettings.h>
#include <data/globals.h>
#include "data/popup.h"

#include "models/basesqlquerymodel.h"
#include "models/basesortfilterproxymodel.h"
#include "docs/docpricing.h"

#include <catalogs/customperiod.h>

#include <common/reportsettingsmanager.h>

namespace Ui {
class ListDoc;
}

class ListDoc : public QDialog
{
    Q_OBJECT
    // pu filtru
    Q_PROPERTY(bool ItFilter READ getItFilter WRITE setItFilter NOTIFY ItFilterChanged)
    Q_PROPERTY(bool ItFilterPeriod READ getItFilterPeriod WRITE setItFilterPeriod NOTIFY ItFilterPeriodChanged)
    Q_PROPERTY(int IdOrganization READ getIdOrganization WRITE setIdOrganization NOTIFY IdOrganizationChanged)
    Q_PROPERTY(int IdContract READ getIdContract WRITE setIdContract NOTIFY IdContractChanged)
    Q_PROPERTY(int IdUser READ getIdUser WRITE setIdUser NOTIFY IdUserChanged)
    Q_PROPERTY(QString NumberDoc READ getNumberDoc WRITE setNumberDoc NOTIFY NumberDocChanged)
    // pu filtru
    Q_PROPERTY(bool ModeSelection READ getModeSelection WRITE setModeSelection NOTIFY ModeSelectionChanged)

public:
    explicit ListDoc(QWidget *parent = nullptr);
    ~ListDoc();

    void setItFilter(bool ItFilter)
    {m_itFilter = ItFilter; emit ItFilterChanged();}
    bool getItFilter() const
    {return m_itFilter;}

    void setItFilterPeriod(bool ItFilterPeriod)
    {m_itFilterPeriod = ItFilterPeriod; emit ItFilterPeriodChanged();}
    bool getItFilterPeriod() const
    {return m_itFilterPeriod;}

    void setIdOrganization(int IdOrganization)
    {m_idOrganization = IdOrganization; emit IdOrganizationChanged();}
    int getIdOrganization() const
    {return m_idOrganization;}

    void setIdContract(int IdContract)
    {m_idContract = IdContract; emit IdContractChanged();}
    int getIdContract() const
    {return m_idContract;}

    void setIdUser(int IdUser)
    {m_idUser = IdUser; emit IdUserChanged();}
    int getIdUser() const
    {return m_idUser;}

    void setNumberDoc(QString NumberDoc)
    {m_numberDoc = NumberDoc; emit NumberDocChanged();}
    QString getNumberDoc() const
    {return m_numberDoc;}

    void setModeSelection(bool ModeSelection)
    {m_modeSelection = ModeSelection; emit ModeSelectionChanged();}
    bool getModeSelection()
    {return m_modeSelection;}

    void SetIdDocSelection(const int id)
    {m_idSelectionDoc = id; emit SelectIdDoc();}
    int GetIdDicSelection() const
    {return m_idSelectionDoc;}

     void updateTableView();

signals:
    void ItFilterChanged();
    void ItFilterPeriodChanged();
    void IdOrganizationChanged();
    void IdContractChanged();
    void IdUserChanged();
    void NumberDocChanged();
    void ModeSelectionChanged();
    void SelectIdDoc();

private:
    void initBtnToolBar();
    void initBtnFilter();
    void updateTextPeriod();

    void loadSizeSectionPeriodTable(bool only_period = false);
    void saveSizeSectionTable();
    int sizeSectionDefault(const int numberSection);

    QString getStrQuery();
//    void updateTableView();
    void updateHeaderTable();

    enum sections
    {
        section_id             = 0,
        section_deletionMark   = 1,
        section_numberDoc      = 2,
        section_dateDoc        = 3,
        section_idOrganization = 4,
        section_Organization   = 5,
        section_Contract       = 6,
        section_author         = 7,
        section_comment        = 8
    };

private slots:
    void onClickChoicePeriod();
    void onChangeCustomPeriod();

    void selectDocPricing();
    void createNewDocPricing();
    void editDocPricing();
    void deletionMarkDocPricing();
    void onDoubleClickedTable(const QModelIndex &index);

    void addFilterForListDoc();
    void setFilterByIdOrganization();
    void removeFilterByIdOrganization();

    bool controlRequiredObjects();
    void onPrintDoc();

    void openFilterPeriod();

    void onStartDateTimeChanged();
    void onEndDateTimeChanged();

    void indexChangedFilterComboOrganization(const int arg1);
    void indexChangedFilterComboContract(const int arg1);
    void indexChangedFilterComboUser(const int arg1);

    void applyFilter();
    void clearItemsFilter();
    void closeFilterComplex();

    void slot_ItFilterChanged();
    void slot_ItFilterPeriodChanged();
    void slot_IdOrganizationChanged();
    void slot_IdContractChanged();
    void slot_IdUserChanged();
    void slot_ModeSelectionChanged();

    void slotContextMenuRequested(QPoint pos);

    void updatePostDocs();

private:
    Ui::ListDoc *ui;
    ReportSettingsManager settings;

    DataBase *db;
    PopUp    *popUp;
    QMenu    *menu;

    static const int sz_id              = 5;    // size section default
    static const int sz_deletionMark    = 5;
    static const int sz_numberDoc       = 100;
    static const int sz_dateDoc         = 165;
    static const int sz_id_organization = 5;
    static const int sz_organization    = 250;
    static const int sz_contract        = 300;
    static const int sz_author          = 250;

    static const int section_zero = 0;
    const QString type_doc = "DocPricing";

    BaseSqlQueryModel *model;              // tableView
    BaseSqlQueryModel *modelOrganizations; // filtru organization
    BaseSqlQueryModel *modelContracts;     // filtru contract
    BaseSqlQueryModel *modelUsers;         // filtru autor
    BaseSortFilterProxyModel *proxy;       // model pu sortare

    DocPricing *docPricing;
    CustomPeriod *customPeriod;

    bool m_itFilter       = false;       // proprietatile pu filtru
    bool m_itFilterPeriod = false;
    bool m_modeSelection  = false;
    int m_idOrganization  = -1;
    int m_idContract      = -1;
    int m_idUser          = -1;
    int m_currentRow      = -1;
    int m_idSelectionDoc  = -1;

    QString m_numberDoc;
    QMap<QString, QVariant> itemsFilter;
    QMap<QString, QDateTime> itemsFilterPeriod;

protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void keyReleaseEvent(QKeyEvent *event); // pu identificare butoanelor:
                                            // 'Key_Up', 'Key_Down', 'Key_Home', 'Key_End', 'Key_Prt'
                                            // se foloseste functia - keyReleaseEvent(), analogica keyPressEvent()
};

#endif // LISTDOC_H
