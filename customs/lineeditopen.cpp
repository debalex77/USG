#include "lineeditopen.h"

LineEditOpen::LineEditOpen(QWidget *parent) : QLineEdit(parent)
{
    action_open = this->addAction(QIcon(":/img/open-search.png"), QLineEdit::ActionPosition::TrailingPosition);
    connect(action_open, &QAction::triggered, this, &LineEditOpen::onClickedButton);

    // Inițial ascunde acțiunea dacă textul este gol
    action_open->setVisible(!this->text().isEmpty());

    // Conectează semnalul textChanged la o funcție care schimbă vizibilitatea acțiunii
    connect(this, &QLineEdit::textChanged, this, &LineEditOpen::onTextChanged);
}

LineEditOpen::~LineEditOpen()
{

}

void LineEditOpen::onTextChanged(const QString &text)
{
    action_open->setVisible(! text.isEmpty());
}
