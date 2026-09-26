#include "toolbarcustom.h"

ToolBarCustom::ToolBarCustom(QWidget *parent, Options options)
    : QWidget{parent}
    , m_options(options)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setSpacing(2);
    m_layout->setContentsMargins(0,0,0,0);

    if (m_options.testFlag(ButtonSelect)) {
        btnSelect = createBtn(":/img/actions/select.png",
                              QKeySequence(Qt::Key_Return),
                              tr("Selecteaza"));
        m_layout->addWidget(btnSelect);
        m_layout->addSpacing(10);

        connect(btnSelect, &QToolButton::clicked,
                this, &ToolBarCustom::selectDoc, Qt::UniqueConnection);
    }

    if (m_options.testFlag(AddEditDelete)) {
        btnAdd    = createBtn(":/img/toolBar/add.png", QKeySequence(Qt::Key_Insert));
        btnEdit   = createBtn(":/img/toolBar/edit.png", QKeySequence(Qt::Key_F2));
        btnDelete = createBtn(":/img/toolBar/delete.png", QKeySequence(Qt::Key_Delete));

        m_layout->addWidget(btnAdd);
        m_layout->addWidget(btnEdit);
        m_layout->addWidget(btnDelete);

        connect(btnAdd, &QToolButton::clicked,
                this, &ToolBarCustom::addDoc, Qt::UniqueConnection);
        connect(btnEdit, &QToolButton::clicked,
                this, &ToolBarCustom::editDoc, Qt::UniqueConnection);
        connect(btnDelete, &QToolButton::clicked,
                this, &ToolBarCustom::deleteDoc, Qt::UniqueConnection);
    }

    if (m_options.testFlag(Filter)) {
        btnAddFilter    = createBtn(":/img/toolBar/filter_add.png", QKeySequence(Qt::CTRL | Qt::Key_F1));
        btnSetFilter    = createBtn(":/img/toolBar/filter_set.png", QKeySequence(Qt::CTRL | Qt::Key_F2));
        btnDeleteFilter = createBtn(":/img/toolBar/filter_delete.png", QKeySequence(Qt::CTRL | Qt::Key_F3));

        m_layout->addSpacing(10);
        m_layout->addWidget(btnAddFilter);
        m_layout->addWidget(btnSetFilter);
        m_layout->addWidget(btnDeleteFilter);

        connect(btnAddFilter, &QToolButton::clicked,
                this, &ToolBarCustom::addFilter, Qt::UniqueConnection);
        connect(btnSetFilter, &QToolButton::clicked,
                this, &ToolBarCustom::setFilter, Qt::UniqueConnection);
        connect(btnDeleteFilter, &QToolButton::clicked,
                this, &ToolBarCustom::deleteFilter, Qt::UniqueConnection);
    }

    if (m_options.testFlag(UpdateColumn)) {
        btnUpdate     = createBtn(":/img/toolBar/update.png", QKeySequence(Qt::Key_F5));
        btnHideColumn = createBtn(":/img/toolBar/column.png", QKeySequence(Qt::CTRL | Qt::Key_H));

        m_layout->addSpacing(10);
        m_layout->addWidget(btnUpdate);
        m_layout->addWidget(btnHideColumn);

        connect(btnUpdate, &QToolButton::clicked,
                this, &ToolBarCustom::updateTable, Qt::UniqueConnection);
        connect(btnHideColumn, &QToolButton::clicked,
                this, &ToolBarCustom::hideShowColumn, Qt::UniqueConnection);
    }

    if (m_options.testFlag(PrintEmail)) {
        btnPrint  = createBtn(":/img/toolBar/print.png", QKeySequence(Qt::CTRL | Qt::Key_P));
        btnEmail  = createBtn(":/img/toolBar/email.png", QKeySequence(Qt::CTRL | Qt::Key_M));

        m_layout->addSpacing(10);
        m_layout->addWidget(btnPrint);
        m_layout->addWidget(btnEmail);

        connect(btnPrint, &QToolButton::clicked,
                this, &ToolBarCustom::printDoc, Qt::UniqueConnection);
        connect(btnEmail, &QToolButton::clicked,
                this, &ToolBarCustom::sendEmail, Qt::UniqueConnection);
    }

    if (m_options.testFlag(ReportViewTab)) {
        btnReport = createBtn(":/img/documents/reportEcho.png", QKeySequence(Qt::CTRL | Qt::Key_R), tr("Raport"));
        btnView   = createBtn(":/img/journal/view_table.png", QKeySequence(Qt::CTRL | Qt::Key_T), tr("Tabela"));

        m_layout->addSpacing(10);
        m_layout->addWidget(btnReport);
        m_layout->addWidget(btnView);

        connect(btnReport, &QToolButton::clicked,
                this, &ToolBarCustom::createReport, Qt::UniqueConnection);
        connect(btnView, &QToolButton::clicked,
                this, &ToolBarCustom::viewTabOrder, Qt::UniqueConnection);
    }

    if (m_options.testFlag(PeriodSearch)) {
        btnPeriod = createBtn(":/img/toolBar/calendar.png", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P));
        btnSearch = createBtn(":/img/toolBar/search_pacients.png", QKeySequence(Qt::CTRL | Qt::Key_F));

        txtPeriod = createLabel("Period:", "color: rgb(0, 191, 255);");

        m_layout->addWidget(btnPeriod);
        m_layout->addWidget(btnSearch);
        m_layout->addStretch();
        m_layout->addWidget(txtPeriod);

        connect(btnPeriod, &QToolButton::clicked,
                this, &ToolBarCustom::openPeriod, Qt::UniqueConnection);
        connect(btnSearch, &QToolButton::clicked,
                this, &ToolBarCustom::searchPacients, Qt::UniqueConnection);
    }

}

