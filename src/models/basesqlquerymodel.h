#ifndef BASESQLQUERYMODEL_H
#define BASESQLQUERYMODEL_H

#include <QObject>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <data/enums.h>

class BaseSqlQueryModel : public QSqlQueryModel
{
    Q_OBJECT
    Q_PROPERTY(ModelParent modelParent READ getModelParent WRITE setModelParent NOTIFY modelParentChanged)      // p-u determinarea QComboBox, QTableView etc.

    QVariant dataFromParent(QModelIndex index, int column) const;

public:
    explicit BaseSqlQueryModel(QString &strQuery, QObject *parent = nullptr);

    enum ModelParent
    {
        UserSettings,
        GeneralListForm
    };
    Q_ENUM(ModelParent)

    void setModelParent(ModelParent modelParent)
    {m_modelParent = modelParent;}
    ModelParent getModelParent() const
    {return m_modelParent;}

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const override;
    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

signals:
    void modelParentChanged();

private:
    QVariant dataFromComboBox(const QModelIndex &item, int role = Qt::DisplayRole) const;
    QVariant dataFromUserSettings(const QModelIndex &item, int role = Qt::DisplayRole) const;
    QVariant dataFromGeneralListForm(const QModelIndex &item, int role = Qt::DisplayRole) const;
    QVariant dataFromCatOrganizations(const QModelIndex &item, int role = Qt::DisplayRole) const;

private:
    ModelParent m_modelParent = GeneralListForm;
};

#endif // BASESQLQUERYMODEL_H
