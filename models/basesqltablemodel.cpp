#include "basesqltablemodel.h"
#include "data/globals.h"
#include <QDebug>
BaseSqlTableModel::BaseSqlTableModel(QObject *parent) : QSqlTableModel(parent)
{
}

void BaseSqlTableModel::setTableSource(const QString tableSource)
{
    m_tableSource = tableSource;  // pu clasa 'DocOrderEcho' = sunt 2 tabele: tableSource + tableOrder
}

void BaseSqlTableModel::setMainFlag(Qt::ItemFlags flag)
{
    m_flag = flag;  // pu clasa 'CatForSqlTableModel'
}

Qt::ItemFlags BaseSqlTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QSqlTableModel::flags(index);

    QString strData = QSqlTableModel::data(index, Qt::DisplayRole).toString();     // daca initial flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled
                                                                                   // nu este posibila completarea programata:
    if (parent()->inherits("DocPricing")){                                         // Procedeul:
        if ((index.column() >= Enums::PRICING_COLUMN::PRICING_COLUMN_ID) &&                                  // 1 - verificam daca sunt date
            (index.column() <= Enums::PRICING_COLUMN::PRICING_COLUMN_ID_PRICINGS)) {                          // 2 - setam flags
            flags |= Qt::ItemIsEditable;
        } else if (index.column() == Enums::PRICING_COLUMN::PRICING_COLUMN_PRICE) {
            flags |= Qt::ItemIsEditable;
        } else {
            if (!strData.isEmpty())  // verificam daca sunt date (daca date nu sunt nu setam 'flags')
                flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        }

    } else if (parent()->inherits("DocOrderEcho")){
        if (m_tableSource == "tableSource"){
            flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        } else {
            if ((index.column() >= Enums::PRICING_COLUMN::PRICING_COLUMN_ID) &&                                   // 1 - verificam daca sunt date
                    (index.column() <= Enums::PRICING_COLUMN::PRICING_COLUMN_ID_PRICINGS)){                        // 2 - setam flags
                flags |= Qt::ItemIsEditable;
            } else if (index.column() == Enums::PRICING_COLUMN::PRICING_COLUMN_PRICE) {
                flags |= Qt::ItemIsEditable;
            } else {
                if (!strData.isEmpty())  // verificam daca sunt date (daca date nu sunt nu setam 'flags')
                    flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
            }
        }

    } else if (parent()->inherits("ListDocWebOrder")){
        flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    } else {
        if (parent()->inherits("CatForSqlTableModel") && index.column() == 3 && m_flag != Qt::NoItemFlags) // pu clasa 'CatForSqlTableModel'
            flags = m_flag;                                                                                // flags |= Qt::ItemIsEditable;
        else if (parent()->inherits("CatForSqlTableModel") && index.column() == 4) // column 'use'
            flags = Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
        else
            flags |= Qt::ItemIsEditable;
    }

    return flags;
}

QVariant BaseSqlTableModel::data(const QModelIndex &index, int role) const
{
    QVariant value = QSqlTableModel::data(index, role);

    if (parent()->inherits("CatForSqlTableModel"))
        return dataFromCatForTableModel(index, role, value);
    else if(parent()->inherits("DocPricing"))
        return dataFromDocPricing(index, role);
    else if(parent()->inherits("DocOrderEcho") || parent()->inherits("ListDocWebOrder"))
        return dataFromDocOrderEcho(index, role);
    return value;
}

void BaseSqlTableModel::setTable(const QString &tableName)
{
    QSqlTableModel::setTable(tableName);
}

void BaseSqlTableModel::setEditStrategy(EditStrategy strategy)
{
    QSqlTableModel::setEditStrategy(strategy);
}

void BaseSqlTableModel::setSort(int column, Qt::SortOrder order)
{
    QSqlTableModel::setSort(column, order);
}

Qt::ItemFlags BaseSqlTableModel::flagsFromPricing(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QSqlTableModel::flags(index);

    if (index.column() == 4 || index.column() == 5)
        flags |= Qt::ItemIsEnabled;
    else if (index.column() == 6)
        flags |= Qt::ItemIsEditable;

    return flags;
}

QVariant BaseSqlTableModel::dataFromCatForTableModel(const QModelIndex &index, int role, QVariant &value) const
{
    switch (role) {
    case Qt::DisplayRole: // datele prezentarii

        if (index.column() == 1)
            return "";

        return value;

    case Qt::EditRole:    // datele redactarii

        return value;

    case Qt::TextAlignmentRole: // alinierea

        return int(Qt::AlignLeft | Qt::AlignVCenter);

    case Qt::DecorationRole:

        if (index.column() == 1 && QSqlTableModel::data(index, Qt::DisplayRole).toInt() == 0)
            return QIcon(":img/element_x32.png");
        else if (index.column() == 1 && QSqlTableModel::data(index, Qt::DisplayRole).toInt() == 1)
            return QIcon(":img/element_delete_x32.png");
        else
            return value;

    case Qt::FontRole:

        if (QSqlTableModel::data(QSqlTableModel::index(index.row(), 1), Qt::DisplayRole).toInt() == 1){
            QFont font = QSqlTableModel::data(index, Qt::FontRole).value<QFont>();  // traversarea textului
            font.setItalic(true);
            font.setStrikeOut(true);
            return font;
        } else {
#if defined(Q_OS_LINUX)
            return value;
#elif defined(Q_OS_WIN)
            QFont font;
            font.setPointSize(9);
            return font;
#endif
        }

    case Qt::BackgroundRole:  // culoarea fondalului

        if (QSqlTableModel::data(QSqlTableModel::index(index.row(), 1), Qt::DisplayRole).toInt() == 1) // DeletionMark - marked
            return QBrush(QColor(255,230,255));

        // if (QSqlTableModel::data(QSqlTableModel::index(index.row(), 4), Qt::DisplayRole).toInt() == 0) // column 'use'
        //     return QBrush(QColor(224,224,224));

        return value;

    default:
        return value;
    }

    return value;
}

