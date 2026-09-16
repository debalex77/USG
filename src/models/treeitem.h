#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVector>
#include <QVariant>

enum TreeItemType {TreeItemGroup, TreeItemItem};
class TreeItem {
public:
    explicit TreeItem (const QVector<QVariant> &data, TreeItemType type, TreeItem *parentItem = nullptr);
    ~TreeItem();

    void appendChild(TreeItem *child); // adaugarea nodului
    TreeItem *child(int row);          // returneaza elementul subordonat
    int childCount() const;            // cantitatea elementelor subordonate
    int columnCount() const;           // cantitatea sectiilor
    QVariant data(int column, int role) const;   // returneaza datele
    int childNumber() const;           // nr.randului

    TreeItem *parentItem();
    bool insertChildren(int position, int count, int columns);
    bool insertColumns(int position, int columns);
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    bool setData(int column, const QVariant &value);

private:
    QList <TreeItem*>  m_childItems;
    QVector <QVariant> m_itemData;
    TreeItem           *m_parentItem;
    TreeItemType       m_type;
};
#endif // TREEITEM_H
