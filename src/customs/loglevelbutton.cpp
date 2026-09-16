#include "loglevelbutton.h"

LogLevelButton::LogLevelButton(QWidget *parent)
    : QToolButton(parent), group(new QButtonGroup(this))
{
    this->setText("Log Level");
    this->setPopupMode(QToolButton::InstantPopup);

    QMenu *menu = new QMenu(this);

    QStringList levels = {"Filtrare [ALL]",
                          "Filtrare [THREAD]",
                          "Filtrare [SYNC]",
                          "Filtrare [INFO]",
                          "Filtrare [CRITICAL]",
                          "Filtrare [WARNING]",
                          "Filtrare [DEBUG]"};

    for (const QString &level : levels) {
        QRadioButton *radio   = new QRadioButton(level);
        QWidgetAction *action = new QWidgetAction(menu);
        action->setDefaultWidget(radio);
        menu->addAction(action);
        group->addButton(radio);

        // închide meniul la alegere
        connect(radio, &QRadioButton::toggled, this, [=, this](bool checked){
            if (checked)
                emit selectedLevel(radio->text());
                menu->close();  // închide popup-ul
        });
    }

    // Selectează implicit "all"
    if (auto btn = group->buttons().value(0))
        btn->setChecked(true);

    this->setMenu(menu);

    menu->setStyleSheet(R"(
        QMenu {
            background-color: #2b2b2b;
            color: white;
            border: 1px solid #555;
            border-radius: 4px;
        }
        QMenu::item {
            padding: 4px 20px;
            background-color: transparent;
        }
        QMenu::item:selected {
            background-color: #c0392b;
            color: white;
        }
        QRadioButton {
            color: white;
            background: transparent;
        }
    )");

    if (globals().isSystemThemeDark)
        this->setStyleSheet(R"(
            QToolButton
            {
                border: 1px solid rgba(255, 255, 255, 0.6);
                border-radius: 8px;
                background-color: #2b2b2b;
                color: #ffffff;
                font-size: 13px;
                padding: 4px 4px;
            }
            QToolButton:hover
            {
                background-color: #3b3b3b;
                border: 1px solid rgba(255, 255, 255, 0.8);
            }
            QToolButton:pressed
            {
                background-color: #4b4b4b;
            }
        )");
    else
        this->setStyleSheet(R"(
            QToolButton
            {
                border: 1px solid rgba(0, 0, 0, 0.1);
                border-radius: 8px;
                background-color: #f1f1f1;
                color: #000000;
                font-size: 13px;
                padding: 4px 4px;
            }
            QToolButton:hover
            {
                background-color: #e0e0e0;
            }
            QToolButton:pressed
            {
                background-color: #d0d0d0;
            }
        )");
}
