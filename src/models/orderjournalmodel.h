#ifndef ORDERJOURNALMODEL_H
#define ORDERJOURNALMODEL_H

#include <QAbstractTableModel>

#include <data/database.h>

#include <common/table_sections.h>
#include <common/appmetatypes.h>

#include <models/orderjournalloader.h>

class OrderJournalModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit OrderJournalModel(DataBase &db, QObject *parent = nullptr);

    enum Role {
        SortRole = Qt::UserRole + 1
    };

    void setFilter(const JournalFilter &filter);
    const JournalFilter &filter() const;
    void setBatchSize(int value);

    const OrderJournal::Item &itemAt(int row) const;

    void reload();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role) const override;
    void fetchMore(const QModelIndex &parent = QModelIndex()) override;
    bool canFetchMore(const QModelIndex &parent = QModelIndex()) const override;

    void clearItems();
    bool isEmpty() const;
    void appendItems(const QVector<OrderJournal::Item> &items);

private:
    void loadInitial();
    void loadMore();

private:
    QVector<OrderJournal::Item> m_items;
    OrderJournalLoader m_loader;
    JournalFilter m_filter;

    int m_batchSize = 100;
    bool m_hasMore = true;
    bool m_loading = false;
};

#endif // ORDERJOURNALMODEL_H
