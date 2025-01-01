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

    enum orderSections
    {
        orderSection_id             = 0,
        orderSection_deletionMark   = 1,
        orderSection_attachedImages = 2,
        orderSection_cardPayment    = 3,
        orderSection_numberDoc      = 4,
        orderSection_dateDoc        = 5,
        orderSection_idOrganization = 6,
        orderSection_Organization   = 7,
        orderSection_idContract     = 8,
        orderSection_Contract       = 9,
        orderSection_idPacient      = 10,
        orderSection_searchPacient  = 11,
        orderSection_pacient        = 12,
        orderSection_IDNP           = 13,
        orderSection_doctor         = 14,
        orderSection_idUser         = 15,
        orderSection_user           = 16,
        orderSection_sum            = 17,
        orderSection_comment        = 18
    };
};

#endif // PAGINATEDSQLMODEL_H
