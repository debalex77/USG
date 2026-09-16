#include "onlineaccountmodel.h"

OnlineAccountModel::OnlineAccountModel(QObject *parent)
    : BaseAbstractModel(parent)
{
    registerColumn(std::make_unique<FieldColumn>(
        QStringLiteral("ID"), QStringLiteral("id"), false, Qt::AlignCenter));

    auto statusColumn = std::make_unique<FieldColumn>(
        QString(), QStringLiteral("deletionMark"), false, Qt::AlignCenter);
    statusColumn->setDecorationRule([](const QVariantMap &row) -> QVariant {
        const int mark = row.value(QStringLiteral("deletionMark")).toInt();
        if (mark == 0)
            return QIcon(QStringLiteral(":/img/catalogs/item.png"));
        if (mark == 1)
            return QIcon(QStringLiteral(":/img/catalogs/item_delete.png"));
        return {};
    });
    statusColumn->setDisplayFormatter([](const QVariant &) -> QVariant { return {}; });
    registerColumn(std::move(statusColumn));

    auto organizationColumn = std::make_unique<FieldColumn>(
        QStringLiteral("Organization (ID)"), QStringLiteral("id_organizations"),
        false, Qt::AlignCenter);
    organizationColumn->setDisplayFormatter([](const QVariant &) -> QVariant { return {}; });
    registerColumn(std::move(organizationColumn));

    auto userColumn = std::make_unique<FieldColumn>(
        QStringLiteral("User (ID)"), QStringLiteral("id_users"),
        false, Qt::AlignCenter);
    userColumn->setDisplayFormatter([](const QVariant &) -> QVariant { return {}; });
    registerColumn(std::move(userColumn));

    registerColumn(std::make_unique<FieldColumn>(
        QStringLiteral("E-mail"), QStringLiteral("email"), false,
        Qt::AlignVCenter | Qt::AlignHCenter));
    registerColumn(std::make_unique<FieldColumn>(
        QStringLiteral("SMTP server"), QStringLiteral("smtp_server"), false,
        Qt::AlignVCenter | Qt::AlignHCenter));
    registerColumn(std::make_unique<FieldColumn>(
        QStringLiteral("Port"), QStringLiteral("port"), false, Qt::AlignVCenter));
    registerColumn(std::make_unique<FieldColumn>(
        QStringLiteral("User name"), QStringLiteral("username"), false,
        Qt::AlignVCenter | Qt::AlignHCenter));
}
