#ifndef CENTERICONDELEGATE_H
#define CENTERICONDELEGATE_H

#include <QStyledItemDelegate>
#include <QApplication>
#include <QPainter>

class CenterIconDelegate : public QStyledItemDelegate
{
public:
    explicit CenterIconDelegate(QObject *parent = nullptr);

public:
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

#endif // CENTERICONDELEGATE_H
