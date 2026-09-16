#ifndef TABLECOLUMNSCONTROLLER_H
#define TABLECOLUMNSCONTROLLER_H

#include <QObject>
#include <QPointer>
#include <QSet>
#include <QMap>
#include <QHash>
#include <QTableView>
#include <QMenu>
#include <QAction>
#include <QAbstractItemModel>

class QTableView;
class QMenu;

class TableColumnsController : public QObject
{
    Q_OBJECT
public:
    explicit TableColumnsController(QTableView *tableView,
                                    QObject *parent = nullptr);

    void setFixedHiddenColumns(const QSet<int> &columns);
    void setExcludedFromMenuColumns(const QSet<int> &columns);
    void setCustomTitle(int column, const QString &title);

    void setHiddenSections(const QMap<int, bool> &hiddenSections);
    QMap<int, bool> hiddenSections() const;

    void showMenu(const QPoint &globalPos);

signals:
    void columnsChanged();

private:
    void rebuildMenu();
    QString columnTitle(int column) const;
    bool isToggleAllowed(int column) const;

private:
    QPointer<QTableView> m_tableView;
    QPointer<QMenu> m_menu;
    QSet<int> m_excludedFromMenuColumns;
    QSet<int> m_fixedHiddenColumns;
    QHash<int, QString> m_customTitles;
};

#endif // TABLECOLUMNSCONTROLLER_H
