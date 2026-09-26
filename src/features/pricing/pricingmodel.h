#ifndef PRICINGMODEL_H
#define PRICINGMODEL_H

#include <QAbstractTableModel>
#include <QBrush>
#include <QFont>
#include <QIcon>

#include <common/table_sections.h>

class PricingModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit PricingModel(QObject *parent = nullptr);

    enum Role {
        SortRole = Qt::UserRole + 1
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation orientation = Qt::Horizontal,
                        int role = Qt::DisplayRole) const override;

    void setItems(const QVector<PricingsJournal::Item> &items);
    void clearItems();

    const PricingsJournal::Item &itemAt(int row) const;

private:
    QVector<PricingsJournal::Item> m_items;
};

#endif // PRICINGMODEL_H
