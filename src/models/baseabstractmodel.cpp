#include "baseabstractmodel.h"

#include <QMetaType>
#include <QSqlDriver>
#include <QRegularExpression>
#include <algorithm>
#include <cmath>

//********************************************************
// helpers
//********************************************************

namespace {

static bool isSqlIdentifier(const QString &identifier)
{
    static const QRegularExpression pattern(
        QStringLiteral("^[A-Za-z_][A-Za-z0-9_]*$"));
    return pattern.match(identifier).hasMatch();
}

static bool isNumericMetaType(const QVariant &v)
{
    const int id = v.metaType().id();
    switch (id) {
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
    case QMetaType::Double:
    case QMetaType::Float:
    case QMetaType::Short:
    case QMetaType::UShort:
    case QMetaType::Char:
    case QMetaType::UChar:
        return true;
    default:
        return false;
    }
}

static int compareVariants(const QVariant &left, const QVariant &right)
{
    if (!left.isValid() && !right.isValid())
        return 0;
    if (!left.isValid())
        return -1;
    if (!right.isValid())
        return 1;

    if (isNumericMetaType(left) || isNumericMetaType(right)) {
        const double l = left.toDouble();
        const double r = right.toDouble();

        if (std::fabs(l - r) < 0.0000001)
            return 0;
        return (l < r) ? -1 : 1;
    }

    if (left.metaType().id() == QMetaType::QDate || right.metaType().id() == QMetaType::QDate) {
        const QDate l = left.toDate();
        const QDate r = right.toDate();
        if (l == r)
            return 0;
        return (l < r) ? -1 : 1;
    }

    if (left.metaType().id() == QMetaType::QDateTime ||
        right.metaType().id() == QMetaType::QDateTime) {
        const QDateTime l = left.toDateTime();
        const QDateTime r = right.toDateTime();
        if (l == r)
            return 0;
        return (l < r) ? -1 : 1;
    }

    if (left.metaType().id() == QMetaType::Bool || right.metaType().id() == QMetaType::Bool) {
        const bool l = left.toBool();
        const bool r = right.toBool();
        if (l == r)
            return 0;
        return (!l && r) ? -1 : 1;
    }

    return QString::localeAwareCompare(left.toString(), right.toString());
}

} // namespace

//********************************************************
// AbstractColumn
//********************************************************

AbstractColumn::AbstractColumn(const QString &title)
    : m_title(title)
{
}

QString AbstractColumn::title() const
{
    return m_title;
}

bool AbstractColumn::setData(QVariantMap &rowData,
                             const QVariant &value,
                             int role) const
{
    Q_UNUSED(rowData)
    Q_UNUSED(value)
    Q_UNUSED(role)
    return false;
}

