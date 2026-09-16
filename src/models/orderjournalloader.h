#ifndef ORDERJOURNALLOADER_H
#define ORDERJOURNALLOADER_H

#include <data/database.h>
#include <common/appmetatypes.h>
#include <common/table_sections.h>

class OrderJournalLoader
{
public:
    struct BatchResult {
        QVector<OrderJournal::Item> items;
        bool hasMore = false;
    };

    explicit OrderJournalLoader(DataBase &db);

    void setFilter(const JournalFilter &filter);

    BatchResult loadFirstBatch(int limit);
    BatchResult loadNextBatch(int limit,
                              const QDateTime &lastDateDoc,
                              qint64 lastId);

private:
    QString buildSql(bool firstBatch) const;
    void bindCommonParams(QSqlQuery &qry) const;
    BatchResult execQuery(QSqlQuery &qry, int limit);

private:
    DataBase &m_db;
    JournalFilter m_filter;
};

#endif // ORDERJOURNALLOADER_H