void ToolBarCustom::setStyles(const QString &mainStyle, const QString &secondaryStyle)
{
    if (m_options.testFlag(ButtonSelect) && !secondaryStyle.isEmpty()) {
        btnSelect->setStyleSheet(secondaryStyle);
    }

    if (m_options.testFlag(AddEditDelete)) {
        btnAdd->setStyleSheet(mainStyle);
        btnEdit->setStyleSheet(mainStyle);
        btnDelete->setStyleSheet(mainStyle);
    }

    if (m_options.testFlag(Filter)) {
        btnAddFilter->setStyleSheet(mainStyle);
        btnSetFilter->setStyleSheet(mainStyle);
        btnDeleteFilter->setStyleSheet(mainStyle);
    }

    if (m_options.testFlag(UpdateColumn)) {
        btnUpdate->setStyleSheet(mainStyle);
        btnHideColumn->setStyleSheet(mainStyle);
    }

    if (m_options.testFlag(PrintEmail)) {
        btnPrint->setStyleSheet(mainStyle);
        btnEmail->setStyleSheet(mainStyle);
    }

    if (m_options.testFlag(ReportViewTab) && !secondaryStyle.isEmpty()) {
        btnReport->setStyleSheet(secondaryStyle);
        btnView->setStyleSheet(secondaryStyle);
    }

    if (m_options.testFlag(PeriodSearch)) {
        btnPeriod->setStyleSheet(mainStyle);
        btnSearch->setStyleSheet(mainStyle);
    }
}

void ToolBarCustom::setTextPeriod(const QString &text)
{
    txtPeriod->setText(text);
}

QString ToolBarCustom::getTextPeriod() const
{
    return txtPeriod->text();
}

QToolButton *ToolBarCustom::getBtnSelect() const
{
    return btnSelect;
}

QToolButton *ToolBarCustom::getBtnAddDoc() const
{
    return btnAdd;
}

QToolButton *ToolBarCustom::getBtnEditDoc() const
{
    return btnEdit;
}

QToolButton *ToolBarCustom::getBtnDeletDoc() const
{
    return btnDelete;
}

QToolButton *ToolBarCustom::getBtnAddFilter() const
{
    return btnAddFilter;
}

QToolButton *ToolBarCustom::getBtnSetFilter() const
{
    return btnSetFilter;
}

QToolButton *ToolBarCustom::getBtnDeleteFilter() const
{
    return btnDeleteFilter;
}

QToolButton *ToolBarCustom::getBtnUpdateTable() const
{
    return btnUpdate;
}

QToolButton *ToolBarCustom::getBtnHideShowColumn() const
{
    return btnHideColumn;
}

QToolButton *ToolBarCustom::getBtnPrintDoc() const
{
    return btnPrint;
}

QToolButton *ToolBarCustom::getBtnSendEmail() const
{
    return btnEmail;
}

QToolButton *ToolBarCustom::getBtnCreateReport() const
{
    return btnReport;
}

QToolButton *ToolBarCustom::getBtnViewTabOrder() const
{
    return btnView;
}

QToolButton *ToolBarCustom::getBtnOpenPeriod() const
{
    return btnPeriod;
}

QToolButton *ToolBarCustom::getBtnSaecrPacient() const
{
    return btnSearch;
}

void ToolBarCustom::setupFilterMenu(QMenu *menu)
{
    btnSetFilter->setMenu(menu);
    btnSetFilter->setPopupMode(QToolButton::MenuButtonPopup);
}

QToolButton *ToolBarCustom::createBtn(const QString &icon,
                                       const QKeySequence &shortcut,
                                       const QString textBtn)
{
    auto *btn = new QToolButton(this);
    btn->setIcon(QIcon(icon));
    if (! textBtn.isEmpty()) {
        btn->setText(textBtn);
        btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }
    btn->setAutoRaise(false);
    btn->setMouseTracking(true);
    btn->setShortcut(shortcut);

    return btn;
}

QLabel *ToolBarCustom::createLabel(const QString &text, const QString style)
{
    auto *lbl = new QLabel(this);
    if (!style.isEmpty())
        lbl->setStyleSheet(style);
    lbl->setText(text);
    return lbl;
}