Qt::ItemFlags AbstractColumn::flags(const QVariantMap &rowData) const
{
    Q_UNUSED(rowData)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool AbstractColumn::lessThan(const QVariantMap &left,
                              const QVariantMap &right) const
{
    const QVariant lv = data(left, Qt::EditRole).isValid()
    ? data(left, Qt::EditRole)
    : data(left, Qt::DisplayRole);

    const QVariant rv = data(right, Qt::EditRole).isValid()
                            ? data(right, Qt::EditRole)
                            : data(right, Qt::DisplayRole);

    return compareVariants(lv, rv) < 0;
}

bool AbstractColumn::validate(const QVariantMap &rowData, const QVariant &value, int role, QString *errorText) const
{
    Q_UNUSED(rowData)
    Q_UNUSED(value)
    Q_UNUSED(role)
    Q_UNUSED(errorText)
    return true;
}

void AbstractColumn::setForegroundRule(const RowRule &rule)
{
    m_foregroundRule = rule;
}

void AbstractColumn::setBackgroundRule(const RowRule &rule)
{
    m_backgroundRule = rule;
}

void AbstractColumn::setFontRule(const RowRule &rule)
{
    m_fontRule = rule;
}

QVariant AbstractColumn::roleValue(const QVariantMap &rowData, int role) const
{
    Q_UNUSED(rowData)

    switch (role) {
    case Qt::TextAlignmentRole:
        return static_cast<int>(m_alignment);

    case Qt::ForegroundRole:
        if (m_foregroundRule)
            return m_foregroundRule(rowData);
        return m_foreground;

    case Qt::BackgroundRole:
        if (m_backgroundRule)
            return m_backgroundRule(rowData);
        return m_background;

    case Qt::FontRole:
        if (m_fontRule)
            return m_fontRule(rowData);
        return m_font;
    default:
        return {};
    }
}

void AbstractColumn::setAlignment(Qt::Alignment alignment)
{
    m_alignment = alignment;
}

void AbstractColumn::setForeground(const QBrush &brush)
{
    m_foreground = brush;
}

void AbstractColumn::setBackground(const QBrush &brush)
{
    m_background = brush;
}

void AbstractColumn::setFont(const QFont &font)
{
    m_font = font;
}

//********************************************************
// FieldColumn
//********************************************************

FieldColumn::FieldColumn(const QString &title,
                         const QString &fieldName,
                         bool editable,
                         Qt::Alignment alignment)
    : AbstractColumn(title)
    , m_fieldName(fieldName)
    , m_editable(editable)
{
    setAlignment(alignment);
}

QString FieldColumn::fieldName() const
{
    return m_fieldName;
}

QVariant FieldColumn::data(const QVariantMap &rowData, int role) const
{
    const QVariant roleSpecific = roleValue(rowData, role);
    if (roleSpecific.isValid())
        return roleSpecific;

    const QVariant value = rowData.value(m_fieldName);

    switch (role) {
    case Qt::DisplayRole:
        return formattedValue(value, role);

    case Qt::EditRole:
        return value;

    case Qt::ToolTipRole:
        return formattedValue(value, Qt::DisplayRole);

    case Qt::StatusTipRole:
        return formattedValue(value, Qt::DisplayRole);

    case Qt::DecorationRole:
        if (m_decorationRule)
            return m_decorationRule(rowData);
        break;

    default:
        break;
    }

    return {};
}

bool FieldColumn::setData(QVariantMap &rowData,
                          const QVariant &value,
                          int role) const
{
    if (!m_editable)
        return false;

    if (role != Qt::EditRole)
        return false;

    QVariant finalValue = value;

    if (m_normalizer)
        finalValue = m_normalizer(finalValue);

    QString errorText;
    if (m_validator && !m_validator(finalValue, &errorText))
        return false;

    rowData.insert(m_fieldName, finalValue);
    return true;
}

Qt::ItemFlags FieldColumn::flags(const QVariantMap &rowData) const
{
    Q_UNUSED(rowData)

    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (m_editable)
        f |= Qt::ItemIsEditable;

    return f;
}

bool FieldColumn::lessThan(const QVariantMap &left,
                           const QVariantMap &right) const
{
    return compareVariants(left.value(m_fieldName), right.value(m_fieldName)) < 0;
}

void FieldColumn::setValidator(const Validator &validator)
{
    m_validator = validator;
}

void FieldColumn::setNormalizer(const Normalizer &normalizer)
{
    m_normalizer = normalizer;
}

bool FieldColumn::validate(const QVariantMap &rowData,
                           const QVariant &value,
                           int role,
                           QString *errorText) const
{
    Q_UNUSED(rowData)

    if (role != Qt::EditRole)
        return true;

    if (!m_validator)
        return true;

    QVariant finalValue = value;
    if (m_normalizer)
        finalValue = m_normalizer(finalValue);

    return m_validator(finalValue, errorText);
}

void FieldColumn::setDisplayFormatter(const std::function<QVariant(const QVariant &)> &formatter)
{
    m_displayFormatter = formatter;
}

void FieldColumn::setDecorationRule(const std::function<QVariant (const QVariantMap &)> &rule)
{
    m_decorationRule = rule;
}

QVariant FieldColumn::formattedValue(const QVariant &value, int role) const
{
    Q_UNUSED(role)

    if (m_displayFormatter)
        return m_displayFormatter(value);

    return value;
}

//********************************************************
// CheckColumn
//********************************************************

CheckColumn::CheckColumn(const QString &title,
                         const QString &fieldName,
                         bool editable)
    : AbstractColumn(title)
    , m_fieldName(fieldName)
    , m_editable(editable)
{
    setAlignment(Qt::AlignCenter);
}

QString CheckColumn::fieldName() const
{
    return m_fieldName;
}

QVariant CheckColumn::data(const QVariantMap &rowData, int role) const
{
    const QVariant roleSpecific = roleValue(rowData, role);
    if (roleSpecific.isValid())
        return roleSpecific;

    const bool checked = rowData.value(m_fieldName).toBool();

    switch (role) {
    case Qt::CheckStateRole:
        return checked ? Qt::Checked : Qt::Unchecked;

    case Qt::EditRole:
        return checked;

    case Qt::DisplayRole:
        return {};

    case Qt::ToolTipRole:
        return checked ? QStringLiteral("Da") : QStringLiteral("Nu");

    default:
        return {};
    }
}

bool CheckColumn::setData(QVariantMap &rowData,
                          const QVariant &value,
                          int role) const
{
    if (!m_editable)
        return false;

    if (role == Qt::CheckStateRole) {
        rowData.insert(m_fieldName, value.toInt() == Qt::Checked);
        return true;
    }

    if (role == Qt::EditRole) {
        rowData.insert(m_fieldName, value.toBool());
        return true;
    }

    return false;
}

Qt::ItemFlags CheckColumn::flags(const QVariantMap &rowData) const
{
    Q_UNUSED(rowData)

    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    if (m_editable)
        f |= Qt::ItemIsEditable;

    return f;
}

bool CheckColumn::lessThan(const QVariantMap &left,
                           const QVariantMap &right) const
{
    return compareVariants(left.value(m_fieldName).toBool(),
                           right.value(m_fieldName).toBool()) < 0;
}

//********************************************************
// DateColumn
//********************************************************

DateColumn::DateColumn(const QString &title,
                       const QString &fieldName,
                       const QString &format,
                       bool editable,
                       Qt::Alignment alignment)
    : FieldColumn(title, fieldName, editable, alignment)
    , m_format(format)
{
}

QVariant DateColumn::data(const QVariantMap &rowData, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole || role == Qt::StatusTipRole) {
        const QDate d = extractDate(rowData.value(fieldName()));
        return d.isValid() ? d.toString(m_format) : QVariant();
    }

    if (role == Qt::EditRole) {
        return extractDate(rowData.value(fieldName()));
    }

    return FieldColumn::data(rowData, role);
}

