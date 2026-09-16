#ifndef BASEABSTRACTMODEL_H
#define BASEABSTRACTMODEL_H

#include <QAbstractTableModel>
#include <QBrush>
#include <QDate>
#include <QDateTime>
#include <QFont>
#include <QHash>
#include <QVector>
#include <QIcon>
#include <QVariantMap>
#include <functional>
#include <memory>

#include <data/database.h>

class AbstractColumn
{
public:
    using RowRule = std::function<QVariant(const QVariantMap &)>;

    explicit AbstractColumn(const QString &title);
    virtual ~AbstractColumn() = default;

    QString title() const;

    virtual QVariant data(const QVariantMap &rowData,
                          int role = Qt::DisplayRole) const = 0;

    virtual bool setData(QVariantMap &rowData,
                         const QVariant &value,
                         int role = Qt::EditRole) const;

    virtual Qt::ItemFlags flags(const QVariantMap &rowData) const;

    virtual bool lessThan(const QVariantMap &left,
                          const QVariantMap &right) const;

    virtual bool validate(const QVariantMap &rowData,
                          const QVariant &value,
                          int role,
                          QString *errorText) const;

    void setForegroundRule(const RowRule &rule);
    void setBackgroundRule(const RowRule &rule);
    void setFontRule(const RowRule &rule);

protected:
    QVariant roleValue(const QVariantMap &rowData, int role) const;

    void setAlignment(Qt::Alignment alignment);
    void setForeground(const QBrush &brush);
    void setBackground(const QBrush &brush);
    void setFont(const QFont &font);

private:
    QString m_title;
    Qt::Alignment m_alignment {Qt::AlignLeft | Qt::AlignVCenter};
    QVariant m_foreground;
    QVariant m_background;
    QVariant m_font;

    RowRule m_foregroundRule;
    RowRule m_backgroundRule;
    RowRule m_fontRule;
};

//********************************************************

class FieldColumn : public AbstractColumn
{
public:
    using Validator = std::function<bool(const QVariant &value, QString *errorText)>;
    using Normalizer = std::function<QVariant(const QVariant &value)>;

    explicit FieldColumn(const QString &title,
                         const QString &fieldName,
                         bool editable = false,
                         Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter);

    QString fieldName() const;

    QVariant data(const QVariantMap &rowData,
                  int role = Qt::DisplayRole) const override;

    bool setData(QVariantMap &rowData,
                 const QVariant &value,
                 int role = Qt::EditRole) const override;

    Qt::ItemFlags flags(const QVariantMap &rowData) const override;

    bool lessThan(const QVariantMap &left,
                  const QVariantMap &right) const override;

    void setValidator(const Validator &validator);
    void setNormalizer(const Normalizer &normalizer);
    bool validate(const QVariantMap &rowData,
                               const QVariant &value,
                               int role,
                  QString *errorText) const override;

    void setDisplayFormatter(const std::function<QVariant(const QVariant &)> &formatter);
    void setDecorationRule(const std::function<QVariant(const QVariantMap &)> &rule);

private:
    QVariant formattedValue(const QVariant &value, int role) const;

private:
    QString m_fieldName;
    bool m_editable {false};
    Validator m_validator;
    Normalizer m_normalizer;

    std::function<QVariant(const QVariant &)> m_displayFormatter;
    std::function<QVariant(const QVariantMap &)> m_decorationRule;

};

//********************************************************

class CheckColumn : public AbstractColumn
{
public:
    explicit CheckColumn(const QString &title,
                         const QString &fieldName,
                         bool editable = true);

    QString fieldName() const;

    QVariant data(const QVariantMap &rowData,
                  int role = Qt::DisplayRole) const override;

    bool setData(QVariantMap &rowData,
                 const QVariant &value,
                 int role = Qt::EditRole) const override;

    Qt::ItemFlags flags(const QVariantMap &rowData) const override;

    bool lessThan(const QVariantMap &left,
                  const QVariantMap &right) const override;

private:
    QString m_fieldName;
    bool m_editable {true};
};

//********************************************************

class DateColumn : public FieldColumn
{
public:
    explicit DateColumn(const QString &title,
                        const QString &fieldName,
                        const QString &format = "dd.MM.yyyy",
                        bool editable = false,
                        Qt::Alignment alignment = Qt::AlignCenter);

    QVariant data(const QVariantMap &rowData,
                  int role = Qt::DisplayRole) const override;

    bool lessThan(const QVariantMap &left,
                  const QVariantMap &right) const override;

private:
    QDate extractDate(const QVariant &value) const;

private:
    QString m_format;
};

//********************************************************

class DateTimeColumn : public FieldColumn
{
public:
    explicit DateTimeColumn(const QString &title,
                            const QString &fieldName,
                            const QString &format = "dd.MM.yyyy HH:mm",
                            bool editable = false,
                            Qt::Alignment alignment = Qt::AlignCenter);

