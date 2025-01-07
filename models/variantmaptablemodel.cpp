#include "variantmaptablemodel.h"
#include "data/globals.h"

#include <QBrush>

//-----------------------------------------------------------------

AbstractColumn::AbstractColumn(QString name) : _name(name)
{
}

//-----------------------------------------------------------------

SimpleColumn::SimpleColumn(QString name) : AbstractColumn(name)
{
}

QVariant SimpleColumn::colData(const QVariantMap &rowData, int role)
{
    if (role != Qt::DisplayRole)
        return QVariant();

    return rowData.value(name());
}

//-----------------------------------------------------------------

VariantMapTableModel::VariantMapTableModel(QObject *parent)
    : QAbstractTableModel{parent}
{}

VariantMapTableModel::TypeNormograms VariantMapTableModel::getTypeNormograms()
{
    return m_type_normograms;
}

void VariantMapTableModel::setTypeNormograms(TypeNormograms typeNormograms)
{
    m_type_normograms = typeNormograms;
    emit typeNormogramsChanged();
}

void VariantMapTableModel::registerColumn(AbstractColumn *column)
{
    _columns.append(column);
}

void VariantMapTableModel::addRow(QVariantMap rowData)
{
    int id = rowData.value("id").toInt();
    beginInsertRows(QModelIndex(), _rowIndex.count(), _rowIndex.count());
    _rowIndex.append(id);
    _dataHash.insert(id, rowData);
    endInsertRows();
}

int VariantMapTableModel::idByRow(int row) const
{
    return _rowIndex.at(row);
}

int VariantMapTableModel::colByName(QString name) const
{
    for (int col = 0; col < _columns.count(); ++col) {
        if (NameByCol(col) == name)
            return col;
    }
    return -1;
}

QString VariantMapTableModel::NameByCol(int col) const
{
    return _columns.at(col)->name();
}


int VariantMapTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _rowIndex.count();
}

int VariantMapTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _columns.count();
}

QVariant VariantMapTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal){
        switch (section) {
        case section_id:
            return "id";
        case section_crl:
            if (m_type_normograms == INDEX_AMNIOTIC || m_type_normograms == DOPPLER)
                return "Săptămâni";
            else
                return "CRL, mm";
        case section_5_centile:
            return "5 centile";
        case section_50_centile:
            return "50 centile";
        case section_95_centile:
            return "95 centile";
        default:
            return VariantMapTableModel::headerData(section, orientation, role);
        }
    }

    return QVariant();//VariantMapTableModel::headerData(section, orientation, role);
}

QVariant VariantMapTableModel::data(const QModelIndex &index, int role) const
{
    if (! index.isValid())
        return QVariant();
    switch (role) {
    case Qt::BackgroundRole:
        if (index.column() == section_crl) {
            if (globals::isSystemThemeDark)
                return QBrush(QColor(120, 100, 50));
            else
                return QBrush(QColor(229,210,153));
        }
        return QVariant();
    case Qt::TextAlignmentRole:
        return int(Qt::AlignHCenter | Qt::AlignVCenter);
    }

    int id = idByRow(index.row());
    QVariantMap rowData = _dataHash.value(id);
    return _columns.at(index.column())->colData(rowData, role);
}

bool VariantMapTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (! index.isValid())
        return false;

    if (role == Qt::EditRole){
        int id = idByRow(index.row());
        _dataHash[id].insert(NameByCol(index.column()), value);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags VariantMapTableModel::flags(const QModelIndex &index) const
{
    if (! index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;// | Qt::ItemIsEditable;
}