bool DateColumn::lessThan(const QVariantMap &left,
                          const QVariantMap &right) const
{
    return compareVariants(extractDate(left.value(fieldName())),
                           extractDate(right.value(fieldName()))) < 0;
}

QDate DateColumn::extractDate(const QVariant &value) const
{
    if (value.metaType().id() == QMetaType::QDate)
        return value.toDate();

    if (value.metaType().id() == QMetaType::QDateTime)
        return value.toDateTime().date();

    const QDate d = QDate::fromString(value.toString(), Qt::ISODate);
    if (d.isValid())
        return d;

    return QDate::fromString(value.toString(), "dd.MM.yyyy");
}

//********************************************************
// DateTimeColumn
//********************************************************

DateTimeColumn::DateTimeColumn(const QString &title,
                               const QString &fieldName,
                               const QString &format,
                               bool editable,
                               Qt::Alignment alignment)
    : FieldColumn(title, fieldName, editable, alignment)
    , m_format(format)
{
}

QVariant DateTimeColumn::data(const QVariantMap &rowData, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole || role == Qt::StatusTipRole) {
        const QDateTime dt = extractDateTime(rowData.value(fieldName()));
        return dt.isValid() ? dt.toString(m_format) : QVariant();
    }

    if (role == Qt::EditRole) {
        return extractDateTime(rowData.value(fieldName()));
    }

    return FieldColumn::data(rowData, role);
}

bool DateTimeColumn::lessThan(const QVariantMap &left,
                              const QVariantMap &right) const
{
    return compareVariants(extractDateTime(left.value(fieldName())),
                           extractDateTime(right.value(fieldName()))) < 0;
}

