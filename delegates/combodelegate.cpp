#include "combodelegate.h"
#include <QDebug>

ComboDelegate::ComboDelegate(QString str_qry, QObject *parent) :
    QItemDelegate(parent), m_str_qry(str_qry)
{
}

QWidget *ComboDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    combo = new QComboBox(parent);
    combo->addItem(tr("<<- selectează ->>"), 0);
    QObject::connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, QOverload<int>::of(&ComboDelegate::setData));
    QSqlQuery query;
    query.prepare(m_str_qry);
    query.exec();

    while (query.next()) {
        combo->addItem(query.value(1).toString(), query.value(0)); // query.value(0) - userData (id)
    }

    return combo;
}

void ComboDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);               // get the index of the text in the combobox
    const QString currentText = index.data(Qt::EditRole).toString(); // that matches the current value of the item
    const int cbIndex = cb->findText(currentText);   
    if (cbIndex >= 0)                                                // if it is valid, adjust the combobox
       cb->setCurrentIndex(cbIndex);
}

void ComboDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    model->setData(index, cb->currentText(), Qt::EditRole);
}

void ComboDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

void ComboDelegate::setData(const int val)
{
    if (val == 0)
        return;
    emit commitData(combo);
}


