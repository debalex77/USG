#include "lineeditpassword.h"

LineEditPassword::LineEditPassword(QWidget *parent) : QLineEdit(parent)
{
    action_hideShowPassword = this->addAction(QIcon(":/img/lock.png"), QLineEdit::ActionPosition::TrailingPosition);
    connect(action_hideShowPassword, &QAction::triggered, this, &LineEditPassword::onClickedButton);

    show_passwd = false;
    this->setEchoMode(QLineEdit::Password);
}

LineEditPassword::~LineEditPassword()
{

}

void LineEditPassword::onClickedButton()
{
    if (show_passwd) {
        action_hideShowPassword->setIcon(QIcon(":/img/lock.png"));
        show_passwd = false;
        this->setEchoMode(QLineEdit::Password);
    } else {
        action_hideShowPassword->setIcon(QIcon(":/img/unlock.png"));
        show_passwd = true;
        this->setEchoMode(QLineEdit::Normal);
    }
}


