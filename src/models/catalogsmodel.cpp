#include "catalogsmodel.h"

static QBrush deletionForegroundBrush()
{
    if (globals().isSystemThemeDark) {
        return QBrush(QColor(200, 140, 180));
    } else {
        return QBrush(QColor(255, 230, 255));
    }
}

CatalogsModel::CatalogsModel(DataBase &db, CatalogType::Type typeCatalog, QObject *parent)
    : QAbstractTableModel{parent}
    , m_typeCatalog(typeCatalog)
    , m_loader(db, m_typeCatalog)
{}

void CatalogsModel::setBatchSize(int value)
{
    if (value > 0)
        m_batchSize = value;
}

const CatalogsCommon &CatalogsModel::itemAt(int row) const
{
    return m_items.at(row);
}

void CatalogsModel::setSort(int column, Qt::SortOrder order)
{
    m_loader.setSort(column, order);
    reload();
}

void CatalogsModel::reload()
{
    beginResetModel();
    m_items.clear();
    m_cursor.clear();
    m_hasMore = true;
    m_loading = true;
    endResetModel();
    m_loading = false;

    if (canFetchMore())
        fetchMore();
}

int CatalogsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_items.size();
}

int CatalogsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    switch (m_typeCatalog) {
    case CatalogType::Type::Doctors:
        return DoctorsSections::Uuid + 1;
    case CatalogType::Type::Nurses:
        return NursesSections::Uuid + 1;
    case CatalogType::Type::Users:
        return UsersSections::Uuid + 1;
    case CatalogType::Type::Patients:
        return PatientsColumns::Uuid + 1;
    case CatalogType::Type::Organizations:
        return OrganizationsSections::Uuid + 1;
    default:
        return 0;
    }
}

QVariant CatalogsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    const int row = index.row();
    const int col = index.column();

    if (row < 0 || row >= m_items.size())
        return {};

    const auto &item = itemAt(row);

    static const QIcon icoWrite(":/img/catalogs/item.png");
    static const QIcon icoDelete(":/img/catalogs/item_delete.png");

    static const QBrush fgDeletion = deletionForegroundBrush();//QBrush bgDeletion(QColor(255, 230, 255));

    static const QFont deletedFont = []{
        QFont f;
        f.setItalic(true);
        f.setStrikeOut(true);
        return f;
    }();

    if (role == Qt::DisplayRole) {
        switch (m_typeCatalog) {
        case CatalogType::Type::Doctors:
        case CatalogType::Type::Nurses:
            switch (col) {
            case DoctorsSections::FullName:  return item.fullName;
            case DoctorsSections::Telephone: return item.phone;
            case DoctorsSections::Email:     return item.email;
            case DoctorsSections::Comment:   return item.comment;
            default:                         return {};
            }

        case CatalogType::Type::Users:
            switch (col) {
            case UsersSections::Name:           return item.fullName;
            case UsersSections::LastConnection: return item.txtLastConnection;
            default:                            return {};
            }

        case CatalogType::Type::Patients:
            switch (col) {
            case PatientsColumns::FullName:      return item.fullName;
            case PatientsColumns::Birthday:      return item.txtBirthday;
            case PatientsColumns::MedicalPolicy: return item.medicalPolicy;
            case PatientsColumns::IDNP:          return item.idnp;
            case PatientsColumns::Address:       return item.adress;
            case PatientsColumns::Telephone:     return item.phone;
            case PatientsColumns::Email:         return item.email;
            case PatientsColumns::Comment:       return item.comment;
            default:                              return {};
            }

        case CatalogType::Type::Organizations:
            switch (col) {
            case OrganizationsSections::Name:      return item.fullName;
            case OrganizationsSections::IDNP:      return item.idnp;
            case OrganizationsSections::Address:   return item.adress;
            case OrganizationsSections::Telephone: return item.phone;
            case OrganizationsSections::Email:     return item.email;
            case OrganizationsSections::Comment:   return item.comment;
            default:                               return {};
            }
        default:
            return {};
        }
    }

    if (role == Qt::UserRole) {
        switch (m_typeCatalog) {
        case CatalogType::Type::Doctors:
        case CatalogType::Type::Nurses:
            switch (col) {
            case DoctorsSections::Id:           return item.id;
            case DoctorsSections::DeletionMark: return item.deletionMark;
            default:                            return data(index, Qt::DisplayRole);
            }

        case CatalogType::Type::Users:
            switch (col) {
            case UsersSections::Id:             return item.id;
            case UsersSections::DeletionMark:   return item.deletionMark;
            default:                            return data(index, Qt::DisplayRole);
            }

        case CatalogType::Type::Patients:
            switch (col) {
            case PatientsColumns::Id:           return item.id;
            case PatientsColumns::DeletionMark: return item.deletionMark;
            default:                             return data(index, Qt::DisplayRole);
            }

        case CatalogType::Type::Organizations:
            switch (col) {
            case OrganizationsSections::Id:            return item.id;
            case OrganizationsSections::DeletionMark:  return item.deletionMark;
            case OrganizationsSections::Id_contracts:  return item.idContract;
            default:                                   return data(index, Qt::DisplayRole);
            }
        default:
            return {};
        }
    }

    if (role == SortRole) {
        switch (m_typeCatalog) {
        case CatalogType::Type::Patients:
            if (col == PatientsColumns::Birthday)
                return item.birthday;
            break;

        case CatalogType::Type::Users:
            if (col == UsersSections::LastConnection)
                return item.lastConnection;
            break;

        default:
            break;
        }

        return data(index, Qt::UserRole);
    }

    if (role == Qt::TextAlignmentRole) {
        return QVariant::fromValue(Qt::AlignLeft | Qt::AlignVCenter);
    }

    if (role == Qt::DecorationRole) {
        switch (m_typeCatalog) {
        case CatalogType::Type::Doctors:
        case CatalogType::Type::Nurses:
            if (col == DoctorsSections::DeletionMark)
                return item.deletionMark == 1 ? icoDelete : icoWrite;
            break;

        case CatalogType::Type::Users:
            if (col == UsersSections::DeletionMark)
                return item.deletionMark == 1 ? icoDelete : icoWrite;
            break;

        case CatalogType::Type::Patients:
            if (col == PatientsColumns::DeletionMark)
                return item.deletionMark == 1 ? icoDelete : icoWrite;
            break;

        case CatalogType::Type::Organizations:
            if (col == OrganizationsSections::DeletionMark)
                return item.deletionMark == 1 ? icoDelete : icoWrite;
            break;
        default:
            return {};
        }
    }

    if (role == Qt::ForegroundRole) {
        if (item.deletionMark == 1) {
            return fgDeletion;
        }
    }

    if (role == Qt::FontRole) {
        if (item.deletionMark == 1)
            return deletedFont;
    }

    return {};
}

