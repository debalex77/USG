#ifndef COMBODELEGATE_H
#define COMBODELEGATE_H

#include <QStyledItemDelegate>
#include <QSqlDatabase>
#include <QString>

class QComboBox;

class ComboDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ComboDelegate(QObject *parent = nullptr);
    ~ComboDelegate() override = default;

    void setQuery(const QString &sqlQuery);
    QString query() const;

    void setIdColumn(int column);
    int idColumn() const;

    void setTextColumn(int column);
    int textColumn() const;

    void setAllowNullItem(bool value);
    bool allowNullItem() const;

    void setNullText(const QString &text);
    QString nullText() const;

    void setNullValue(const QVariant &value);
    QVariant nullValue() const;

    void setMaxVisibleItems(int count);
    void setStyleCombo(QString str);
    void setMinimWidthPopup(int width);

    void setConnectionName(const QString &connectionName);
    QString connectionName() const;

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;

    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

    QString displayText(const QVariant &value,
                        const QLocale &locale) const override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void loadItems(QComboBox *combo) const;
    QString textForValue(const QVariant &value) const;
    QSqlDatabase database() const;

private:
    QString  m_sqlQuery;
    int      m_idColumn       = 0;
    int      m_textColumn     = 1;
    bool     m_allowNullItem  = true;
    QString  m_nullText       = QObject::tr("<<- selectează ->>");
    QVariant m_nullValue      = 0;
    QString  m_connectionName;
    int m_maxVisibleItems     = 0;
    QString  m_styleCombo;
    int      m_minimWidthPopup = 0;
};

#endif // COMBODELEGATE_H