QDateTime DateTimeColumn::extractDateTime(const QVariant &value) const
{
    if (value.metaType().id() == QMetaType::QDateTime)
        return value.toDateTime();

    if (value.metaType().id() == QMetaType::QDate)
        return QDateTime(value.toDate(), QTime(0, 0));

    QDateTime dt = QDateTime::fromString(value.toString(), Qt::ISODate);
    if (dt.isValid())
        return dt;

    dt = QDateTime::fromString(value.toString(), "dd.MM.yyyy HH:mm");
    return dt;
}

//********************************************************
// NumberColumn
//********************************************************

NumberColumn::NumberColumn(const QString &title,
                           const QString &fieldName,
                           int decimals,
                           bool editable,
                           Qt::Alignment alignment)
    : FieldColumn(title, fieldName, editable, alignment)
    , m_decimals(decimals)
{
}

QVariant NumberColumn::data(const QVariantMap &rowData, int role) const
{
    const QVariant raw = rowData.value(fieldName());

    if (role == Qt::DisplayRole) {
        bool ok = false;
        const double value = raw.toDouble(&ok);
        if (!ok)
            return raw;
        return QString::number(value, 'f', m_decimals);
    }

    if (role == Qt::EditRole)
        return raw;

    return FieldColumn::data(rowData, role);
}

bool NumberColumn::lessThan(const QVariantMap &left,
                            const QVariantMap &right) const
{
    return compareVariants(left.value(fieldName()).toDouble(),
                           right.value(fieldName()).toDouble()) < 0;
}

//********************************************************
// ComboColumn
//********************************************************

ComboColumn::ComboColumn(const QString &title,
                         const QString &fieldName,
                         bool editable,
                         Qt::Alignment alignment)
    : FieldColumn(title, fieldName, editable, alignment)
{
}

void ComboColumn::setNullValue(const QVariant &value)
{
    m_nullValue = value;
}

QVariant ComboColumn::nullValue() const
{
    return m_nullValue;
}

void ComboColumn::setAllowNull(bool allow)
{
    m_allowNull = allow;
}

bool ComboColumn::allowNull() const
{
    return m_allowNull;
}

void ComboColumn::setStoreNullInsteadOfNullValue(bool on)
{
    m_storeNullInsteadOfNullValue = on;
}

bool ComboColumn::storeNullInsteadOfNullValue() const
{
    return m_storeNullInsteadOfNullValue;
}

void ComboColumn::setRequiredErrorText(const QString &text)
{
    m_requiredErrorText = text;
}

QString ComboColumn::requiredErrorText() const
{
    return m_requiredErrorText;
}

bool ComboColumn::setData(QVariantMap &rowData,
                          const QVariant &value,
                          int role) const
{
    if (role != Qt::EditRole)
        return false;

    QVariant finalValue = value;

    if (storeNullInsteadOfNullValue() && finalValue == m_nullValue)
        finalValue = QVariant();

    return FieldColumn::setData(rowData, finalValue, role);
}

bool ComboColumn::validate(const QVariantMap &rowData,
                           const QVariant &value,
                           int role,
                           QString *errorText) const
{
    Q_UNUSED(rowData)

    if (role != Qt::EditRole)
        return true;

    QVariant finalValue = value;

    if (storeNullInsteadOfNullValue() && finalValue == m_nullValue)
        finalValue = QVariant();

    if (!allowNull()) {
        const bool isNullLike =
            !finalValue.isValid() ||
            finalValue.isNull() ||
            finalValue == m_nullValue ||
            finalValue.toString().trimmed().isEmpty();

        if (isNullLike) {
            if (errorText) {
                *errorText = m_requiredErrorText.isEmpty()
                ? QObject::tr("Trebuie selectată o valoare.")
                : m_requiredErrorText;
            }
            return false;
        }
    }

    return true;
}

//********************************************************
// IconColumn
//********************************************************

IconColumn::IconColumn(const QString &title, Rule rule)
    : AbstractColumn(title), m_rule(rule)
{
    setAlignment(Qt::AlignCenter);
}

QVariant IconColumn::data(const QVariantMap &rowData, int role) const
{
    if (role == Qt::DecorationRole && m_rule)
        return m_rule(rowData);

    if (role == Qt::DisplayRole)
        return {};

    if (role == Qt::TextAlignmentRole)
        return static_cast<int>(Qt::AlignCenter);

    return {};
}

