#ifndef REPORTSEMAILSELECTIONDIALOG_H
#define REPORTSEMAILSELECTIONDIALOG_H

#include <QDate>
#include <QDialog>
#include <QSet>
#include <QVector>

#include <database/database.h>

class QCheckBox;
class QComboBox;
class QDateEdit;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSortFilterProxyModel;
class QStandardItem;
class QStandardItemModel;
class QTableView;
class QueryRolesModel;

// Selectarea (prin bifare) a rapoartelor ecografice validate ale unei
// organizații trimițătoare, pentru transmiterea lor într-o singură scrisoare.
class ReportsEmailSelectionDialog final : public QDialog
{
    Q_OBJECT

public:
    // Numărul maxim de rapoarte într-o scrisoare (limita atașamentelor SMTP).
    static constexpr int MaxReports = 30;

    explicit ReportsEmailSelectionDialog(DataBase &db, QWidget *parent = nullptr);

    // Organizația și perioada inițiale (filtrul din ReportView) și rapoartele
    // marcate în jurnal, bifate implicit dacă aparțin listei.
    void setInitialState(int organizationId,
                         const QDate &startDate,
                         const QDate &endDate,
                         const QSet<qint64> &preselectedIds);

    [[nodiscard]] int organizationId() const;
    [[nodiscard]] QVector<qint64> selectedReportIds() const;
    [[nodiscard]] bool includeImages() const;

public slots:
    void accept() override;

private slots:
    void reloadReports();
    void onOrganizationChanged();
    void onItemChanged(QStandardItem *item);
    void setVisibleRowsChecked(bool checked);

private:
    void buildUi();
    void updateRecipientInfo();
    void updateSelectionInfo();
    int checkedCount() const;

    DataBase &m_db;
    QSet<qint64> m_preselectedIds;
    QString m_recipientEmail;
    bool m_bulkUpdate = false;

    QueryRolesModel *m_organizations = nullptr;
    QStandardItemModel *m_model = nullptr;
    QSortFilterProxyModel *m_proxy = nullptr;

    QComboBox *m_comboOrganization = nullptr;
    QLabel *m_lblRecipient = nullptr;
    QDateEdit *m_dateStart = nullptr;
    QDateEdit *m_dateEnd = nullptr;
    QLineEdit *m_search = nullptr;
    QTableView *m_table = nullptr;
    QCheckBox *m_chkImages = nullptr;
    QLabel *m_lblSelection = nullptr;
    QDialogButtonBox *m_buttons = nullptr;
    QPushButton *m_btnAccept = nullptr;
};

#endif // REPORTSEMAILSELECTIONDIALOG_H
