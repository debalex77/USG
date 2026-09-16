#include "reportjournalmodel.h"

#include <QBrush>
#include <QFont>
#include <QIcon>

ReportJournalModel::ReportJournalModel(DataBase &db, QObject *parent)
    : QAbstractTableModel(parent), m_loader(db) {}

void ReportJournalModel::setFilter(const JournalFilter &filter)
{
    m_filter = filter;
    m_loader.setFilter(filter);
}

const JournalFilter &ReportJournalModel::filter() const { return m_filter; }
void ReportJournalModel::setBatchSize(int value) { if (value > 0) m_batchSize = value; }
const ReportJournal::Item &ReportJournalModel::itemAt(int row) const { return m_items.at(row); }
QString ReportJournalModel::lastError() const { return m_lastError; }

void ReportJournalModel::reload()
{
    beginResetModel();
    m_items.clear();
    m_hasMore = true;
    m_loading = false;
    m_lastError.clear();
    endResetModel();
    m_loader.setFilter(m_filter);
    fetchMore();
}

int ReportJournalModel::rowCount(const QModelIndex &parent) const { return parent.isValid() ? 0 : m_items.size(); }
int ReportJournalModel::columnCount(const QModelIndex &parent) const { return parent.isValid() ? 0 : ReportJournal::Uuid + 1; }

QVariant ReportJournalModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) return {};
    const auto &item = m_items.at(index.row());
    const int col = index.column();
    if (role == Qt::DisplayRole) {
        switch (col) {
        case ReportJournal::NumberDoc: return item.numberDoc;
        case ReportJournal::DateDoc: return item.dateDocText;
        case ReportJournal::PatientFullName: return item.patientName;
        case ReportJournal::PatientIdnp: return item.patientIdnp;
        case ReportJournal::OrderDescription: return item.orderDescription;
        case ReportJournal::UserName: return item.userName;
        case ReportJournal::Conclusion: return item.conclusion;
        case ReportJournal::Comment: return item.comment;
        default: break;
        }
    }
    if (role == Qt::UserRole) {
        switch (col) {
        case ReportJournal::Id: return item.id;
        case ReportJournal::DeletionMark: return item.deletionMark;
        case ReportJournal::AttachedImages: return item.attachedImages;
        case ReportJournal::NumberDoc: return item.numberDoc;
        case ReportJournal::DateDoc: return item.dateDoc;
        case ReportJournal::OrderId: return item.orderId;
        case ReportJournal::PatientId: return item.patientId;
        case ReportJournal::UserId: return item.userId;
        case ReportJournal::PatientFullName: return item.patientName;
        case ReportJournal::PatientIdnp: return item.patientIdnp;
        case ReportJournal::OrderDescription: return item.orderDescription;
        case ReportJournal::UserName: return item.userName;
        case ReportJournal::Conclusion: return item.conclusion;
        case ReportJournal::Comment: return item.comment;
        case ReportJournal::PatientSearch: return item.patientSearch;
        case ReportJournal::Uuid: return item.uuid;
        default: break;
        }
    }
    if (role == SortRole) return col == ReportJournal::DateDoc ? QVariant(item.dateDoc) : data(index, Qt::UserRole);
    if (role == Qt::DecorationRole) {
        static const QIcon writeIcon(":/img/documents/doc_write.png");
        static const QIcon deleteIcon(":/img/documents/doc_delete.png");
        static const QIcon postIcon(":/img/documents/doc_post.png");
        static const QIcon imageIcon(":/img/common/image.png");
        if (col == ReportJournal::DeletionMark) {
            if (item.deletionMark == 1) return deleteIcon;
            if (item.deletionMark == 2) return postIcon;
            return writeIcon;
        }
        if (col == ReportJournal::AttachedImages && item.attachedImages > 0) return imageIcon;
    }
    if (role == Qt::TextAlignmentRole)
        return QVariant::fromValue(col <= ReportJournal::UserId || col == ReportJournal::UserName
                                       ? Qt::AlignCenter : Qt::AlignLeft | Qt::AlignVCenter);
    if (role == Qt::BackgroundRole && col == ReportJournal::DeletionMark) return QBrush(QColor(255, 230, 255));
    if (role == Qt::FontRole && item.deletionMark == 1) {
        QFont font; font.setItalic(true); font.setStrikeOut(true); return font;
    }
    return {};
}

QVariant ReportJournalModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical && role == Qt::DisplayRole) return section + 1;
    if (orientation != Qt::Horizontal) return {};
    if (role == Qt::DecorationRole && section == ReportJournal::AttachedImages) return QIcon(":/img/common/image.png");
    if (role != Qt::DisplayRole) return {};
    switch (section) {
    case ReportJournal::Id: return "ID";
    case ReportJournal::DeletionMark: case ReportJournal::AttachedImages: return "";
    case ReportJournal::NumberDoc: return tr("Număr");
    case ReportJournal::DateDoc: return tr("Data");
    case ReportJournal::OrderId: return tr("Comandă (ID)");
    case ReportJournal::PatientId: return tr("Pacient (ID)");
    case ReportJournal::UserId: return tr("Autor (ID)");
    case ReportJournal::PatientFullName: return tr("Pacient");
    case ReportJournal::PatientIdnp: return "IDNP";
    case ReportJournal::OrderDescription: return tr("Comandă ecografică");
    case ReportJournal::UserName: return tr("Autor");
    case ReportJournal::Conclusion: return tr("Concluzie");
    case ReportJournal::Comment: return tr("Comentariu");
    case ReportJournal::PatientSearch: return tr("Căutare pacient");
    case ReportJournal::Uuid: return "UUID";
    default: return {};
    }
}

bool ReportJournalModel::canFetchMore(const QModelIndex &parent) const
{ return !parent.isValid() && m_hasMore && !m_loading; }

void ReportJournalModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid() || !canFetchMore(parent)) return;
    m_loading = true;
    const auto batch = m_items.isEmpty()
        ? m_loader.loadFirstBatch(m_batchSize)
        : m_loader.loadNextBatch(m_batchSize, m_items.constLast().dateDoc, m_items.constLast().id);
    m_lastError = batch.error;
    if (!batch.items.isEmpty()) {
        const int first = m_items.size();
        beginInsertRows({}, first, first + batch.items.size() - 1);
        m_items += batch.items;
        endInsertRows();
    }
    m_hasMore = batch.hasMore;
    m_loading = false;
}
