#ifndef CATALOGSMODEL_H
#define CATALOGSMODEL_H

#include <QAbstractTableModel>

#include <data/database.h>
#include <common/globals.h>

#include <common/table_sections.h>
#include <common/appmetatypes.h>

#include <models/catalogsloader.h>

class CatalogsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit CatalogsModel(DataBase &db, CatalogType::Type typeCatalog, QObject *parent = nullptr);

    enum Role {
        SortRole = Qt::UserRole + 1
    };

    void setBatchSize(int value);
    const CatalogsCommon &itemAt(int row) const;
    void reload();
    void setSort(int column, Qt::SortOrder order);

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
    void appendItems(const QVector<CatalogsCommon> &items);

private:
    void loadInitial();
    void loadMore();

private:
    QVector<CatalogsCommon> m_items;
    CatalogType::Type m_typeCatalog;
    CatalogsLoader m_loader;

    QVariant m_cursor;
    int m_batchSize = 100;
    bool m_hasMore = true;
    bool m_loading = false;
};

#endif // CATALOGSMODEL_H
