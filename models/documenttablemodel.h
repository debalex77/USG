#ifndef DOCUMENTTABLEMODEL_H
#define DOCUMENTTABLEMODEL_H

#include <QAbstractTableModel>
#include <QSqlQuery>

class DocumentTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit DocumentTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    void fetchMore(const QModelIndex &parent = QModelIndex()) override;
    bool canFetchMore(const QModelIndex &parent) const override;

    void setTotalRecords(int total = 0);
    int getTotalRecords();

private:
    QVector<QVector<QVariant>> rows; // Datele modelului
    int limit;
    int offset;
    int totalRecords;
};

#endif // DOCUMENTTABLEMODEL_H
