#include "doublespinboxdelegate.h"

DoubleSpinBoxDelegate::DoubleSpinBoxDelegate(double min, double max, double step, int precision, QObject *parent)
    : QItemDelegate{parent}, m_min(min), m_max(max), m_step(step), m_precision(precision)
{}


QWidget *DoubleSpinBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    editor->setMinimum(m_min);
    editor->setMaximum(m_max);
    editor->setDecimals(m_precision);
    editor->setSingleStep(m_step);
    editor->installEventFilter(const_cast<DoubleSpinBoxDelegate*>(this));
    return editor;
}

void DoubleSpinBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    double value = index.model()->data(index, Qt::EditRole).toDouble();
    QDoubleSpinBox *dsb = static_cast<QDoubleSpinBox*>(editor);
    dsb->setValue(value);
}

void DoubleSpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QDoubleSpinBox *dsb = static_cast<QDoubleSpinBox*>(editor);
    dsb->interpretText();
    double value = dsb->value();
    model->setData(index, value);
}

void DoubleSpinBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)

    editor->setGeometry(option.rect);
}
