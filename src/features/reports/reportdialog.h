#ifndef REPORTDIALOG_H
#define REPORTDIALOG_H

#include <QCompleter>
#include <QDialog>
#include <QTimer>
#include <QStandardItemModel>
#include <QCommandLinkButton>
#include <memory.h>
#include <QVector>

#include <settings/layoutsettingsmanager.h>

#include <features/catalogs/catalogdialog.h> // patient
#include <database/database.h>

#include <features/reports/reportpageorgansinternal.h>
#include <features/reports/reportpageurinarysystem.h>
#include <features/reports/reportpageprostate.h>
#include <features/reports/reportpagegynecology.h>
#include <features/reports/reportpagebreast.h>
#include <features/reports/reportpagethyroid.h>
#include <features/reports/reportpagegestation0.h>
#include <features/reports/reportpagegestation1.h>
#include <features/reports/reportpagegestation2.h>
#include <features/reports/reportpagelymphnodes.h>
#include <features/reports/reportpageimage.h>
#include <features/reports/reportpagevideo.h>

#include <common/table_sections.h>
#include <common/reportdialogcontext.h>

namespace Ui {
class ReportDialog;
}

class ReportDialog : public QDialog
{
    Q_OBJECT

public:
    struct ReportDialogParameters
    {
        bool isNew      = false;
        int id          = 0;
        int idPatient   = 0;
        int idOrder     = 0;
        int idUser      = 0;
        QString nameUser;
        DocStatus::Column status = DocStatus::Unknow;
        ReportSections::ReportSystems systems;
        QString orderDisplayText;
    };

    explicit ReportDialog(DataBase &db,
                          const ReportDialogParameters &parameters,
                          QWidget *parent = nullptr);
    ~ReportDialog();

    QDate documentDate() const;    // pu ReportPageGynecology
    int getId() const;             // pu ReportPageVideo
    int getIdOrder() const;        // pu ReportPageVideo + ReportPageImage
    int getIdPatient() const;      // pu ReportPageImage
    int getIdUser() const;         // pu ReportPageImage
    bool documentModified() const; // pu toate clasele ReportPage...
    bool documentIsNew() const;

    /** functiile exportate pu alte clase */
    void onPrintDocument(PrintType::Column type_print,
                         const QString &filePDF = QString());
    [[nodiscard]] QStringList exportToPdf(const QString &fileBase,
                                          bool showDoctorStamp,
                                          bool showDoctorSignature,
                                          QString *error = nullptr);
    bool extPostDocument();

signals:
    void reportCreated();
    void reportChanged();
    void reportPost();


private slots:
    void dataWasModified();
    void updateTimerDateDoc();
    void onDateTimeChanged();

    void onChooseInvestigations();
    void onOpenPrintParameters();
    void onOpenOrder();
    void onOpenPatient();
    void onOpenPatientHistory();

    void slotPatientTextChanged(const QString &text);
    void updateModelPatientsByText();
    void activatedItemCompleter(const QModelIndex &index);

    void updateTextConcluzionBySystem();

    void openNormograms();

    bool controlRequiredObjects();
    void onPrint(PrintType::Column type_print,
                 const QString &filePDF);
    bool onSave();
    bool onPost();

private:
    void applyParameters();
    void buildSections();
    void setupVisibleSections();
    void setupNavigationConnections();
    void showFirstAvailablePage();

    void addSection(ReportSections::ReportSystem system,
                    ReportPageBase *page,
                    QCommandLinkButton *button);
    void loadDataSection();
    void setupPageSignals();

    void initConnections();
    void setupOptionsMenu();

    void setStatusDcument(DocStatus::Column status);

    void setIdPatient(const int id);
    void setIdUser(const int id);
    void ensurePatientInCompleterModel(int idPatient, const QString &fullName);
    void loadPatientDetails();

    void loadReport();
    bool loadDocumentContext();

    void initSetCompleter();

    void initSetStyleFrame();
    void updateStyleBtnNavigation();

    void initFooterDoc();

    void setPost(DocStatus::Column post);

    void bindValues(QSqlQuery &q);
    bool insertData();
    bool updateData();
    bool updateParentOrderAttachedMedia(QString *error = nullptr);

    void initSync();

    void saveWindowSize();
    void restoreWindowSize();

private:
    Ui::ReportDialog *ui;
    LayoutSettingsManager m_settings;

    DataBase    &m_db;
    QSqlDatabase m_currentDB;
    PopUp    *popUp;
    QTimer   *timer; // data si ora

    ReportDialogParameters m_params;
    ReportDialogContext m_documentContext;
    ReportSections::ReportSystems m_systems;
    DocStatus::Column m_statusDoc;

    bool isOpenNormogram = false;

    struct SectionUi {
        ReportPageBase *page = nullptr;
        QCommandLinkButton *button = nullptr;
    };
    QHash<ReportSections::ReportSystem, SectionUi> m_sections;

    QStandardItemModel *modelPatients      = nullptr;
    QCompleter         *completerPatients  = nullptr;
    QTimer             *timerPatientSearch = nullptr;

    QString toolButtonStyleForText;
    QString styleForButtonMessageBox;

    DocStatus::Column m_post;
    bool m_postInProgress = false;

    int m_attachedImages = 0;
    int m_attachedVideos = 0;

    QString style_pressed;
    QString style_unpressed;

    bool m_showDoctorStamp      = false;
    bool m_showDoctorSignature  = false;

protected:
    void closeEvent(QCloseEvent *event);   // controlam modificarea datelor
    void changeEvent(QEvent *event);       // contolam traducerea aplicatiei
    void keyPressEvent(QKeyEvent *event);  // procedura de bypass a elementelor
                                           // (Qt::Key_Return | Qt::Key_Enter)
};

#endif // REPORTDIALOG_H