//********************************************************
// CalculatedColumn
//********************************************************

CalculatedColumn::CalculatedColumn(const QString &title,
                                   ValueGetter getter,
                                   SortGetter sortGetter)
    : AbstractColumn(title)
    , m_getter(std::move(getter))
    , m_sortGetter(std::move(sortGetter))
{
}

QVariant CalculatedColumn::data(const QVariantMap &rowData, int role) const
{
    const QVariant roleSpecific = roleValue(rowData, role);
    if (roleSpecific.isValid())
        return roleSpecific;

    if (!m_getter)
        return {};

    return m_getter(rowData, role);
}

bool CalculatedColumn::lessThan(const QVariantMap &left,
                                const QVariantMap &right) const
{
    if (m_sortGetter)
        return compareVariants(m_sortGetter(left), m_sortGetter(right)) < 0;

    if (!m_getter)
        return false;

    return compareVariants(m_getter(left, Qt::EditRole), m_getter(right, Qt::EditRole)) < 0;
}

//********************************************************
// BaseAbstractModel
//********************************************************

BaseAbstractModel::BaseAbstractModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void BaseAbstractModel::registerColumn(std::unique_ptr<AbstractColumn> column)
{
    if (!column)
        return;

    const int newColumn = static_cast<int>(m_columns.size());

    beginInsertColumns(QModelIndex(), newColumn, newColumn);
    m_columns.push_back(std::move(column));
    endInsertColumns();
}

int BaseAbstractModel::columnByTitle(const QString &title) const
{
    for (int i = 0; i < static_cast<int>(m_columns.size()); ++i) {
        if (m_columns.at(i)->title() == title)
            return i;
    }
    return -1;
}

int BaseAbstractModel::columnByField(const QString &fieldName) const
{
    for (int i = 0; i < static_cast<int>(m_columns.size()); ++i) {
        if (const auto *fieldCol = dynamic_cast<FieldColumn *>(m_columns.at(i).get())) {
            if (fieldCol->fieldName() == fieldName)
                return i;
        }

        if (const auto *checkCol = dynamic_cast<CheckColumn *>(m_columns.at(i).get())) {
            if (checkCol->fieldName() == fieldName)
                return i;
        }
    }

    return -1;
}

QString BaseAbstractModel::titleByColumn(int column) const
{
    if (!isValidColumn(column))
        return {};

    return m_columns.at(column)->title();
}

void BaseAbstractModel::clear()
{
    beginResetModel();
    m_rowIndex.clear();
    m_dataHash.clear();
    endResetModel();
}

void BaseAbstractModel::setRows(const QList<QVariantMap> &rows)
{
    beginResetModel();

    m_rowIndex.clear();
    m_dataHash.clear();

    for (const QVariantMap &rowData : rows) {
        int id = 0;
        if (!hasValidId(rowData, &id))
            continue;

        if (m_dataHash.contains(id))
            continue;

        m_rowIndex.append(id);
        m_dataHash.insert(id, rowData);
    }

    endResetModel();
}

bool BaseAbstractModel::addRow(const QVariantMap &rowData)
{
    int id = 0;
    if (!hasValidId(rowData, &id))
        return false;

    if (m_dataHash.contains(id))
        return false;

    const int newRow = m_rowIndex.count();
    beginInsertRows(QModelIndex(), newRow, newRow);
    m_rowIndex.append(id);
    m_dataHash.insert(id, rowData);
    endInsertRows();

    return true;
}

bool BaseAbstractModel::insertRowData(int row, const QVariantMap &rowData)
{
    int id = 0;
    if (!hasValidId(rowData, &id))
        return false;

    if (m_dataHash.contains(id))
        return false;

    if (row < 0 || row > m_rowIndex.count())
        return false;

    beginInsertRows(QModelIndex(), row, row);
    m_rowIndex.insert(row, id);
    m_dataHash.insert(id, rowData);
    endInsertRows();

    return true;
}

