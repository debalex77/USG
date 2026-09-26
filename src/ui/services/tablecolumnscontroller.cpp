#include "tablecolumnscontroller.h"

TableColumnsController::TableColumnsController(QTableView *tableView,
                                               QObject *parent)
    : QObject{parent}
    , m_tableView(tableView)
    , m_menu(new QMenu(tableView))
{

}

void TableColumnsController::setFixedHiddenColumns(const QSet<int> &columns)
{
    m_fixedHiddenColumns = columns;

    if (!m_tableView || !m_tableView->model())
        return;

    for (int col : std::as_const(m_fixedHiddenColumns))
        m_tableView->setColumnHidden(col, true);
}

void TableColumnsController::setExcludedFromMenuColumns(const QSet<int> &columns)
{
    m_excludedFromMenuColumns = columns;
}

void TableColumnsController::setCustomTitle(int column, const QString &title)
{
    m_customTitles[column] = title;
}

void TableColumnsController::setHiddenSections(const QMap<int, bool> &hiddenSections)
{
    if (!m_tableView || !m_tableView->model())
        return;

    const int count = m_tableView->model()->columnCount();
    for (int col = 0; col < count; ++col) {
        bool hide = hiddenSections.value(col, false);

        if (m_fixedHiddenColumns.contains(col))
            hide = true;

        m_tableView->setColumnHidden(col, hide);
    }
}

QMap<int, bool> TableColumnsController::hiddenSections() const
{
    QMap<int, bool> result;

    if (!m_tableView || !m_tableView->model())
        return result;

    const int count = m_tableView->model()->columnCount();
    for (int col = 0; col < count; ++col) {
        if (m_fixedHiddenColumns.contains(col))
            continue;

        if (m_excludedFromMenuColumns.contains(col))
            continue;

        result[col] = m_tableView->isColumnHidden(col);
    }

    return result;
}

QString TableColumnsController::columnTitle(int column) const
{
    if (m_customTitles.contains(column))
        return m_customTitles.value(column);

    if (!m_tableView || !m_tableView->model())
        return QString("Column %1").arg(column);

    const QString title = m_tableView->model()
                              ->headerData(column, Qt::Horizontal, Qt::DisplayRole)
                              .toString()
                              .trimmed();

    return title.isEmpty() ? QString("Column %1").arg(column) : title;
}

bool TableColumnsController::isToggleAllowed(int column) const
{
    return !m_fixedHiddenColumns.contains(column);
}

void TableColumnsController::rebuildMenu()
{
    if (!m_menu || !m_tableView || !m_tableView->model())
        return;

    m_menu->clear();

    const int count = m_tableView->model()->columnCount();
    for (int col = 0; col < count; ++col) {

        if (m_fixedHiddenColumns.contains(col))
            continue;

        if (m_excludedFromMenuColumns.contains(col))
            continue;

        QAction *act = m_menu->addAction(columnTitle(col));
        act->setCheckable(true);
        act->setChecked(!m_tableView->isColumnHidden(col));
        act->setData(col);

        connect(act, &QAction::toggled, this, [this, col](bool checked) {
            if (!m_tableView)
                return;

            m_tableView->setColumnHidden(col, !checked);
            emit columnsChanged();
        });
    }
}

void TableColumnsController::showMenu(const QPoint &globalPos)
{
    rebuildMenu();

    if (m_menu)
        m_menu->exec(globalPos);
}
