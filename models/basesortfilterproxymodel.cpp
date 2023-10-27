#include "basesortfilterproxymodel.h"


BaseSortFilterProxyModel::BaseSortFilterProxyModel(QObject *parent)
    :QSortFilterProxyModel(parent)
{    
}

QVariant BaseSortFilterProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    //redefinim sortarea numeratiei randurilor tabelei
    if (role == Qt::DisplayRole){
        if (orientation == Qt::Vertical || section == 0){
            return section + 1;
        }
    }
    return QSortFilterProxyModel::headerData(section, orientation, role);
}

bool BaseSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{

    // capatam datele din model
    QVariant leftData = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);

    //*******************************************************************************
    // SORTAREA - numberDoc (QString)
    //*******************************************************************************

    if (left.column() == column_numberDoc && m_listFormType == ListFormType::ListDocuments){
        QVariant _dataLeft = leftData.toInt();
        QVariant _dataRight = rightData.toInt();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        if ((_dataLeft.isValid() && _dataLeft.type() == QVariant::Int
             && (_dataRight.isValid() && _dataRight.type() == QVariant::Int))){
            return _dataLeft.toInt() < _dataRight.toInt();
        }
#else
        if ((_dataLeft.isValid() && _dataLeft.typeId() == QVariant::Int
             && (_dataRight.isValid() && _dataRight.typeId() == QVariant::Int))){
            return _dataLeft.toInt() < _dataRight.toInt();
        }
#endif
    }

    //*******************************************************************************
    // SORTAREA - QDateTime
    //*******************************************************************************

    if (left.column() == column_dateDoc && m_listFormType == ListFormType::ListDocuments){
        // convertam datele in QDateTime si introducem in QVariant
        QVariant _dateTimeLeft  = (QDateTime::fromString(leftData.toString(), "dd.MM.yyyy hh:mm:ss"));
        QVariant _dateTimeRight = (QDateTime::fromString(rightData.toString(), "dd.MM.yyyy hh:mm:ss"));

        /* convertam datele in toDateTime pentru verificarea daca este valid,
         * apoi verificam daca este QVariant::DateTime (poate sa fie QVariant::Date)
         *  - rezultat - sortarea dupa QDateTime */
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        if ((_dateTimeLeft.toDateTime().isValid() && _dateTimeLeft.type() == QVariant::DateTime)
                && (_dateTimeRight.toDateTime().isValid() && _dateTimeRight.type() == QVariant::DateTime)){
            return _dateTimeLeft.toDateTime() < _dateTimeRight.toDateTime();
        }
#else
        if ((_dateTimeLeft.toDateTime().isValid() && _dateTimeLeft.typeId() == QVariant::DateTime)
            && (_dateTimeRight.toDateTime().isValid() && _dateTimeRight.typeId() == QVariant::DateTime)){
            return _dateTimeLeft.toDateTime() < _dateTimeRight.toDateTime();
        }
#endif
    }
    //*******************************************************************************
    // SORTAREA - QDate
    //*******************************************************************************

    // convertam datele in QDate si introducem in QVariant
    QVariant _dateLeft  = (QDate::fromString(leftData.toString(), "dd.MM.yyyy"));
    QVariant _dateRight = (QDate::fromString(rightData.toString(), "dd.MM.yyyy"));

    /* convertam datele in toDate pentru verificarea daca este valid,
     * apoi verificam daca este QVariant::Date (poate sa fie QVariant::DateTime)
     *  - rezultat - sortarea dupa QDate */
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    if ((_dateLeft.toDate().isValid() && _dateLeft.type() == QVariant::Date)
            && (_dateRight.toDate().isValid() && _dateRight.type() == QVariant::Date)){
        return _dateLeft.toDate() < _dateRight.toDate();
    }
#else
    if ((_dateLeft.toDate().isValid() && _dateLeft.typeId() == QVariant::Date)
        && (_dateRight.toDate().isValid() && _dateRight.typeId() == QVariant::Date)){
        return _dateLeft.toDate() < _dateRight.toDate();
    }
#endif

    //*******************************************************************************
    // SORTAREA - QString
    //*******************************************************************************

    // controlam daca
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    if (leftData.type() == QVariant::DateTime) {
#else
    if (leftData.typeId() == QVariant::DateTime) {
#endif
        return leftData.toDateTime() < rightData.toDateTime();
    } else {
        static const QRegularExpression emailPattern("[\\w\\.]*@[\\w\\.]*");

        QString leftString = leftData.toString();
        if (left.column() == 1) {
            const QRegularExpressionMatch match = emailPattern.match(leftString);
            if (match.hasMatch())
                leftString = match.captured(0);
        }
        QString rightString = rightData.toString();
        if (right.column() == 1) {
            const QRegularExpressionMatch match = emailPattern.match(rightString);
            if (match.hasMatch())
                rightString = match.captured(0);
        }

        return QString::localeAwareCompare(leftString, rightString) < 0;
    }
}