bool BaseAbstractModel::updateRowById(int id, const QVariantMap &rowData)
{
    if (!m_dataHash.contains(id))
        return false;

    int newId = 0;
    if (!hasValidId(rowData, &newId))
        return false;

    const int row = rowById(id);
    if (row < 0)
        return false;

    if (newId != id) {
        if (m_dataHash.contains(newId))
            return false;

        m_dataHash.remove(id);
        m_dataHash.insert(newId, rowData);
        m_rowIndex[row] = newId;
    } else {
        m_dataHash[id] = rowData;
    }

    emitFullRowChanged(row);
    return true;
}

bool BaseAbstractModel::updateRowByIndex(int row, const QVariantMap &rowData)
{
    if (!isValidRow(row))
        return false;

    return updateRowById(m_rowIndex.at(row), rowData);
}

bool BaseAbstractModel::updateFieldById(int id, const QString &fieldName, const QVariant &value)
{
    auto it = m_dataHash.find(id);
    if (it == m_dataHash.end())
        return false;

    QVariantMap &rowData = it.value();
    rowData.insert(fieldName, value);

    const int row = rowById(id);
    if (row >= 0)
        emitFullRowChanged(row);

    return true;
}

bool BaseAbstractModel::removeRowById(int id)
{
    const int row = rowById(id);
    if (row < 0)
        return false;

    return removeRowAt(row);
}

bool BaseAbstractModel::removeRowAt(int row)
{
    if (!isValidRow(row))
        return false;

    const int id = m_rowIndex.at(row);

    beginRemoveRows(QModelIndex(), row, row);
    m_rowIndex.removeAt(row);
    m_dataHash.remove(id);
    endRemoveRows();

    return true;
}

QVariantMap BaseAbstractModel::rowDataByRow(int row) const
{
    if (!isValidRow(row))
        return {};

    return m_dataHash.value(m_rowIndex.at(row));
}

QVariantMap BaseAbstractModel::rowDataById(int id) const
{
    return m_dataHash.value(id);
}

int BaseAbstractModel::idByRow(int row) const
{
    if (!isValidRow(row))
        return -1;

    return m_rowIndex.at(row);
}

int BaseAbstractModel::rowById(int id) const
{
    return m_rowIndex.indexOf(id);
}

bool BaseAbstractModel::containsId(int id) const
{
    return m_dataHash.contains(id);
}

QList<int> BaseAbstractModel::allIds() const
{
    return m_rowIndex;
}

bool BaseAbstractModel::insertRowToDatabase(DataBase &m_db,
                                            const QString &tableName,
                                            int row,
                                            QString *errorText)
{
    QVariantMap rowData = rowDataByRow(row);
    if (rowData.isEmpty()) {
        if (errorText) *errorText = "Row invalid.";
        return false;
    }

    const int oldId = rowData.value("id").toInt();
    if (!containsId(oldId)) {
        if (errorText) *errorText = "ID-ul rândului nu există în model.";
        return false;
    }

    QSqlDatabase db = m_db.getDatabase();
    if (!db.isValid() || !db.isOpen()) {
        if (errorText) *errorText = "Database not open.";
        return false;
    }

    if (!isSqlIdentifier(tableName)) {
        if (errorText) *errorText = "Numele tabelului nu este valid.";
        return false;
    }

    QStringList fields;
    QStringList placeholders;

    for (auto it = rowData.begin(); it != rowData.end(); ++it) {
        if (it.key() == "id") // NU trimitem id
            continue;

        if (!isSqlIdentifier(it.key())) {
            if (errorText)
                *errorText = QString("Numele câmpului nu este valid: %1").arg(it.key());
            return false;
        }

        fields << db.driver()->escapeIdentifier(it.key(), QSqlDriver::FieldName);
        placeholders << ":" + it.key();
    }

    if (fields.isEmpty()) {
        if (errorText) *errorText = "Rândul nu conține câmpuri pentru inserare.";
        return false;
    }

    const QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)")
                            .arg(db.driver()->escapeIdentifier(tableName, QSqlDriver::TableName),
                                 fields.join(", "),
                                 placeholders.join(", "));

    QSqlQuery query(db);
    if (!query.prepare(sql)) {
        if (errorText) *errorText = query.lastError().text();
        return false;
    }

    for (auto it = rowData.begin(); it != rowData.end(); ++it) {
        if (it.key() == "id")
            continue;

        query.bindValue(":" + it.key(), it.value());
    }

    if (!query.exec()) {
        if (errorText) *errorText = query.lastError().text();
        return false;
    }

    const QVariant newIdValue = query.lastInsertId();
    bool idOk = false;
    const int newId = newIdValue.toInt(&idOk);
    if (!newIdValue.isValid() || !idOk || newId <= 0) {
        if (errorText)
            *errorText = "lastInsertId() invalid.";
        return false;
    }

    if (newId != oldId && containsId(newId)) {
        if (errorText)
            *errorText = QString("ID-ul generat %1 există deja în model.").arg(newId);
        return false;
    }

    rowData["id"] = newId;

    if (!updateRowById(oldId, rowData)) {
        if (errorText)
            *errorText = "Nu s-a putut actualiza ID-ul intern al modelului.";
        return false;
    }

    return true;
}

