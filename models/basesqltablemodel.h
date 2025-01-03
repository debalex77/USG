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

    void setTableSource(const QString tableSource); // pu clasa 'DocOrderEcho'
                                                    // sunt 2 tabele: tableSource + tableOrder

public:
    void setMainFlag(Qt::ItemFlags flag);  // pu clasa 'CatForSqlTableModel'
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual void setTable(const QString &tableName) override;
    virtual void setEditStrategy(EditStrategy strategy) override;
    virtual void setSort(int column, Qt::SortOrder order) override;

private:
    Qt::ItemFlags flagsFromPricing(const QModelIndex &index) const;
    QVariant dataFromCatForTableModel(const QModelIndex &index, int role, QVariant &value) const;
    QVariant dataFromDocPricing(const QModelIndex &index, int role) const;
    QVariant dataFromDocOrderEcho(const QModelIndex &index, int role) const;

private:
    Qt::ItemFlags m_flag = Qt::NoItemFlags;
    QString m_tableSource; // pu clasa 'DocOrderEcho' - vezi setTableSource(const QString tableSource)

};

#endif // BASESQLTABLEMODEL_H
