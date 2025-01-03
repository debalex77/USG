#ifndef BASESORTFILTERPROXYMODEL_H
#define BASESORTFILTERPROXYMODEL_H

#include <QObject>
#include <QDebug>
#include <QDate>
#include <QDateTime>
#include <QSortFilterProxyModel>
#include <data/enums.h>

class BaseSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(ListFormType listFormType READ getListFormType WRITE setListFormType NOTIFY ListFormTypeChanged)
public:
    BaseSortFilterProxyModel(QObject *parent = nullptr);

    enum ListFormType{ListCatalog, ListDocuments};
    Q_ENUM(ListFormType)

    bool compareDates(const QVariant &left, const QVariant &right, const QString &format) const;
    bool compareInts(const QVariant &left, const QVariant &right) const;

    void setListFormType(ListFormType listFormType){m_listFormType = listFormType; emit ListFormTypeChanged();}
    ListFormType getListFormType() const {return m_listFormType;}

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

signals:
    void ListFormTypeChanged();

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    bool dateInRange(const QDate &date) const;

private:
    ListFormType m_listFormType;
};

#endif // BASESORTFILTERPROXYMODEL_H
