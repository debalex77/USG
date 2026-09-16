#ifndef REPORTJOURNALMODEL_H
#define REPORTJOURNALMODEL_H

#include <QAbstractTableModel>
#include <models/reportjournalloader.h>

class ReportJournalModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Role { SortRole = Qt::UserRole + 1 };
    explicit ReportJournalModel(DataBase &db, QObject *parent = nullptr);
    void setFilter(const JournalFilter &filter);
    const JournalFilter &filter() const;
    void setBatchSize(int value);
    const ReportJournal::Item &itemAt(int row) const;
    QString lastError() const;
    void reload();
    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void fetchMore(const QModelIndex &parent = {}) override;
    bool canFetchMore(const QModelIndex &parent = {}) const override;

private:
    QVector<ReportJournal::Item> m_items;
    ReportJournalLoader m_loader;
    JournalFilter m_filter;
    int m_batchSize = 100;
    bool m_hasMore = true;
    bool m_loading = false;
    QString m_lastError;
};
#endif
