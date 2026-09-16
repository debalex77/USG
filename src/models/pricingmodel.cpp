#include "pricingmodel.h"


PricingModel::PricingModel(QObject *parent)
    : QAbstractTableModel{parent}
{}


int PricingModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.size();
}

int PricingModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return PricingsJournal::Uuid + 1; // ultima sectie +1
}

QVariant PricingModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const int row = index.row();
    const int col = index.column();

    if (row < 0 || row >= m_items.size())
        return QVariant();

    const auto &item = itemAt(row);

    static const QIcon icoWrite(":/img/documents/doc_write.png");
    static const QIcon icoDelete(":/img/documents/doc_delete.png");
    static const QIcon icoPost(":/img/documents/doc_post.png");
    static const QIcon icoImage(":/img/image_x16.png");
    static const QIcon icoCard(":/img/card_x16.png");

    static const QBrush bgDeletion(QColor(255, 230, 255));

    static const QFont deletedFont = []{
        QFont f;
        f.setItalic(true);
        f.setStrikeOut(true);
        return f;
    }();

    if (role == Qt::DisplayRole) {
        switch (col) {
        case PricingsJournal::NumberDoc:        return item.numberDoc;
        case PricingsJournal::DateDoc:          return item.dateDocText;
        case PricingsJournal::OrganizationName: return item.organizationName;
        case PricingsJournal::ContractName:     return item.contractName;
        case PricingsJournal::TypePriceName:    return item.typePriceName;
        case PricingsJournal::UserName:         return item.userName;
        case PricingsJournal::Comment:          return item.comment;
        default:
            break;
        }
    }

    if (role == Qt::UserRole) {
        switch (col) {
        case PricingsJournal::Id:               return item.id;
        case PricingsJournal::DeletionMark:     return item.deletionMark;
        case PricingsJournal::NumberDoc:        return item.numberDoc.toInt();
        case PricingsJournal::DateDoc:          return item.dateDoc;
        case PricingsJournal::Id_Organizations: return item.idOrganizations;
        case PricingsJournal::Id_Contracts:     return item.idContracts;
        case PricingsJournal::Id_TypesPrices:   return item.idTypesPrices;
        case PricingsJournal::Id_Users:         return item.idUsers;
        case PricingsJournal::OrganizationName: return item.organizationName;
        case PricingsJournal::ContractName:     return item.contractName;
        case PricingsJournal::TypePriceName:    return item.typePriceName;
        case PricingsJournal::UserName:         return item.userName;
        case PricingsJournal::Comment:          return item.comment;
        case PricingsJournal::Uuid:             return item.uuid;
        }
    }

    if (role == SortRole) {
        switch (col) {
        case PricingsJournal::NumberDoc: {
            bool ok = false;
            int n = item.numberDoc.toInt(&ok);
            return ok ? QVariant(n) : QVariant(item.numberDoc);
        }
        default:
            return data(index, Qt::UserRole);
        }
    }

    if (role == Qt::TextAlignmentRole) {
        switch (col) {
        case PricingsJournal::Id:           return Qt::AlignCenter;
        case PricingsJournal::DeletionMark: return Qt::AlignCenter;;
        case PricingsJournal::NumberDoc:    return Qt::AlignCenter;
        case PricingsJournal::DateDoc:      return Qt::AlignCenter;
        case PricingsJournal::UserName:     return Qt::AlignCenter;
        default: return QVariant::fromValue(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    if (role == Qt::DecorationRole) {
        switch (col) {
        case PricingsJournal::DeletionMark:
            if (item.deletionMark == DocStatus::Write)
                return icoWrite;
            else if (item.deletionMark == DocStatus::DeletionMark)
                return icoDelete;
            else if (item.deletionMark == DocStatus::Post)
                return icoPost;
            else
                return icoWrite;
            break;
        default:
            break;
        }
    }

    if (role == Qt::BackgroundRole) {
        if (col == PricingsJournal::DeletionMark) {
            return bgDeletion;
        }
    }

    if (role == Qt::FontRole) {
        if (col == PricingSections::DeletionMark) {
            return deletedFont;
        }
    }

    return QVariant();
}

QVariant PricingModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
        return section + 1; // numerotare 1-based
    }

    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
    case PricingsJournal::Id:               return "ID";
    case PricingsJournal::DeletionMark:     return ""; // v-a fi imagine salvat/validat
    case PricingsJournal::NumberDoc:        return "Număr";
    case PricingsJournal::DateDoc:          return "Data";
    case PricingsJournal::Id_TypesPrices:   return "Tipul prețului (ID)";
    case PricingsJournal::Id_Organizations: return "Organizație (ID)";
    case PricingsJournal::Id_Contracts:     return "Contract (ID)";
    case PricingsJournal::Id_Users:         return "Autor (ID)";
    case PricingsJournal::TypePriceName:    return "Tipul prețului";
    case PricingsJournal::OrganizationName: return "Organizație";
    case PricingsJournal::ContractName:     return "Contract";
    case PricingsJournal::UserName:         return "Autor";
    case PricingsJournal::Comment:          return "Comentariu";
    case PricingsJournal::Uuid:             return "UUID";
    default: return QVariant();
    }
}

void PricingModel::setItems(const QVector<PricingsJournal::Item> &items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
}

void PricingModel::clearItems()
{
    beginResetModel();
    m_items.clear();
    endResetModel();
}

const PricingsJournal::Item &PricingModel::itemAt(int row) const
{
    return m_items.at(row);
}
