#include "centericondelegate.h"

CenterIconDelegate::CenterIconDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{}


void CenterIconDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    QVariant value = index.data(Qt::DecorationRole);

    if (value.canConvert<QIcon>()) {
        QIcon icon = qvariant_cast<QIcon>(value);

        painter->save();

        // desenare fundal/selectie standard
        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);

        // eliminam textul si iconul standard ca sa le desenam noi
        opt.text.clear();
        opt.icon = QIcon();

        const QWidget *widget = opt.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

        QSize iconSize = opt.decorationSize.isValid()
                             ? opt.decorationSize
                             : QSize(16, 16);

        QPixmap pixmap = icon.pixmap(iconSize);

        QRect r = option.rect;
        QPoint p(r.x() + (r.width() - pixmap.width()) / 2,
                 r.y() + (r.height() - pixmap.height()) / 2);

        painter->drawPixmap(p, pixmap);
        painter->restore();
        return;
    }

    QStyledItemDelegate::paint(painter, option, index);
}
