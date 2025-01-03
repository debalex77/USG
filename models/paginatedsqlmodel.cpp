#include "paginatedsqlmodel.h"

PaginatedSqlModel::PaginatedSqlModel(QObject *parent)
    : QSqlQueryModel{parent}, columnCountValue(0)
{
    db = new DataBase(this);
}

PaginatedSqlModel::~PaginatedSqlModel()
{
    delete db;
}

void PaginatedSqlModel::setPagination(int limit, int offset)
{
    this->limit = limit;
    this->offset = offset;
}

void PaginatedSqlModel::resetPagination()
{
    rows.clear();   // Șterge toate datele
    offset = 0;     // Resetează offset-ul
    beginResetModel();
    endResetModel();
}

void PaginatedSqlModel::setStrQuery(const QString strQry)
{
    m_strQry = strQry;
}

void PaginatedSqlModel::fetchMoreData()
{
    // auto *task = new LoadDataTask(limit, offset, this);
    // task->setQuery(m_strQry);
    // connect(task, &LoadDataTask::dataReady, this, &PaginatedSqlModel::onDataLoaded);
    // connect(task, &LoadDataTask::columnCountDetermined, this, &PaginatedSqlModel::updateColumnCount);
    // QThreadPool::globalInstance()->start(task);

    loadDataAsync();
}

void PaginatedSqlModel::fetchMoreDataFull()
{
    if (m_strQry == nullptr || m_strQry.isEmpty()) {
        qCritical(logCritical()) << this->metaObject()->className()
        << ": nu este setat textul solicitarii pentru popularea documentelor !!!";
        return;
    }

    int num_records = 0;
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM orderEcho;");
    if (query.exec() && query.next()) {
        num_records = query.value(0).toInt();
    }

    query.prepare(m_strQry);
    if (! query.exec()) {
        qCritical(logCritical()) << this->metaObject()->className()
        << ": eroare solicitarii - " << query.lastError().text();
        return;
    }

    int progress = 0;

    rows.clear();

    // Adaugă rezultatele la lista internă
    while (query.next()) {
        QVector<QVariant> row;
        for (int i = 0; i < query.record().count(); ++i) {
            row.append(query.value(i));
        }
        rows.append(row);

        ++progress;
        emit updateProgress(num_records, progress);

        // Setează numărul de coloane (o singură dată, la prima interogare)
        if (columnCountValue == 0) {
            columnCountValue = query.record().count();
        }
    }

    emit finishedProgress(tr("Au fost încărcate %1 documente.").arg(QString::number(num_records)));

    // Reîmprospătează modelul
    updateModel();
}

void PaginatedSqlModel::updateVisibleData(int scrollPosition, int rowHeight)
{
    if (m_strQry == nullptr || m_strQry.isEmpty()) {
        qCritical(logCritical()) << this->metaObject()->className()
        << ": nu este setat textul solicitarii pentru popularea documentelor !!!";
        return;
    }

    int startRow = scrollPosition / rowHeight; // Calculam rândul de start în funcție de scroll
    int endRow = startRow + limit;            // Calculam rândul final vizibil

    QSqlQuery query;
    query.prepare(m_strQry);
    query.bindValue(":limit", limit);
    query.bindValue(":offset", startRow);

    if (! query.exec()) {
        qCritical(logCritical()) << this->metaObject()->className()
                                 << ": eroare la interogare - " << query.lastError().text();
        return;
    }

    // Actualizează rândurile vizibile
    int rowIndex = startRow;
    while (query.next()) {
        for (int col = 0; col < query.record().count(); ++col) {
            rows[rowIndex][col] = query.value(col);
        }
        rowIndex++;
    }

    emit dataChanged(index(startRow, 0), index(endRow - 1, columnCount() - 1));
}

void PaginatedSqlModel::loadDataAsync()
{
    if (m_strQry == nullptr || m_strQry.isEmpty()) {
        qCritical(logCritical()) << this->metaObject()->className()
                                 << ": nu este setat textul solicitarii pentru popularea documentelor !!!";
        return;
    }

    QSqlQuery query;
    query.prepare(m_strQry);
    query.bindValue(":limit", limit);
    query.bindValue(":offset", offset);
    if (! query.exec()) {
        qCritical(logCritical()) << this->metaObject()->className()
                                 << ": eroare solicitarii - " << query.lastError().text();
        return;
    }

    // Adaugă rezultatele la lista internă
    while (query.next()) {
        QVector<QVariant> row;
        for (int i = 0; i < query.record().count(); ++i) {
            row.append(query.value(i));
        }
        rows.append(row);

        // Setează numărul de coloane (o singură dată, la prima interogare)
        if (columnCountValue == 0) {
            columnCountValue = query.record().count();
        }
    }

    // Actualizează offset-ul pentru următoarea încărcare
    offset += limit;

    // Reîmprospătează modelul
    updateModel();
}

