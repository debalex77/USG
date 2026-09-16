#ifndef REPORTJOURNALLOADER_H
#define REPORTJOURNALLOADER_H

#include <data/database.h>
#include <common/appmetatypes.h>
#include <common/table_sections.h>

class ReportJournalLoader
{
public:
    struct BatchResult {
        QVector<ReportJournal::Item> items;
        bool hasMore = false;
        QString error;
    };

    explicit ReportJournalLoader(DataBase &db);
    void setFilter(const JournalFilter &filter);
    BatchResult loadFirstBatch(int limit);
    BatchResult loadNextBatch(int limit, const QDateTime &lastDateDoc, qint64 lastId);

private:
    QString buildSql(bool firstBatch) const;
    void bindCommonParams(QSqlQuery &query) const;
    BatchResult execQuery(QSqlQuery &query, int limit) const;

    DataBase &m_db;
    JournalFilter m_filter;
};

#endif
