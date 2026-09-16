#ifndef CATALOGSLOADER_H
#define CATALOGSLOADER_H

#include <data/database.h>
#include <common/globals.h>
#include <common/appmetatypes.h>
#include <common/table_sections.h>

class CatalogsLoader
{
public:
    struct BatchResult {
        QVector<CatalogsCommon> items;
        bool hasMore = false;
        QVariant cursor;
    };

    explicit CatalogsLoader(DataBase &db,
                            CatalogType::Type typeCatalogs);

    void setSort(int column, Qt::SortOrder order);
    BatchResult loadFirstBatch(int limit);
    BatchResult loadNextBatch(int limit,
                              const QVariant &lastName,
                              qint64 lastId);

private:
    QString buildSql(bool firstBatch) const;
    BatchResult execQuery(QSqlQuery &qry, int limit);

private:
    DataBase &m_db;
    CatalogType::Type m_typeCatalog;

    QString strQry;
    int m_sortColumn = 2;
    Qt::SortOrder m_sortOrder = Qt::AscendingOrder;
};

#endif // CATALOGSLOADER_H
