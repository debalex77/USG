#include "lineeditcustom.h"

LineEditCustom::LineEditCustom(QWidget *parent) : QLineEdit(parent)
{
    QAction *action_add_item = this->addAction(QIcon(":/img/add_x32.png"), QLineEdit::ActionPosition::TrailingPosition);
    action_add_item->setToolTip(tr("Adaug\304\203 \303\256n lista cu \310\231abloane"));
    connect(action_add_item, &QAction::triggered, this, &LineEditCustom::onClickAddItem);

    QAction *action_open_list = this->addAction(QIcon(":/img/select.png"), QLineEdit::ActionPosition::TrailingPosition);
    action_open_list->setToolTip(tr("Selecteaz\304\203 din lista cu \310\231abloane"));
    connect(action_open_list, &QAction::triggered, this, &LineEditCustom::onClickSelect);
}

LineEditCustom::~LineEditCustom()
{

}
