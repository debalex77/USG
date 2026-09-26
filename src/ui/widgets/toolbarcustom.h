#ifndef TOOLBARCUSTOM_H
#define TOOLBARCUSTOM_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

class ToolBarCustom : public QWidget
{
    Q_OBJECT
public:
    enum Option {
        AddEditDelete = 0x01,
        Filter        = 0x02,
        UpdateColumn  = 0x04,
        PrintEmail    = 0x08,
        ReportViewTab = 0x10,
        PeriodSearch  = 0x20,
        ButtonSelect  = 0x40
    };
    Q_DECLARE_FLAGS(Options, Option)

    explicit ToolBarCustom(QWidget *parent = nullptr, Options options = AddEditDelete);

    void setStyles(const QString &mainStyle,
                   const QString &secondaryStyle = QString());

    void setTextPeriod(const QString &text);
    QString getTextPeriod() const;

    QToolButton *getBtnSelect() const;

    QToolButton *getBtnAddDoc() const;
    QToolButton *getBtnEditDoc() const;
    QToolButton *getBtnDeletDoc() const;

    QToolButton *getBtnAddFilter() const;
    QToolButton *getBtnSetFilter() const;
    QToolButton *getBtnDeleteFilter() const;

    QToolButton *getBtnUpdateTable() const;
    QToolButton *getBtnHideShowColumn() const;

    QToolButton *getBtnPrintDoc() const;
    QToolButton *getBtnSendEmail() const;

    QToolButton *getBtnCreateReport() const;
    QToolButton *getBtnViewTabOrder() const;

    QToolButton *getBtnOpenPeriod() const;
    QToolButton *getBtnSaecrPacient() const;

    void setupFilterMenu(QMenu *menu);

signals:
    void selectDoc();

    void addDoc();
    void editDoc();
    void deleteDoc();

    void addFilter();
    void setFilter();
    void deleteFilter();

    void updateTable();
    void hideShowColumn();

    void printDoc();
    void sendEmail();
    void createReport();
    void viewTabOrder();

    void openPeriod();
    void searchPacients();

private:
    QToolButton *createBtn(const QString &icon,
                           const QKeySequence &shortcut,
                           const QString textBtn = nullptr);

    QLabel *createLabel(const QString &text,
                        const QString style = nullptr);

private:
    Options m_options;

    QHBoxLayout *m_layout;

    QToolButton *btnSelect;
    QToolButton *btnAdd;
    QToolButton *btnEdit;
    QToolButton *btnDelete;

    QToolButton *btnAddFilter;
    QToolButton *btnSetFilter;
    QToolButton *btnDeleteFilter;

    QToolButton *btnUpdate;
    QToolButton *btnHideColumn;

    QToolButton *btnPrint;
    QToolButton *btnEmail;
    QToolButton *btnReport;
    QToolButton *btnView;

    QToolButton *btnPeriod;
    QToolButton *btnSearch;

    QLabel *txtPeriod;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ToolBarCustom::Options)

#endif // TOOLBARCUSTOM_H
