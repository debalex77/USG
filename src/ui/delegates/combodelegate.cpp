#include "combodelegate.h"

#include <QComboBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QAbstractItemModel>
#include <QApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QLineEdit>

ComboDelegate::ComboDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void ComboDelegate::setQuery(const QString &sqlQuery)
{
    m_sqlQuery = sqlQuery;
}

QString ComboDelegate::query() const
{
    return m_sqlQuery;
}

void ComboDelegate::setIdColumn(int column)
{
    m_idColumn = column;
}

int ComboDelegate::idColumn() const
{
    return m_idColumn;
}

void ComboDelegate::setTextColumn(int column)
{
    m_textColumn = column;
}

int ComboDelegate::textColumn() const
{
    return m_textColumn;
}

void ComboDelegate::setAllowNullItem(bool value)
{
    m_allowNullItem = value;
}

bool ComboDelegate::allowNullItem() const
{
    return m_allowNullItem;
}

void ComboDelegate::setNullText(const QString &text)
{
    m_nullText = text;
}

QString ComboDelegate::nullText() const
{
    return m_nullText;
}

void ComboDelegate::setNullValue(const QVariant &value)
{
    m_nullValue = value;
}

QVariant ComboDelegate::nullValue() const
{
    return m_nullValue;
}

void ComboDelegate::setMaxVisibleItems(int count)
{
    m_maxVisibleItems = count;
}

void ComboDelegate::setStyleCombo(QString str)
{
    m_styleCombo = str;
}

void ComboDelegate::setMinimWidthPopup(int width)
{
    m_minimWidthPopup = width;
}

void ComboDelegate::setConnectionName(const QString &connectionName)
{
    m_connectionName = connectionName;
}

QString ComboDelegate::connectionName() const
{
    return m_connectionName;
}

void ComboDelegate::setEnterMovesToNextItem(bool enabled)
{
    m_enterMovesToNextItem = enabled;
}

bool ComboDelegate::enterMovesToNextItem() const
{
    return m_enterMovesToNextItem;
}

QSqlDatabase ComboDelegate::database() const
{
    if (!m_connectionName.trimmed().isEmpty()) {
        if (QSqlDatabase::contains(m_connectionName))
            return QSqlDatabase::database(m_connectionName);
    }

    return QSqlDatabase::database();
}

void ComboDelegate::loadItems(QComboBox *combo) const
{
    if (!combo)
        return;

    combo->clear();

    if (m_allowNullItem)
        combo->addItem(m_nullText, m_nullValue);

    if (m_sqlQuery.trimmed().isEmpty())
        return;

    QSqlDatabase db = database();
    if (!db.isValid() || !db.isOpen()) {
        qWarning() << "ComboDelegate: baza de date nu este validă sau deschisă.";
        return;
    }

    QSqlQuery query(db);
    if (!query.prepare(m_sqlQuery)) {
        qWarning() << "ComboDelegate::loadItems prepare error:"
                   << query.lastError().text()
                   << "\nSQL:" << m_sqlQuery;
        return;
    }

    if (!query.exec()) {
        qWarning() << "ComboDelegate::loadItems exec error:"
                   << query.lastError().text()
                   << "\nSQL:" << m_sqlQuery;
        return;
    }

    while (query.next()) {
        const QVariant idValue   = query.value(m_idColumn);
        const QString  textValue = query.value(m_textColumn).toString();
        combo->addItem(textValue, idValue);
    }
}

QWidget *ComboDelegate::createEditor(QWidget *parent,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    auto *combo = new QComboBox(parent);
    combo->setEditable(false);
    combo->setFrame(false);
    if (m_maxVisibleItems > 0)
        combo->setMaxVisibleItems(m_maxVisibleItems);
    if (!m_styleCombo.isEmpty())
        combo->setStyleSheet(m_styleCombo);

    loadItems(combo);

    if (m_minimWidthPopup > 0)
        combo->view()->setMinimumWidth(m_minimWidthPopup);

    if (m_maxVisibleItems > 0) {
        combo->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        combo->view()->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }

    combo->installEventFilter(const_cast<ComboDelegate*>(this));
    if (m_enterMovesToNextItem) {
        if (combo->lineEdit())
            combo->lineEdit()->installEventFilter(const_cast<ComboDelegate*>(this));
        combo->view()->installEventFilter(const_cast<ComboDelegate*>(this));
        combo->view()->viewport()->installEventFilter(const_cast<ComboDelegate*>(this));
    }

    connect(combo,
            QOverload<int>::of(&QComboBox::activated),
            this,
            [this, combo](int) {
                auto *that = const_cast<ComboDelegate*>(this);
                emit that->commitData(combo);
            });

    return combo;
}