    QVariant data(const QVariantMap &rowData,
                  int role = Qt::DisplayRole) const override;

    bool lessThan(const QVariantMap &left,
                  const QVariantMap &right) const override;

private:
    QDateTime extractDateTime(const QVariant &value) const;

private:
    QString m_format;
};

//********************************************************

class NumberColumn : public FieldColumn
{
public:
    explicit NumberColumn(const QString &title,
                          const QString &fieldName,
                          int decimals = 2,
                          bool editable = false,
                          Qt::Alignment alignment = Qt::AlignRight | Qt::AlignVCenter);

    QVariant data(const QVariantMap &rowData,
                  int role = Qt::DisplayRole) const override;

    bool lessThan(const QVariantMap &left,
                  const QVariantMap &right) const override;

private:
    int m_decimals {2};
};

//********************************************************

class ComboColumn : public FieldColumn
{
public:
    explicit ComboColumn(const QString &title,
                         const QString &fieldName,
                         bool editable = true,
                         Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter);

    void setNullValue(const QVariant &value);
    QVariant nullValue() const;

    void setAllowNull(bool allow);
    bool allowNull() const;

    void setStoreNullInsteadOfNullValue(bool on);
    bool storeNullInsteadOfNullValue() const;

    void setRequiredErrorText(const QString &text);
    QString requiredErrorText() const;

    bool setData(QVariantMap &rowData,
                 const QVariant &value,
                 int role = Qt::EditRole) const override;

    bool validate(const QVariantMap &rowData,
                  const QVariant &value,
                  int role,
                  QString *errorText) const override;

private:
    QVariant m_nullValue {0};
    bool m_allowNull {true};
    bool m_storeNullInsteadOfNullValue {false};
    QString m_requiredErrorText;
};

//********************************************************

class IconColumn : public AbstractColumn
{
public:
    using Rule = std::function<QIcon(const QVariantMap&)>;

    IconColumn(const QString &title, Rule rule);

    QVariant data(const QVariantMap &rowData, int role) const override;

private:
    Rule m_rule;
};

//********************************************************

class CalculatedColumn : public AbstractColumn
{
public:
    using ValueGetter = std::function<QVariant(const QVariantMap &, int role)>;
    using SortGetter  = std::function<QVariant(const QVariantMap &)>;

    explicit CalculatedColumn(const QString &title,
                              ValueGetter getter,
                              SortGetter sortGetter = SortGetter());

    QVariant data(const QVariantMap &rowData,
                  int role = Qt::DisplayRole) const override;

    bool lessThan(const QVariantMap &left,
                  const QVariantMap &right) const override;

private:
    ValueGetter m_getter;
    SortGetter m_sortGetter;
};

//********************************************************

class BaseAbstractModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit BaseAbstractModel(QObject *parent = nullptr);
    ~BaseAbstractModel() override = default;

    void registerColumn(std::unique_ptr<AbstractColumn> column);

    int columnByTitle(const QString &title) const;
    int columnByField(const QString &fieldName) const;
    QString titleByColumn(int column) const;

    void clear();
    void setRows(const QList<QVariantMap> &rows);

    bool addRow(const QVariantMap &rowData);
    bool insertRowData(int row, const QVariantMap &rowData);
    bool updateRowById(int id, const QVariantMap &rowData);
    bool updateRowByIndex(int row, const QVariantMap &rowData);
    bool updateFieldById(int id, const QString &fieldName, const QVariant &value);
    bool removeRowById(int id);
    bool removeRowAt(int row);

    QVariantMap rowDataByRow(int row) const;
    QVariantMap rowDataById(int id) const;

    int idByRow(int row) const;
    int rowById(int id) const;
    bool containsId(int id) const;
    QList<int> allIds() const;

    bool insertRowToDatabase(DataBase &m_db,
                             const QString &tableName,
                             int row,
                             QString *errorText);

    bool updateRowToDatabase(DataBase &m_db,
                             const QString &tableName,
                             int row,
                             QString *errorText);

    bool setFieldValueInternal(int row,
                               const QString &fieldName,
                               const QVariant &value);

    bool setDeletionMark(int row, int value); // apel direct fără să depindă de flags-ul UI

    QString lastError() const;
    void clearLastError();

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index,
                 const QVariant &value,
                 int role = Qt::EditRole) override;

    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

signals:
    void validationFailed(const QPersistentModelIndex &index, const QString &errorText);

private:
    bool isValidRow(int row) const;
    bool isValidColumn(int column) const;
    bool hasValidId(const QVariantMap &rowData, int *idOut = nullptr) const;

    void emitFullRowChanged(int row);

private:
    QList<int> m_rowIndex;
    QHash<int, QVariantMap> m_dataHash;
    std::vector<std::unique_ptr<AbstractColumn>> m_columns;

    QString m_lastError;
};

#endif // BASEABSTRACTMODEL_H
