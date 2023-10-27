#ifndef BASECOMBOMODEL_H
#define BASECOMBOMODEL_H

#include <QSqlQueryModel>

class BaseComboModel : public QSqlQueryModel
{
    Q_OBJECT

    QVariant dataFromParent(QModelIndex index, int column) const;

public:
    explicit BaseComboModel( const QString &columns, const QString &queryTail, QObject *parent = 0 );
    virtual QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex &parent) const;
};

#endif // BASECOMBOMODEL_H
