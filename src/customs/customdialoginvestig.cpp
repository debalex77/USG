#include "customdialoginvestig.h"

#include <QAbstractItemView>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

CustomDialogInvestig::CustomDialogInvestig(QWidget *parent)
    : QDialog(parent)
{
    buildUi();
    populateList();
    updateInfoLabel();

    connect(m_buttonBox, &QDialogButtonBox::accepted,
            this, &CustomDialogInvestig::onAccept);
    connect(m_buttonBox, &QDialogButtonBox::rejected,
            this, &CustomDialogInvestig::reject);

    connect(m_btnCheckAll, &QPushButton::clicked,
            this, &CustomDialogInvestig::onCheckAll);
    connect(m_btnUncheckAll, &QPushButton::clicked,
            this, &CustomDialogInvestig::onUncheckAll);

    connect(m_listWidget, &QListWidget::itemChanged,
            this, [this](QListWidgetItem *)
            {
                updateInfoLabel();
            });
}

void CustomDialogInvestig::buildUi()
{
    setWindowTitle(tr("Selectare investigații"));
    resize(320, 350);
    setModal(true);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    auto *titleLabel = new QLabel(tr("Selectați investigațiile necesare:"), this);
    mainLayout->addWidget(titleLabel);

    m_listWidget = new QListWidget(this);
    m_listWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_listWidget->setAlternatingRowColors(true);
    mainLayout->addWidget(m_listWidget, 1);

    m_infoLabel = new QLabel(this);
    m_infoLabel->setMaximumWidth(310);
    m_infoLabel->setWordWrap(true);
    mainLayout->addWidget(m_infoLabel);

    auto *buttonsRowLayout = new QHBoxLayout;
    buttonsRowLayout->setContentsMargins(0, 0, 0, 0);
    buttonsRowLayout->setSpacing(6);

    m_btnCheckAll = new QPushButton(tr("Bifează tot"), this);
    m_btnUncheckAll = new QPushButton(tr("Debifează tot"), this);
    m_btnCheckAll->setMinimumWidth(40);
    m_btnUncheckAll->setMinimumWidth(40);

    buttonsRowLayout->addWidget(m_btnCheckAll);
    buttonsRowLayout->addWidget(m_btnUncheckAll);
    buttonsRowLayout->addStretch();

    mainLayout->addLayout(buttonsRowLayout);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_buttonBox->button(QDialogButtonBox::Ok)->setMinimumWidth(80);
    m_buttonBox->button(QDialogButtonBox::Cancel)->setMinimumWidth(80);

    mainLayout->addWidget(m_buttonBox);
}

void CustomDialogInvestig::populateList()
{
    m_listWidget->clear();

    const QList<ReportSections::ReportSystem> systems = ReportSections::allSystems();
    for (ReportSections::ReportSystem system : systems) {
        if (system == ReportSections::ReportSystem::Video ||
            system == ReportSections::ReportSystem::Images)
            continue;
        auto *item = new QListWidgetItem(ReportSections::systemDisplayName(system), m_listWidget);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        item->setData(Qt::UserRole, static_cast<int>(system));
    }
}

void CustomDialogInvestig::setCodes(const QStringList &codes)
{

    const ReportSections::ReportSystems systems = ReportSections::systemsByCodes(codes);
    setSystems(systems);
}

void CustomDialogInvestig::setSystems(ReportSections::ReportSystems systems)
{
    for (int row = 0; row < m_listWidget->count(); ++row) {
        QListWidgetItem *item = m_listWidget->item(row);
        if (!item)
            continue;

        const ReportSections::ReportSystem system = systemFromItem(item);
        item->setCheckState(systems.testFlag(system) ? Qt::Checked : Qt::Unchecked);
    }

    updateInfoLabel();
}

QStringList CustomDialogInvestig::checkedDisplayNames() const
{
    QStringList result;

    for (int row = 0; row < m_listWidget->count(); ++row) {
        QListWidgetItem *item = m_listWidget->item(row);

        if (!item)
            continue;

        if (item->text().isEmpty()) // din cauza Video si Image
            continue;

        if (item->checkState() == Qt::Checked)
            result << item->text();
    }

    return result;
}

ReportSections::ReportSystems CustomDialogInvestig::selectedSystems() const
{
    ReportSections::ReportSystems result;

    for (int row = 0; row < m_listWidget->count(); ++row) {
        const QListWidgetItem *item = m_listWidget->item(row);
        if (!item)
            continue;

        if (item->checkState() != Qt::Checked)
            continue;

        result |= systemFromItem(item);
    }

    return result;
}

void CustomDialogInvestig::onAccept()
{
    if (selectedSystems() == ReportSections::ReportSystems{}) {
        QMessageBox::warning(this,
                             tr("Atenție"),
                             tr("Selectați cel puțin o investigație."));
        return;
    }

    accept();
}

void CustomDialogInvestig::onCheckAll()
{
    for (int row = 0; row < m_listWidget->count(); ++row) {
        QListWidgetItem *item = m_listWidget->item(row);
        if (item)
            item->setCheckState(Qt::Checked);
    }

    updateInfoLabel();
}

void CustomDialogInvestig::onUncheckAll()
{
    for (int row = 0; row < m_listWidget->count(); ++row) {
        QListWidgetItem *item = m_listWidget->item(row);
        if (item)
            item->setCheckState(Qt::Unchecked);
    }

    updateInfoLabel();
}

void CustomDialogInvestig::updateInfoLabel()
{
    const QStringList names = checkedDisplayNames();

    if (names.isEmpty()) {
        m_infoLabel->setText(tr("Nu este selectată nicio investigație."));
        return;
    }

    m_infoLabel->setText(tr("Selectate: %1").arg(names.join(", ")));
}

QListWidgetItem *CustomDialogInvestig::itemBySystem(ReportSections::ReportSystem system) const
{
    for (int row = 0; row < m_listWidget->count(); ++row) {
        QListWidgetItem *item = m_listWidget->item(row);
        if (!item)
            continue;

        if (systemFromItem(item) == system)
            return item;
    }

    return nullptr;
}

ReportSections::ReportSystem CustomDialogInvestig::systemFromItem(const QListWidgetItem *item)
{
    return static_cast<ReportSections::ReportSystem>(item->data(Qt::UserRole).toInt());
}