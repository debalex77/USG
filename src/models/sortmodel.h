#ifndef SORTMODEL_H
#define SORTMODEL_H

#include <QSortFilterProxyModel>
#include <QDateTime>

class SortModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit SortModel(QObject *parent = nullptr);

protected:
    bool lessThan(const QModelIndex &source_left,
                  const QModelIndex &source_right) const override;
};

#endif // SORTMODEL_H
