#include "orderinvestigationmodel.h"

OrderInvestigationModel::OrderInvestigationModel(Kind kind, QObject *parent)
    : BaseAbstractModel(parent)
    , m_kind(kind)
{
    registerColumn(std::make_unique<FieldColumn>(
        QStringLiteral("ID"), QStringLiteral("id"), false, Qt::AlignCenter));
    registerColumn(std::make_unique<FieldColumn>(
        QString(), QStringLiteral("deletionMark"), false, Qt::AlignCenter));

    if (kind == Kind::Available) {
        registerColumn(std::make_unique<FieldColumn>(
            QStringLiteral("Pricing (ID)"), QStringLiteral("id_pricings"),
            false, Qt::AlignCenter));
    } else {
        registerColumn(std::make_unique<FieldColumn>(
            QStringLiteral("Order (ID)"), QStringLiteral("id_orderEcho"),
            false, Qt::AlignCenter));
    }

    registerColumn(std::make_unique<FieldColumn>(
        QStringLiteral("Cod MS"), QStringLiteral("cod"), false,
        Qt::AlignHCenter | Qt::AlignVCenter));
    registerColumn(std::make_unique<FieldColumn>(
        QStringLiteral("Denumirea investigației"), QStringLiteral("name"), false,
        Qt::AlignLeft | Qt::AlignVCenter));
    registerColumn(std::make_unique<NumberColumn>(
        QStringLiteral("Preț"), QStringLiteral("price"), 2,
        kind == Kind::Selected, Qt::AlignLeft | Qt::AlignVCenter));
}

OrderInvestigationModel::Kind OrderInvestigationModel::kind() const
{
    return m_kind;
}
