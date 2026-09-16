#include "tabledocmodel.h"

TableDocModel::TableDocModel(QObject *parent)
    : QSqlTableModel{parent},
    m_flags(Qt::NoItemFlags)
{}

void TableDocModel::setPurpose(Purpose purpose)
{
    m_purpose = purpose;
}

void TableDocModel::setFlags(Qt::ItemFlags flag)
{
    m_flags = flag;
}

QVariant TableDocModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const QVariant value = QSqlTableModel::data(index, role);

    const double price = QSqlTableModel::data(
                             this->index(index.row(), PricingsTableSection::Price),
                             Qt::EditRole).toDouble();

#if defined(Q_OS_WIN)
    QFont font;
    font.setPointSize(9);
#endif

    if (role == Qt::DisplayRole && index.column() == PricingsTableSection::Price) {
        return QString::number(value.toDouble(), 'f', 2);
    }

    if (role == Qt::EditRole)
        return value;

    if (role == Qt::FontRole) {
#if defined(Q_OS_WIN)
        return font;
#else
        return QVariant();
#endif
    }

    if (role == Qt::BackgroundRole) {


        if (m_purpose == Table_destination) {
            if (qFuzzyIsNull(price))
                return QBrush(QColor(191,198,188));
            return globals().isSystemThemeDark
                       ? QBrush(QColor(52, 112, 93))
                       : QBrush(QColor(214,235,206));
        }

        if (index.column() != PricingsTableSection::Price) {
            return globals().isSystemThemeDark
                       ? QBrush(QColor(52, 112, 93))
                       : QBrush(QColor(217,255,210));
        }

        return QVariant();
    }

    if (role == Qt::ForegroundRole) {

        if (m_purpose == Table_destination && qFuzzyIsNull(price))
            return QBrush(QColor(88,94,86));

        return QVariant();
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() == PricingsTableSection::Cod ||
            index.column() == PricingsTableSection::Price) {
            return int(Qt::AlignHCenter | Qt::AlignVCenter);
        }
        return QVariant();
    }

    return value;
}

Qt::ItemFlags TableDocModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    // dacă flags sunt setate manual
    if (m_flags != Qt::NoItemFlags)
        return m_flags;

    Qt::ItemFlags flags = QSqlTableModel::flags(index);

    if (m_flags == Qt::NoItemFlags)
    if (index.column() == PricingsTableSection::Price)
        flags |= Qt::ItemIsEditable;

    return flags;
}