void ComboDelegate::setEditorData(QWidget *editor,
                                  const QModelIndex &index) const
{
    auto *combo = qobject_cast<QComboBox *>(editor);
    if (!combo)
        return;

    const QVariant currentValue = index.model()->data(index, Qt::EditRole);

    QSignalBlocker blocker(combo);

    int comboIndex = combo->findData(currentValue);

    if (comboIndex < 0 && currentValue.isValid()) {
        const QString textValue = currentValue.toString();
        comboIndex = combo->findText(textValue);
    }

    if (comboIndex < 0)
        comboIndex = 0;

    combo->setCurrentIndex(comboIndex);
}

void ComboDelegate::setModelData(QWidget *editor,
                                 QAbstractItemModel *model,
                                 const QModelIndex &index) const
{
    auto *combo = qobject_cast<QComboBox *>(editor);
    if (!combo || !model)
        return;

    const QVariant selectedValue = combo->currentData();

    model->setData(index, selectedValue, Qt::EditRole);
}

void ComboDelegate::updateEditorGeometry(QWidget *editor,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    Q_UNUSED(index)

    if (editor)
        editor->setGeometry(option.rect);
}

QString ComboDelegate::textForValue(const QVariant &value) const
{
    if (!value.isValid() || value.isNull())
        return m_allowNullItem ? m_nullText : QString();

    if (m_allowNullItem && value == m_nullValue)
        return m_nullText;

    if (m_sqlQuery.trimmed().isEmpty())
        return value.toString();

    QSqlDatabase db = database();
    if (!db.isValid() || !db.isOpen())
        return value.toString();

    QSqlQuery query(db);
    if (!query.prepare(m_sqlQuery)) {
        qWarning() << "ComboDelegate::textForValue prepare error:"
                   << query.lastError().text();
        return value.toString();
    }

    if (!query.exec()) {
        qWarning() << "ComboDelegate::textForValue exec error:"
                   << query.lastError().text();
        return value.toString();
    }

    while (query.next()) {
        const QVariant idValue = query.value(m_idColumn);
        if (idValue == value)
            return query.value(m_textColumn).toString();
    }

    return value.toString();
}

QString ComboDelegate::displayText(const QVariant &value,
                                   const QLocale &locale) const
{
    Q_UNUSED(locale)
    return textForValue(value);
}

bool ComboDelegate::eventFilter(QObject *obj, QEvent *event)
{
    QComboBox *combo = qobject_cast<QComboBox *>(obj);
    if (!combo && m_enterMovesToNextItem) {
        QWidget *widget = qobject_cast<QWidget *>(obj);
        while (widget && !combo) {
            combo = qobject_cast<QComboBox *>(widget);
            widget = widget->parentWidget();
        }
    }
    if (combo && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (m_enterMovesToNextItem
            && (keyEvent->key() == Qt::Key_Return
                || keyEvent->key() == Qt::Key_Enter)) {
            combo->hidePopup();
            emit commitData(combo);
            emit closeEditor(combo, QAbstractItemDelegate::EditNextItem);
            return true;
        }
        if (keyEvent->key() == Qt::Key_Escape) {
            auto *that = const_cast<ComboDelegate*>(this);
            emit that->closeEditor(combo, QAbstractItemDelegate::RevertModelCache);
            return true;
        }
    }

    // Cu m_enterMovesToNextItem filtrul este instalat și pe lineEdit/popup;
    // tratarea implicită (FocusOut, Tab -> commitData/closeEditor) este
    // valabilă doar pentru editorul înregistrat în view, adică pentru combo.
    return qobject_cast<QComboBox *>(obj)
           && QStyledItemDelegate::eventFilter(obj, event);
}
