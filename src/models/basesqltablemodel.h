#ifndef BASESQLTABLEMODEL_H
#define BASESQLTABLEMODEL_H

#include <QSqlTableModel>
#include <QObject>
#include <QSize>
#include <QColor>
#include <QIcon>
#include <QFont>
#include <QBrush>
#include <data/enums.h>

class BaseSqlTableModel : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit BaseSqlTableModel(QObject *parent = nullptr);

public:
    void setMainFlag(Qt::ItemFlags flag);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual void setTable(const QString &tableName) override;
    virtual void setEditStrategy(EditStrategy strategy) override;
    virtual void setSort(int column, Qt::SortOrder order) override;

private:
    Qt::ItemFlags flagsFromPricing(const QModelIndex &index) const;
    QVariant dataFromCatForTableModel(const QModelIndex &index, int role, QVariant &value) const;

private:
    Qt::ItemFlags m_flag = Qt::NoItemFlags;
};

#endif // BASESQLTABLEMODEL_H
