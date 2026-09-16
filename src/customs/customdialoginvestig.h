#ifndef CUSTOMDIALOGINVESTIG_H
#define CUSTOMDIALOGINVESTIG_H

#include <QDialog>
#include <QStringList>

#include <common/table_sections.h>

class QListWidget;
class QListWidgetItem;
class QDialogButtonBox;
class QPushButton;
class QLabel;

class CustomDialogInvestig : public QDialog
{
    Q_OBJECT

public:
    explicit CustomDialogInvestig(QWidget *parent = nullptr);
    ~CustomDialogInvestig() override = default;

    void setCodes(const QStringList &codes);
    void setSystems(ReportSections::ReportSystems systems);

    QStringList checkedDisplayNames() const;
    ReportSections::ReportSystems selectedSystems() const;

private slots:
    void onAccept();
    void onCheckAll();
    void onUncheckAll();

private:
    void buildUi();
    void populateList();
    void updateInfoLabel();

    QListWidgetItem *itemBySystem(ReportSections::ReportSystem system) const;
    static ReportSections::ReportSystem systemFromItem(const QListWidgetItem *item);

private:
    QListWidget *m_listWidget = nullptr;
    QLabel      *m_infoLabel  = nullptr;
    QDialogButtonBox *m_buttonBox = nullptr;
    QPushButton *m_btnCheckAll    = nullptr;
    QPushButton *m_btnUncheckAll  = nullptr;
};

#endif // CUSTOMDIALOGINVESTIG_H