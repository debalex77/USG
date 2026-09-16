#include "orderjournalmodel.h"

OrderJournalModel::OrderJournalModel(DataBase &db, QObject *parent)
    : QAbstractTableModel{parent}
    , m_loader(db)
{

}

void OrderJournalModel::setFilter(const JournalFilter &filter)
{
    m_filter = filter;
    m_loader.setFilter(m_filter);
}

const JournalFilter &OrderJournalModel::filter() const
{
    return m_filter;
}

void OrderJournalModel::setBatchSize(int value)
{
    if (value > 0)
        m_batchSize = value;
}

const OrderJournal::Item &OrderJournalModel::itemAt(int row) const
{
    return m_items.at(row);
}

void OrderJournalModel::reload()
{
    beginResetModel();
    m_items.clear();
    m_hasMore = true;
    m_loading = false;
    endResetModel();

    m_loader.setFilter(m_filter);

    if (canFetchMore())
        fetchMore();
}

int OrderJournalModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_items.size();
}

int OrderJournalModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return OrderJournal::Uuid + 1;
}

QVariant OrderJournalModel::data(const QModelIndex &index, int role) const
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

    static const QIcon icoImage(":/img/common/image.png");

    static const QIcon icoCash(":/img/journal/payment_cash.png");
    static const QIcon icoCard(":/img/journal/master_card.png");
    static const QIcon icoTransfer(":/img/journal/payment_bank.png");

    static const QBrush bgDeletion(QColor(255, 230, 255));

    static const QFont deletedFont = []{
        QFont f;
        f.setItalic(true);
        f.setStrikeOut(true);
        return f;
    }();

    if (role == Qt::DisplayRole) {
        switch (col) {
        case OrderJournal::NumberDoc:        return item.numberDoc;
        case OrderJournal::DateDoc:          return item.dateDocText; // vezi loader -> convertarea in loader
        case OrderJournal::OrganizationName: return item.organizationName;
        case OrderJournal::ContractName:     return item.contractName;
        case OrderJournal::PatientFullName:  return item.patientName;
        case OrderJournal::PatientIdnp:      return item.patientIdnp;
        case OrderJournal::DoctorName:       return item.doctorName;
        case OrderJournal::UserName:         return item.userName;
        case OrderJournal::Sum:              return item.sumText; // vezi loader -> convertarea in loader
        case OrderJournal::Comment:          return item.comment;
        default:
            break;
        }
    }

    if (role == Qt::UserRole) {
        switch (col) {
        case OrderJournal::Id:               return item.id;
        case OrderJournal::DeletionMark:     return item.deletionMark;
        case OrderJournal::AttachedImages:   return item.attachedImages;
        case OrderJournal::CardPayment:      return item.cardPayment;
        case OrderJournal::NumberDoc:        return item.numberDoc;
        case OrderJournal::DateDoc:          return item.dateDoc;
        case OrderJournal::Id_Organization:  return item.idOrganizations;
        case OrderJournal::Id_Contract:      return item.idContracts;
        case OrderJournal::PatientId:       return item.patientId;
        case OrderJournal::Id_Doctor:        return item.idDoctors;
        case OrderJournal::Id_User:          return item.idUsers;
        case OrderJournal::OrganizationName: return item.organizationName;
        case OrderJournal::ContractName:     return item.contractName;
        case OrderJournal::PatientFullName:  return item.patientName;
        case OrderJournal::PatientIdnp:      return item.patientIdnp;
        case OrderJournal::DoctorName:       return item.doctorName;
        case OrderJournal::UserName:         return item.userName;
        case OrderJournal::PatientSearch:    return item.patientSearch;
        case OrderJournal::Sum:              return item.sum;
        case OrderJournal::Comment:          return item.comment;
        case OrderJournal::Uuid:             return item.uuid;
        default:
            break;
        }
    }

    if (role == SortRole) {
        switch (col) {
        case OrderJournal::NumberDoc: {
            bool ok = false;
            const int n = item.numberDoc.toInt(&ok);
            return ok ? QVariant(n) : QVariant(item.numberDoc);
        }
        case OrderJournal::DateDoc:
            return item.dateDoc;
        case OrderJournal::Sum:
            return item.sum;
        default:
            return data(index, Qt::UserRole);
        }
    }

    if (role == Qt::TextAlignmentRole) {
        switch (col) {
        case OrderJournal::Id:
        case OrderJournal::DeletionMark:
        case OrderJournal::AttachedImages:
        case OrderJournal::CardPayment:
        case OrderJournal::NumberDoc:
        case OrderJournal::DateDoc:
        case OrderJournal::UserName:
        case OrderJournal::Sum:
            return QVariant::fromValue(Qt::AlignCenter);
        default:
            return QVariant::fromValue(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    if (role == Qt::DecorationRole) {
        switch (col) {
        case OrderJournal::DeletionMark:
            if (item.deletionMark == 0)
                return icoWrite;
            else if (item.deletionMark == 1)
                return icoDelete;
            else if (item.deletionMark == 2)
                return icoPost;
            return icoWrite;

        case OrderJournal::AttachedImages:
            if (item.attachedImages > 0)
                return icoImage;
            break;

        case OrderJournal::CardPayment:
            if (item.cardPayment == PaymentMethod::Cash)
                return icoCash;
            else if (item.cardPayment == PaymentMethod::Card)
                return icoCard;
            else if (item.cardPayment == PaymentMethod::Transfer)
                return icoTransfer;
            break;

        default:
            break;
        }
    }

    if (role == Qt::BackgroundRole) {
        if (col == OrderJournal::DeletionMark)
            return bgDeletion;
    }

    if (role == Qt::FontRole) {
        if (item.deletionMark == 1) {
            return deletedFont;
        }
    }

    return QVariant();
}

QVariant OrderJournalModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
        return section + 1; // numerotare 1-based
    }

    if (orientation != Qt::Horizontal)
        return QVariant();

    static const QIcon icoImage(":/img/common/image.png");
    static const QIcon icoCard(":/img/journal/master_card.png");

    if (role == Qt::DisplayRole) {
        switch (section) {
        case OrderJournal::Id:               return "ID";
        case OrderJournal::DeletionMark:     return "";
        case OrderJournal::AttachedImages:   return "";
        case OrderJournal::CardPayment:      return "";
        case OrderJournal::NumberDoc:        return "Număr";
        case OrderJournal::DateDoc:          return "Data";
        case OrderJournal::Id_Organization:  return "Organizație (ID)";
        case OrderJournal::Id_Contract:      return "Contract (ID)";
        case OrderJournal::PatientId:       return "Pacient (ID)";
        case OrderJournal::Id_Doctor:        return "Doctor (ID)";
        case OrderJournal::Id_User:          return "Autor (ID)";
        case OrderJournal::OrganizationName: return "Organizație";
        case OrderJournal::ContractName:     return "Contract";
        case OrderJournal::PatientFullName:  return "Pacient";
        case OrderJournal::PatientIdnp:      return "IDNP";
        case OrderJournal::DoctorName:       return "Doctor";
        case OrderJournal::UserName:         return "Autor";
        case OrderJournal::PatientSearch:    return "Căutare pacient";
        case OrderJournal::Sum:              return "Suma";
        case OrderJournal::Comment:          return "Comentariu";
        case OrderJournal::Uuid:             return "UUID";
        default:
            return QVariant();
        }
    }

    if (role == Qt::DecorationRole) {
        switch (section) {
        case OrderJournal::AttachedImages:
            return icoImage;
        case OrderJournal::CardPayment:
            return icoCard;
        default:
            return QVariant();
        }
    }

    return QVariant();
}

