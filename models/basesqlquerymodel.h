#ifndef BASESQLQUERYMODEL_H
#define BASESQLQUERYMODEL_H

#include <QObject>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>

class BaseSqlQueryModel : public QSqlQueryModel
{
    Q_OBJECT
    Q_PROPERTY(ModelParent modelParent READ getModelParent WRITE setModelParent NOTIFY modelParentChanged)      // p-u determinarea QComboBox, QTableView etc.
    Q_PROPERTY(TypeCatalogs typeCatalogs READ getTypeCatalogs WRITE setTypeCatalogs NOTIFY typeCatalogsChanged)   //

    QVariant dataFromParent(QModelIndex index, int column) const;

public:
    explicit BaseSqlQueryModel(QString &strQuery, QObject *parent = nullptr);

    enum ModelParent
    {
        UserSettings,
        GeneralListForm,
        ListOrderReport,
        ViewTableOrder,
        ViewTableReport
    };
    Q_ENUM(ModelParent)

    enum TypeCatalogs
    {
        Doctors,
        Nurses,
        Pacients,
        Users,
        Organizations
    };
    Q_ENUM(TypeCatalogs)

    void setModelParent(ModelParent modelParent)
    {m_modelParent = modelParent;}
    ModelParent getModelParent() const
    {return m_modelParent;}

    void setTypeCatalogs(TypeCatalogs typeCatalogs)
    {m_typeCatalogs = typeCatalogs;}
    TypeCatalogs getTypeCatalogs() const
    {return m_typeCatalogs;}

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const override;
    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    void setIdContract(const int _id);

signals:
    void modelParentChanged();
    void typeCatalogsChanged();

private:
    QVariant dataFromComboBox(const QModelIndex &item, int role = Qt::DisplayRole) const;
    QVariant dataFromUserSettings(const QModelIndex &item, int role = Qt::DisplayRole) const;
    QVariant dataFromGeneralListForm(const QModelIndex &item, int role = Qt::DisplayRole) const;
    QVariant dataFromCatOrganizations(const QModelIndex &item, int role = Qt::DisplayRole) const;
    QVariant dataFromListDoc(const QModelIndex &item, int role = Qt::DisplayRole) const;
    QVariant dataFromDocPricing(const QModelIndex &item, int role = Qt::DisplayRole) const;

    enum orderSections
    {
        orderSection_id             = 0,
        orderSection_deletionMark   = 1,
        orderSection_attachedImages = 2,
        orderSection_cardPayment    = 3,
        orderSection_numberDoc      = 4,
        orderSection_dateDoc        = 5,
        orderSection_idOrganization = 6,
        orderSection_Organization   = 7,
        orderSection_idContract     = 8,
        orderSection_Contract       = 9,
        orderSection_idPacient      = 10,
        orderSection_searchPacient  = 11,
        orderSection_pacient        = 12,
        orderSection_IDNP           = 13,
        orderSection_doctor         = 14,
        orderSection_idUser         = 15,
        orderSection_user           = 16,
        orderSection_sum            = 17,
        orderSection_comment        = 18
    };
    QVariant dataFromDocOrderEcho(const QModelIndex &item, int role = Qt::DisplayRole) const;
    QVariant dataFrom_view_table_order(const QModelIndex &item, int role = Qt::DisplayRole) const;

private:
    ModelParent m_modelParent;
    TypeCatalogs m_typeCatalogs;
    int id_contract = -1;
};

#endif // BASESQLQUERYMODEL_H