QVariant BaseSqlTableModel::dataFromDocPricing(const QModelIndex &index, int role) const
{
    QVariant value = QSqlTableModel::data(index, role);

#if defined(Q_OS_WIN)
    QFont font;
    font.setPointSize(9);
#endif

    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == Enums::PRICING_COLUMN::PRICING_COLUMN_PRICE){
            double _num = value.toDouble();
            return QVariant(QString("%1").arg(_num, 0, 'f', 2));
        }
        return value;
    case Qt::EditRole:
        return value;
    case Qt::FontRole:
#if defined(Q_OS_LINUX)
        return value;
#elif defined(Q_OS_WIN)
        return font;
#endif
    case Qt::BackgroundRole:  // culoarea fondalului
        if (QSqlTableModel::data(QSqlTableModel::index(index.row(), Enums::PRICING_COLUMN::PRICING_COLUMN_PRICE), Qt::DisplayRole).toInt() == 0) {
            if (globals().isSystemThemeDark)
                return QBrush(QColor(90, 100, 90));
            else
                return QBrush(QColor(191,198,188));
        } else {                                      // toate sectiile in afara
            if (globals().isSystemThemeDark)
                return QBrush(QColor(52, 112, 93));   // de sectia nr.5 - 'pretul'
            else
                return QBrush(QColor(217,255,210));
        }
        return value;
    case Qt::TextAlignmentRole:
        if (index.column() == Enums::PRICING_COLUMN::PRICING_COLUMN_COD) // sectia 'Cod'
            return int(Qt::AlignHCenter | Qt::AlignVCenter);
        else if (index.column() == Enums::PRICING_COLUMN::PRICING_COLUMN_PRICE) // sectia 'Pret'
            return int(Qt::AlignHCenter | Qt::AlignVCenter);
        return value;
    default:
        return value;
    }
}

QVariant BaseSqlTableModel::dataFromDocOrderEcho(const QModelIndex &index, int role) const
{
    QVariant value = QSqlTableModel::data(index, role);

#if defined(Q_OS_WIN)
    QFont font;
    font.setPointSize(9);
#endif

    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == Enums::PRICING_COLUMN::PRICING_COLUMN_PRICE){
            double _num = value.toDouble();
            return QVariant(QString("%1").arg(_num, 0, 'f', 2));
        }
        return value;
    case Qt::EditRole:
        return value;
    case Qt::FontRole:
#if defined(Q_OS_LINUX)
        return value;
#elif defined(Q_OS_WIN)
        return font;
#endif
    case Qt::BackgroundRole:  // culoarea fondalului
        if (m_tableSource == "tableSource"){
            if (QSqlTableModel::data(QSqlTableModel::index(index.row(), Enums::PRICING_COLUMN::PRICING_COLUMN_PRICE), Qt::DisplayRole).toInt() == 0) {
                return QBrush(QColor(191,198,188));
            } else {
                if (globals().isSystemThemeDark)
                    return QBrush(QColor(52, 112, 93));   // de sectia nr.5 - 'pretul'
                else
                    return QBrush(QColor(214,235,206));
            }
        }
        if (index.column() != Enums::PRICING_COLUMN::PRICING_COLUMN_PRICE) { // toate sectiile in afara
            if (globals().isSystemThemeDark)
                return QBrush(QColor(52, 112, 93));   // de sectia nr.5 - 'pretul'
            else
                return QBrush(QColor(217,255,210));
        }
        return value;
    case Qt::ForegroundRole:
        if (m_tableSource == "tableSource"){
            if (QSqlTableModel::data(QSqlTableModel::index(index.row(), Enums::PRICING_COLUMN::PRICING_COLUMN_PRICE), Qt::DisplayRole).toInt() == 0)
                return QBrush(QColor(88,94,86));
        }
        return value;
    case Qt::TextAlignmentRole:
        if (index.column() == Enums::PRICING_COLUMN::PRICING_COLUMN_COD) // sectia 'Cod'
            return int(Qt::AlignHCenter | Qt::AlignVCenter);
        else if (index.column() == Enums::PRICING_COLUMN::PRICING_COLUMN_PRICE) // sectia 'Pret'
            return int(Qt::AlignHCenter | Qt::AlignVCenter);
        return value;
    default:
        return value;
    }
}