void OrderJournalModel::fetchMore(const QModelIndex &parent)
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

bool OrderJournalModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid())
        return false;

    return m_hasMore && !m_loading;
}

void OrderJournalModel::clearItems()
{
    beginResetModel();
    m_items.clear();
    m_hasMore = false;
    m_loading = false;
    endResetModel();
}

bool OrderJournalModel::isEmpty() const
{
    return m_items.isEmpty();
}

void OrderJournalModel::appendItems(const QVector<OrderJournal::Item> &items)
{
    if (items.isEmpty())
        return;

    const int first = m_items.size();
    const int last  = first + items.size() - 1;

    beginInsertRows(QModelIndex(), first, last);
    m_items += items;
    endInsertRows();
}

void OrderJournalModel::loadInitial()
{
    m_loading = true;

    const auto result = m_loader.loadFirstBatch(m_batchSize);
    appendItems(result.items);
    m_hasMore = result.hasMore;

    m_loading = false;
}

void OrderJournalModel::loadMore()
{
    if (m_items.isEmpty()) {
        m_hasMore = false;
        return;
    }

    m_loading = true;

    const auto &last = m_items.last();
    const auto result = m_loader.loadNextBatch(m_batchSize, last.dateDoc, last.id);

    appendItems(result.items);
    m_hasMore = result.hasMore;

    m_loading = false;
}