QVariant CatalogsModel::headerData(int section,
                                   Qt::Orientation orientation,
                                   int role) const
{
    if (role != Qt::DisplayRole)
        return {};

    // numerotare pe verticală (1-based)
    if (orientation == Qt::Vertical)
        return section + 1;

    if (orientation != Qt::Horizontal)
        return {};

    // coloane comune
    switch (section) {
    case 0: return "ID";
    case 1: return ""; // icon deletion / edit
    default:
        break;
    }

    // coloane specifice catalogului
    switch (m_typeCatalog) {

    case CatalogType::Type::Doctors:
    case CatalogType::Type::Nurses:
        switch (section) {
        case DoctorsSections::FullName:  return "Nume";
        case DoctorsSections::Telephone: return "Telefon";
        case DoctorsSections::Email:     return "Email";
        case DoctorsSections::Comment:   return "Comentariu";
        default:                         return {};
        }

    case CatalogType::Type::Users:
        switch (section) {
        case UsersSections::Name:           return "Nume";
        case UsersSections::LastConnection: return "Ultima conectare";
        default:                            return {};
        }

    case CatalogType::Type::Patients:
        switch (section) {
        case PatientsColumns::FullName:       return "Nume";
        case PatientsColumns::Birthday:       return "Data nașterii";
        case PatientsColumns::MedicalPolicy:  return "Polița medicală";
        case PatientsColumns::IDNP:           return "IDNP";
        case PatientsColumns::Address:        return "Adresă";
        case PatientsColumns::Telephone:      return "Telefon";
        case PatientsColumns::Email:          return "E-mail";
        case PatientsColumns::Comment:        return "Comentariu";
        default:                               return {};
        }

    case CatalogType::Type::Organizations:
        switch (section) {
        case OrganizationsSections::Name:      return "Denumire";
        case OrganizationsSections::IDNP:      return "IDNO";
        case OrganizationsSections::Address:   return "Adresă";
        case OrganizationsSections::Telephone: return "Telefon";
        case OrganizationsSections::Email:     return "Email";
        case OrganizationsSections::Comment:   return "Comentariu";
        default:                               return {};
        }
    default: return {};
    }

    return {};
}

void CatalogsModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid())
        return;

    if (m_loading || !m_hasMore)
        return;

    if (m_items.isEmpty())
        loadInitial();
    else
        loadMore();
}

bool CatalogsModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid())
        return false;

    return m_hasMore && !m_loading;
}

void CatalogsModel::clearItems()
{
    beginResetModel();
    m_items.clear();
    m_hasMore = false;
    m_loading = false;
    endResetModel();
}

bool CatalogsModel::isEmpty() const
{
    return m_items.isEmpty();
}

void CatalogsModel::appendItems(const QVector<CatalogsCommon> &items)
{
    if (items.isEmpty())
        return;

    const int first = m_items.size();
    const int last  = first + items.size() - 1;

    beginInsertRows(QModelIndex(), first, last);
    m_items += items;
    endInsertRows();
}

void CatalogsModel::loadInitial()
{
    m_loading = true;

    const auto result = m_loader.loadFirstBatch(m_batchSize);
    m_cursor = result.cursor;
    appendItems(result.items);
    m_hasMore = result.hasMore;

    m_loading = false;
}

void CatalogsModel::loadMore()
{
    if (m_items.isEmpty()) {
        m_hasMore = false;
        return;
    }

    m_loading = true;

    const auto &last = m_items.last();
    const auto result = m_loader.loadNextBatch(m_batchSize, m_cursor, last.id);

    m_cursor = result.cursor;
    appendItems(result.items);
    m_hasMore = result.hasMore;

    m_loading = false;
}
