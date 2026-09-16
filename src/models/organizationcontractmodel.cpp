#include "organizationcontractmodel.h"

#include <QBrush>
#include <QFont>
#include <QIcon>

#include <common/globals.h>

OrganizationContractModel::OrganizationContractModel(QObject *parent)
    : BaseAbstractModel(parent)
{
    const auto fontRule = [this](const QVariantMap &row) -> QVariant {
        QFont font;
        bool changed = false;

        if (row.value("deletionMark").toInt() == 1) {
            font.setItalic(true);
            font.setStrikeOut(true);
            changed = true;
        }

        if (row.value("id").toInt() == m_mainContractId) {
            font.setBold(true);
            changed = true;
        }

        return changed ? QVariant(font) : QVariant();
    };

    const auto foregroundRule = [this](const QVariantMap &row) -> QVariant {
        const bool isDeleted = row.value("deletionMark").toInt() == 1;
        const bool isMain = row.value("id").toInt() == m_mainContractId;

        if (isDeleted) {
            return globals().isSystemThemeDark
                       ? QVariant::fromValue(QBrush(QColor(200, 140, 180)))
                       : QVariant::fromValue(QBrush(QColor(255, 230, 255)));
        }

        if (isMain)
            return QBrush(QColor(0, 206, 209));

        return {};
    };

    auto idColumn = std::make_unique<FieldColumn>("ID", "id", false, Qt::AlignCenter);
    idColumn->setFontRule(fontRule);
    idColumn->setForegroundRule(foregroundRule);
    registerColumn(std::move(idColumn));

    auto deletionColumn = std::make_unique<FieldColumn>("", "deletionMark", false,
                                                        Qt::AlignCenter);
    deletionColumn->setDecorationRule([](const QVariantMap &row) -> QVariant {
        const int mark = row.value("deletionMark").toInt();
        if (mark == 0)
            return QIcon(":/img/catalogs/item.png");
        if (mark == 1)
            return QIcon(":/img/catalogs/item_delete.png");
        return {};
    });
    deletionColumn->setDisplayFormatter([](const QVariant &) -> QVariant { return {}; });
    deletionColumn->setFontRule(fontRule);
    deletionColumn->setForegroundRule(foregroundRule);
    registerColumn(std::move(deletionColumn));

    auto nameColumn = std::make_unique<FieldColumn>(
        "Denumirea contractului", "name", false, Qt::AlignLeft);
    nameColumn->setFontRule(fontRule);
    nameColumn->setForegroundRule(foregroundRule);
    registerColumn(std::move(nameColumn));

    auto dateColumn = std::make_unique<DateColumn>(
        "Data încep.", "dateInit", "dd.MM.yyyy", false, Qt::AlignLeft);
    dateColumn->setFontRule(fontRule);
    dateColumn->setForegroundRule(foregroundRule);
    registerColumn(std::move(dateColumn));
}

void OrganizationContractModel::setMainContractId(int id)
{
    if (m_mainContractId == id)
        return;

    m_mainContractId = id;
    if (rowCount() > 0)
        emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1),
                         {Qt::FontRole, Qt::ForegroundRole});
}
