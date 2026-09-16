#ifndef REPORTDIALOG_H
#define REPORTDIALOG_H

#include <QCompleter>
#include <QDialog>
#include <QTimer>
#include <QStandardItemModel>
#include <QCommandLinkButton>
#include <LimeReport>
#include <memory.h>
#include <QVector>

#include <catalogs/catalogdialog.h> // patient
#include <data/database.h>

#include <documents/reportpageorgansinternal.h>
#include <documents/reportpageurinarysystem.h>
#include <documents/reportpageprostate.h>
#include <documents/reportpagegynecology.h>
#include <documents/reportpagebreast.h>
#include <documents/reportpagethyroid.h>
#include <documents/reportpagegestation0.h>
#include <documents/reportpagegestation1.h>
#include <documents/reportpagegestation2.h>
#include <documents/reportpagelymphnodes.h>
#include <documents/reportpageimage.h>
#include <documents/reportpagevideo.h>

#include <common/table_sections.h>

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
    bool extPostDocument();

signals:
    void reportCreated();
    void reportChanged();
    void reportPost();

    void printToPdfFinished();

private slots:
    void dataWasModified();
    void updateTimerDateDoc();
    void onDateTimeChanged();

    void onOpenParameters();
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

    void setStatusDcument(DocStatus::Column status);

    void setIdPatient(const int id);
    void setIdUser(const int id);
    void ensurePatientInCompleterModel(int idPatient, const QString &fullName);
    void loadPatientDetails();

    void loadReport();

    void initSetCompleter();

    void initSetStyleFrame();
    void updateStyleBtnNavigation();

    void initFooterDoc();

    void setPost(DocStatus::Column post);

    void bindValues(QSqlQuery &q);
    bool insertData();
    bool updateData();
    bool updateParentOrderAttachedMedia(QString *error = nullptr);

    QStandardItem* mkImageItem(const QString& cacheKey,
                               const QByteArray& bytes,
                               const QSize& targetSize);

    void setLogoStampOrganization(QStandardItemModel *model_img);
    void setPrintModelOrganization(QSqlQueryModel *print_model_organization);
    void setPrintModelPatient(QSqlQueryModel *print_model_patient);

    void showTemplateComplex(LimeReport::ReportEngine &report,
                             QSqlQueryModel &modelOrgansInternal,
                             QSqlQueryModel &modelUrinarySystem,
                             PrintType::Column typePrint,
                             const QString &filePDF);
    void showTemplateOrgansInternal(LimeReport::ReportEngine &report,
                                    QSqlQueryModel &modelOrgansInternal,
                                    PrintType::Column typePrint,
                                    const QString &filePDF);
    void showTemplateUrinarySystem(LimeReport::ReportEngine &report,
                                   QSqlQueryModel &modelUrinarySystem,
                                   PrintType::Column typePrint,
                                   const QString &filePDF);
    void showTemplateProstate(LimeReport::ReportEngine &report,
                              QSqlQueryModel &modelProstate,
                              PrintType::Column typePrint,
                              const QString &filePDF);
    void showTemplateGynecology(LimeReport::ReportEngine &report,
                                QSqlQueryModel &modelGynecology,
                                PrintType::Column typePrint,
                                const QString &filePDF);
    void showTemplateBreast(LimeReport::ReportEngine &report,
                            QSqlQueryModel &modelBreast,
                            PrintType::Column typePrint,
                            const QString &filePDF);
    void showTemplateThyroid(LimeReport::ReportEngine &report,
                             QSqlQueryModel &modelThyroid,
                             PrintType::Column typePrint,
                             const QString &filePDF);
    void showTemplateGestation0(LimeReport::ReportEngine &report,
                                QSqlQueryModel &modelGestation0,
                                PrintType::Column typePrint,
                                const QString &filePDF);
    void showTemplateGestation1(LimeReport::ReportEngine &report,
                                QSqlQueryModel &modelGestation1,
                                PrintType::Column typePrint,
                                const QString &filePDF);
    void showTemplateGestation2(LimeReport::ReportEngine &report,
                                QSqlQueryModel &modelGestation2,
                                PrintType::Column typePrint,
                                const QString &filePDF);
    void showTemplateLymphNodes(LimeReport::ReportEngine &report,
                                QSqlQueryModel &modelLymphNodes,
                                PrintType::Column typePrint,
                                const QString &filePDF);

    void initSync();

private:
    Ui::ReportDialog *ui;
    DataBase &m_db;
    QSqlDatabase m_currentDB;
    PopUp    *popUp;
    QTimer   *timer; // data si ora

    ReportDialogParameters m_params;
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

    int exist_logo              = 0;
    int exist_stamp_doctor      = 0;
    int exist_signature_doctor  = 0;
    int exist_stam_organization = 0;
    bool allTemplates = false;

protected:
    void closeEvent(QCloseEvent *event);   // controlam modificarea datelor
    void changeEvent(QEvent *event);       // contolam traducerea aplicatiei
    void keyPressEvent(QKeyEvent *event);  // procedura de bypass a elementelor
                                           // (Qt::Key_Return | Qt::Key_Enter)
};

#endif // REPORTDIALOG_H
