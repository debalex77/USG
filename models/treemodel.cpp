#include "treeitem.h"
#include "treemodel.h"
#include <QDebug>

TreeModel::TreeModel(bool isEditable, const QStringList &headers, const QString &data, QObject *parent)
    : QAbstractItemModel(parent), m_isEditable(isEditable)
{

    Q_UNUSED(headers)

    QVector <QVariant> rootData;
    rootData << tr("Arbore investigatiilor");                       // arbore
    rootItem = new TreeItem(rootData, TreeItemType::TreeItemGroup); // crearea rootData
    setupModelData(data.split(QString("\n")), rootItem);            // procesarea datelor din solicitare
}

TreeModel::~TreeModel()
{
    delete rootItem;
}

int TreeModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (! index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::DecorationRole && role != Qt::EditRole)
        return QVariant();

    TreeItem *item = getItem(index);
    return item->data(index.column(), role);
}

TreeItem *TreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (m_isEditable)
        return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    else
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section, role);
    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    TreeItem *parentItem = getItem(parent);
    TreeItem *childItem = parentItem->child(row);

    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem = getItem(parent);
    return parentItem->childCount();
}

void TreeModel::setupModelData(const QStringList &lines, TreeItem *parent)
{
    QList<TreeItem*> parents;
    QList<int> indentations;

    parents << parent;
    indentations << 0;

    int number = 0; // primul simbol nespatial - position

    while (number < lines.count()) {
        int position = 0;

        while (position < lines[number].length()) {
            if (lines[number].at(position) != ' ')
                break;
            position++;
        }

        bool this_item;                        // determinam item or group
        QString item = lines[number].left(3);
        if (item == " it" || item == "it_")
            this_item = true;
        else if (item == "gr_")
            this_item = false;
        else
            this_item = true;

        QString lineData = lines[number].mid(position + 3).trimmed(); // eliminam spatiu + simbolurile ' it' or 'it_' or 'gr_'

        if (!lineData.isEmpty()) {

            QStringList columnStrings = lineData.split(":::", Qt::KeepEmptyParts); // separator de sectii

            QVector <QVariant> columnData; // lista sectiilor
            for (int column = 0; column < columnStrings.count(); ++column)
                columnData << columnStrings[column];

            if (position > indentations.last()) {

                // Ultimul descendent al actualului părinte va fi acum un nou părinte,
                // în timp ce actualul părinte nu are descendenți
                if (parents.last()->childCount() > 0) {
                    parents << parents.last()->child(parents.last()->childCount()-1);
                    indentations << position;
                }
            }
            else {
                while (position < indentations.last() && parents.count() > 0) {
                    parents.pop_back();
                    indentations.pop_back();
                }
            }
            // Adăugați un nou nod la lista descendenților părintelui actual
            if (this_item)
                parents.last()->appendChild(new TreeItem(columnData, TreeItemType::TreeItemItem, parents.last()));
            else
                parents.last()->appendChild(new TreeItem(columnData, TreeItemType::TreeItemGroup, parents.last()));
        }
        ++number;
    }
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    TreeItem *item = getItem(index);
    bool result = item->setData(index.column(), value);
    if (result) {
        emit dataChanged(index, index);
    }
    return result;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    bool result = rootItem->setData(section, value);

    if (result) {
        emit headerDataChanged(orientation, section, section);
    }
    return result;
}

bool TreeModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
    bool success;

    beginInsertColumns(parent, position, position + columns - 1);
    success = rootItem->insertColumns(position, columns);
    endInsertColumns();

    return success;
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, rootItem->columnCount());
    endInsertRows();

    return success;
}

bool TreeModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
    bool success;

    beginRemoveColumns(parent, position, position + columns - 1);
    success = rootItem->removeColumns(position, columns);
    endRemoveColumns();
    if (rootItem->columnCount() == 0)
        removeRows(0, rowCount());

    return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success = true;
    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

