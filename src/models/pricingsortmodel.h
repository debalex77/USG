#ifndef PRICINGSORTMODEL_H
#define PRICINGSORTMODEL_H

#include <QSortFilterProxyModel>
#include <QDateTime>

class PricingSortModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit PricingSortModel(QObject *parent = nullptr);

protected:
    bool lessThan(const QModelIndex &source_left,
                  const QModelIndex &source_right) const override;
};

#endif // PRICINGSORTMODEL_H
