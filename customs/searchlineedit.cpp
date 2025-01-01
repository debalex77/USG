#include "searchlineedit.h"

SearchLineEdit::SearchLineEdit(QWidget *parent) : QLineEdit(parent)
{
    setupUI();
    setupMenu();
}

SearchLineEdit::~SearchLineEdit()
{

}

void SearchLineEdit::setupUI()
{
    // Creează butonul de căutare
    searchButton = new QPushButton(this);

    // Configurăm iconul
    QIcon icon(":/img/oxygen/page-zoom.png");
    searchButton->setIcon(icon);

    // Calculăm înălțimea QLineEdit și ajustăm dimensiunea butonului
    int buttonHeight = height() - 4;  // 2px margin sus și jos
    int buttonWidth = buttonHeight + 2;  // Puțin mai lat decât înalt pentru aspect

    searchButton->setFixedSize(buttonWidth, buttonHeight);
    searchButton->setIconSize(QSize(buttonHeight - 4, buttonHeight - 4));  // Icon puțin mai mic decât butonul

    // Stilizare cu padding de 4px
    searchButton->setStyleSheet(QString(
                                    "QPushButton {"
                                    "    border: none;"
                                    "    background: transparent;"
                                    "    margin: 0px;"
                                    "    padding: %1px;"
                                    "}"
                                    "QPushButton:hover {"
                                    "    background-color: rgba(0,0,0,10);"
                                    "}"
                                    "QPushButton:pressed {"
                                    "    background-color: rgba(0,0,0,20);"
                                    "}"
                                    ).arg(PADDING));

    searchButton->setCursor(Qt::PointingHandCursor);

    // Poziționare inițială
    updateButtonPosition();
}

void SearchLineEdit::setupMenu()
{
    searchMenu = new QMenu(this);

    // Adaugă opțiuni în meniu
    QAction *searchByCode = new QAction(tr("Caut\304\203 dup\304\203 cod"), this);
    searchByCode->setData("code");  // atașăm un identificator

    QAction *searchByName = new QAction(tr("Caut\304\203 dup\304\203 denumire"), this);
    searchByName->setData("name");  // atașăm un identificator

    searchMenu->addAction(searchByCode);
    searchMenu->addAction(searchByName);

    // Conectează butonul la meniu
    searchButton->setMenu(searchMenu);

    // Conectează acțiunile meniului
    connect(searchByCode, &QAction::triggered, this, &SearchLineEdit::onSearchOptionSelected);
    connect(searchByName, &QAction::triggered, this, &SearchLineEdit::onSearchOptionSelected);
}

void SearchLineEdit::updateButtonPosition()
{
    if (searchButton) {
        // Poziționăm butonul la stânga cu padding
        int buttonY = (height() - searchButton->height()) / 2;
        searchButton->move(PADDING, buttonY);

        // Setăm marginea pentru text, incluzând padding-ul
        setTextMargins(searchButton->width() + PADDING * 2, 0, 0, 0);
    }
}

void SearchLineEdit::setSearchOptionSelected(QString typeSearch)
{
    m_typeSearch = typeSearch;
    if (m_typeSearch == "code")
        setPlaceholderText(tr("Caut\304\203 dup\304\203 cod"));
    else if (m_typeSearch == "name")
        setPlaceholderText(tr("Caut\304\203 dup\304\203 denumire"));
    else
        setPlaceholderText("Unknow !!!");
}

QString SearchLineEdit::getSearchOptionSelected()
{
    return m_typeSearch;
}

void SearchLineEdit::onSearchOptionSelected()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (action) {
        // set placeholder
        setPlaceholderText(action->text());

        QString type = action->data().toString(); // identificator = code || name
        m_typeSearch = type;
        emit setSearchByCodeName(m_typeSearch);
    }
}

void SearchLineEdit::resizeEvent(QResizeEvent *event)
{
    QLineEdit::resizeEvent(event);

    // Actualizăm dimensiunea butonului la resize, luând în considerare padding-ul
    int buttonHeight = height() - 4;
    int buttonWidth = buttonHeight + 2;
    searchButton->setFixedSize(buttonWidth + PADDING * 2, buttonHeight);  // Adăugăm padding la lățime
    searchButton->setIconSize(QSize(buttonHeight - 4, buttonHeight - 4));

    updateButtonPosition();
}