bool BaseAbstractModel::updateRowToDatabase(DataBase &m_db,
                                            const QString &tableName,
                                            int row,
                                            QString *errorText)
{
    QVariantMap rowData = rowDataByRow(row);
    if (rowData.isEmpty()) {
        if (errorText) *errorText = "Row invalid.";
        return false;
    }

    const int id = rowData.value("id").toInt();
    if (id <= 0) {
        if (errorText) *errorText = "Invalid ID.";
        return false;
    }

    QSqlDatabase db = m_db.getDatabase();
    if (!db.isValid() || !db.isOpen()) {
        if (errorText) *errorText = "Database not open.";
        return false;
    }

    if (!isSqlIdentifier(tableName)) {
        if (errorText) *errorText = "Numele tabelului nu este valid.";
        return false;
    }

    QStringList setParts;

    for (auto it = rowData.begin(); it != rowData.end(); ++it) {
        if (it.key() == "id")
            continue;

        if (!isSqlIdentifier(it.key())) {
            if (errorText)
                *errorText = QString("Numele câmpului nu este valid: %1").arg(it.key());
            return false;
        }

        setParts << QString("%1 = :%2").arg(db.driver()->escapeIdentifier(it.key(), QSqlDriver::FieldName), it.key());
    }

    if (setParts.isEmpty()) {
        if (errorText) *errorText = "Rândul nu conține câmpuri pentru actualizare.";
        return false;
    }

    const QString sql = QString("UPDATE %1 SET %2 WHERE id = :id")
                            .arg(db.driver()->escapeIdentifier(tableName, QSqlDriver::TableName), setParts.join(", "));


    QSqlQuery query(db);
    if (!query.prepare(sql)) {
        if (errorText) *errorText = query.lastError().text();
        return false;
    }

    for (auto it = rowData.begin(); it != rowData.end(); ++it) {
        if (it.key() == "id")
            continue;

        query.bindValue(":" + it.key(), it.value());
    }

    query.bindValue(":id", id);

    if (!query.exec()) {
        if (errorText) *errorText = query.lastError().text();
        return false;
    }

    return true;
}

bool BaseAbstractModel::setFieldValueInternal(int row, const QString &fieldName, const QVariant &value)
{
    if (!isValidRow(row))
        return false;

    const int id = m_rowIndex.at(row);
    auto it = m_dataHash.find(id);
    if (it == m_dataHash.end())
        return false;

    QVariantMap &rowData = it.value();

    if (rowData.value(fieldName) == value)
        return true;

    rowData[fieldName] = value;

    const int col = columnByField(fieldName);
    if (col >= 0) {
        const QModelIndex idx = index(row, col);
        if (idx.isValid()) {
            emit dataChanged(idx, idx,
                             {Qt::DisplayRole, Qt::EditRole,
                              Qt::DecorationRole,
                              Qt::ForegroundRole, Qt::BackgroundRole,
                              Qt::FontRole, Qt::TextAlignmentRole,
                              Qt::ToolTipRole, Qt::StatusTipRole});
        }
    } else {
        emit dataChanged(index(row, 0), index(row, columnCount() - 1));
    }

    return true;
}

bool BaseAbstractModel::setDeletionMark(int row, int value)
{
    return setFieldValueInternal(row, "deletionMark", value);
}

QString BaseAbstractModel::lastError() const
{
    return m_lastError;
}

