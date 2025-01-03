#ifndef PAGINATEDSQLMODEL_H
#define PAGINATEDSQLMODEL_H

#include <QSqlQuery>
#include <QDateTime>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQueryModel>
#include <QStandardItem>
#include <QThreadPool>
#include <QProgressBar>
#include <QProgressDialog>

#include <data/database.h>
#include <data/globals.h>
#include <data/enums.h>
#include <models/loaddatatask.h>

class PaginatedSqlModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    explicit PaginatedSqlModel(QObject *parent = nullptr);
    ~PaginatedSqlModel();

    void setPagination(int limit, int offset);
    void resetPagination();
    void setStrQuery(const QString strQry);
    void fetchMoreData();
    void fetchMoreDataFull();
    void updateVisibleData(int scrollPosition, int rowHeight);
    void loadDataAsync();
    void onDataLoaded(const QVector<QVector<QVariant>> &newRows);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

signals:
    void dataUpdated();
    void updateProgress(const int num_records, const int value);
    void finishedProgress(const QString txt);

private slots:
    void updateColumnCount(int columnCount);

private:
    void updateModel();

private:
    int limit = 50;
    int offset = 0;
    int columnCountValue;
    QString m_strQry = nullptr;
    QVector<QVector<QVariant>> rows;
    DataBase *db;

    QProgressDialog *p_dialog;
    QProgressBar p_bar;
};

#endif // PAGINATEDSQLMODEL_H