void PaginatedSqlModel::onDataLoaded(const QVector<QVector<QVariant> > &newRows)
{
    rows.append(newRows);
    // Actualizează offset-ul pentru următoarea încărcare
    offset += limit;
    // Reîmprospătează modelul
    updateModel();
}

int PaginatedSqlModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return rows.size();
}

int PaginatedSqlModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return columnCountValue; // Returnează numărul de coloane
}

QVariant PaginatedSqlModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    int row = index.row();
    int col = index.column();

    if (row < 0 || row >= rows.size() || col < 0 || col >= rows[row].size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {

        if (col == Enums::ORDER_COLUMN::ORDER_DEL_MARK)
            return QVariant(); // Nu afișa nimic

        if (col == Enums::ORDER_COLUMN::ORDER_ATTACHED_IMAGE && rows[row][col].toInt() == 0)
            return QVariant(); // Nu afișa nimic

        if (col == Enums::ORDER_COLUMN::ORDER_CARD_PAYMENT)
            return QVariant(); // Nu afișa nimic

        if (col == Enums::ORDER_COLUMN::ORDER_DATE_DOC)
            return rows[row][col].toDateTime().toString("dd.MM.yyyy hh:mm:ss");

        if (col == Enums::ORDER_COLUMN::ORDER_SUM) {
            double _num = rows[row][col].toDouble();
            return QVariant(QString("%1").arg(_num, 0, 'f', 2));
        }

        return rows[row][col];
    }

    if (role == Qt::DecorationRole) {

        // write & valide
        if (col == Enums::ORDER_COLUMN::ORDER_DEL_MARK && rows[row][col].toInt() == Enums::IDX::IDX_WRITE)
            return QIcon(":img/document_write.png");
        if (col == Enums::ORDER_COLUMN::ORDER_DEL_MARK && rows[row][col].toInt() == Enums::IDX::IDX_POST)
            return QIcon(":img/document_accept.png");

        // attachedImages
        if (col == Enums::ORDER_COLUMN::ORDER_ATTACHED_IMAGE && rows[row][col].toInt() == Enums::ORDER_IMAGE_VIDEO::NOT_ATTACHED) // 0
            return QVariant();
        if (col == Enums::ORDER_COLUMN::ORDER_ATTACHED_IMAGE && rows[row][col].toInt() == Enums::ORDER_IMAGE_VIDEO::ATTACHED_IMAGE) // 1
            return QIcon(":img/image-files.png");
        if (col == Enums::ORDER_COLUMN::ORDER_ATTACHED_IMAGE && rows[row][col].toInt() == Enums::ORDER_IMAGE_VIDEO::ATTACHED_VIDEO)
            return QIcon(":img/video.png");

        // card payment
        if (col == Enums::ORDER_COLUMN::ORDER_CARD_PAYMENT && rows[row][col].toInt() == Enums::ORDER_PAYMENT::PAYMENT_CASH)
            return QIcon(":img/payment_cash.png");
        if (col == Enums::ORDER_COLUMN::ORDER_CARD_PAYMENT && rows[row][col].toInt() == Enums::ORDER_PAYMENT::PAYMENT_CARD)
            return QIcon(":img/master_card.png");
        if (col == Enums::ORDER_COLUMN::ORDER_CARD_PAYMENT && rows[row][col].toInt() == Enums::ORDER_PAYMENT::PAYMENT_TRANSFER)
            return QIcon(":img/payment_bank.png");

    }

    if (role == Qt::TextAlignmentRole) {

        if (col == Enums::ORDER_COLUMN::ORDER_NUMBER_DOC ||
            col == Enums::ORDER_COLUMN::ORDER_SUM ||
            col == Enums::ORDER_COLUMN::ORDER_USER)
            return int(Qt::AlignHCenter | Qt::AlignVCenter);

    }

    if (role == Qt::ToolTipRole) {

        return rows[row][col].toString();

    }

    return QVariant();
}

bool PaginatedSqlModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    return QSqlQueryModel::setHeaderData(section, orientation, value, role);
}

void PaginatedSqlModel::updateColumnCount(int columnCount)
{
    if (columnCountValue != columnCount) {
        columnCountValue = columnCount;
        emit headerDataChanged(Qt::Horizontal, 0, columnCount - 1);
    }
}

void PaginatedSqlModel::updateModel()
{
    beginResetModel();
    endResetModel();
}