void BaseAbstractModel::clearLastError()
{
    m_lastError.clear();
}

void BaseAbstractModel::sort(int column, Qt::SortOrder order)
{
    if (!isValidColumn(column))
        return;

    layoutAboutToBeChanged();

    auto &col = m_columns[column];

    std::sort(m_rowIndex.begin(), m_rowIndex.end(),
              [&](int leftId, int rightId) {
                  const QVariantMap leftRow = m_dataHash.value(leftId);
                  const QVariantMap rightRow = m_dataHash.value(rightId);

                  if (order == Qt::AscendingOrder)
                      return col->lessThan(leftRow, rightRow);

                  return col->lessThan(rightRow, leftRow);
              });

    layoutChanged();
}

int BaseAbstractModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_rowIndex.count();
}

int BaseAbstractModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return static_cast<int>(m_columns.size());
}

QVariant BaseAbstractModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    if (!isValidRow(index.row()) || !isValidColumn(index.column()))
        return {};

    const int id = m_rowIndex.at(index.row());
    const QVariantMap rowData = m_dataHash.value(id);

    return m_columns.at(index.column())->data(rowData, role);
}

bool BaseAbstractModel::setData(const QModelIndex &index,
                                const QVariant &value,
                                int role)
{
    if (!index.isValid())
        return false;

    if (!isValidRow(index.row()) || !isValidColumn(index.column()))
        return false;

    const int id = m_rowIndex.at(index.row());
    auto it = m_dataHash.find(id);
    if (it == m_dataHash.end())
        return false;

    QVariantMap &rowData = it.value();

    m_lastError.clear();

    QString errorText;
    if (!m_columns.at(index.column())->validate(rowData, value, role, &errorText)) {
        m_lastError = errorText;
        QMetaObject::invokeMethod(this,
                                  [this, idx = QPersistentModelIndex(index), msg = m_lastError]() {
                                      emit validationFailed(idx, msg);
                                  },
                                  Qt::QueuedConnection);
        return false;
    }

    if (!m_columns.at(index.column())->setData(rowData, value, role))
        return false;

    emit dataChanged(index, index,
                     {Qt::DisplayRole, Qt::EditRole, Qt::CheckStateRole,
                      Qt::ForegroundRole, Qt::BackgroundRole, Qt::FontRole,
                      Qt::TextAlignmentRole, Qt::ToolTipRole, Qt::StatusTipRole});

    return true;
}

QVariant BaseAbstractModel::headerData(int section,
                                       Qt::Orientation orientation,
                                       int role) const
{
    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Horizontal) {
        if (!isValidColumn(section))
            return {};
        return m_columns.at(section)->title();
    }

    if (orientation == Qt::Vertical)
        return section + 1;

    return {};
}

Qt::ItemFlags BaseAbstractModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (!isValidRow(index.row()) || !isValidColumn(index.column()))
        return Qt::NoItemFlags;

    const int id = m_rowIndex.at(index.row());
    const QVariantMap rowData = m_dataHash.value(id);

    return m_columns.at(index.column())->flags(rowData);
}

bool BaseAbstractModel::isValidRow(int row) const
{
    return row >= 0 && row < m_rowIndex.count();
}

bool BaseAbstractModel::isValidColumn(int column) const
{
    return column >= 0 && column < static_cast<int>(m_columns.size());
}

bool BaseAbstractModel::hasValidId(const QVariantMap &rowData, int *idOut) const
{
    if (!rowData.contains("id"))
        return false;

    bool ok = false;
    const int id = rowData.value("id").toInt(&ok);
    if (!ok)
        return false;

    if (idOut)
        *idOut = id;

    return true;
}

void BaseAbstractModel::emitFullRowChanged(int row)
{
    if (!isValidRow(row) || m_columns.empty())
        return;

    emit dataChanged(index(row, 0),
                     index(row, columnCount() - 1),
                     {Qt::DisplayRole, Qt::EditRole, Qt::CheckStateRole,
                      Qt::ForegroundRole, Qt::BackgroundRole, Qt::FontRole,
                      Qt::TextAlignmentRole, Qt::ToolTipRole, Qt::StatusTipRole});
}
