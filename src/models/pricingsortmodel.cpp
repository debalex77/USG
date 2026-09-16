#include "pricingsortmodel.h"

PricingSortModel::PricingSortModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{
    setDynamicSortFilter(true);
}


bool PricingSortModel::lessThan(const QModelIndex &source_left,
                                const QModelIndex &source_right) const
{
    QVariant l = sourceModel()->data(source_left, sortRole());
    QVariant r = sourceModel()->data(source_right, sortRole());

    switch (l.typeId()) {
    case QMetaType::Int:
        return l.toInt() < r.toInt();

    case QMetaType::QString:
        return QString::localeAwareCompare(l.toString(), r.toString()) < 0;

    case QMetaType::QDateTime:
        return l.toDateTime() < r.toDateTime();

    case QMetaType::Bool:
        return l.toBool() < r.toBool();

    default:
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    }
}
