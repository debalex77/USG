#ifndef DOCREPORTECHO_H
#define DOCREPORTECHO_H

#include <QDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QCompleter>
#include <QStandardItemModel>
#include <QSqlQueryModel>
#include <LimeReport>
#include <QFileDialog>
#include <QMediaPlayer>
#include <QMediaFormat>
#include <QVideoWidget>
#include <infowindow.h>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QGraphicsView>

#include "data/popup.h"
#include "data/database.h"
#include <data/enums.h>
#include <data/customdialoginvestig.h>
#include <catalogs/catgeneral.h>
#include <catalogs/chooseformprint.h>
#include <docs/docorderecho.h>
#include <models/basesortfilterproxymodel.h>

#include <catalogs/normograms.h>

#include <common/datapercentage.h>

namespace Ui {
class DocReportEcho;
}

class DocReportEcho : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(bool ItNew READ getItNew WRITE setItNew NOTIFY ItNewChanged)
    Q_PROPERTY(int Id READ getId WRITE setId NOTIFY IdChanged)
    Q_PROPERTY(int IdPacient READ getIdPacient WRITE setIdPacient NOTIFY IdPacientChanged)
    Q_PROPERTY(int IdDocOrderEcho READ getIdDocOrderEcho WRITE setIdDocOrderEcho NOTIFY IdDocOrderEchoChanged)
    Q_PROPERTY(int IdUser READ getIdUser WRITE setIdUser NOTIFY IdUserChanged)
    Q_PROPERTY(int Post READ getPost WRITE setPost NOTIFY PostChanged)

    Q_PROPERTY(bool t_organs_internal READ get_t_organs_internal WRITE set_t_organs_internal NOTIFY t_organs_internalChanged)
    Q_PROPERTY(bool t_urinary_system READ get_t_urinary_system WRITE set_t_urinary_system NOTIFY t_urinary_systemChanged)
    Q_PROPERTY(bool t_prostate READ get_t_prostate WRITE set_t_prostate NOTIFY t_prostateChanged)
    Q_PROPERTY(bool t_gynecology READ get_t_gynecology WRITE set_t_gynecology NOTIFY t_gynecologyChanged)
    Q_PROPERTY(bool t_breast READ get_t_breast WRITE set_t_breast NOTIFY t_breastChanged)
    Q_PROPERTY(bool t_thyroide READ get_t_thyroide WRITE set_t_thyroide NOTIFY t_thyroideChanged)
    Q_PROPERTY(bool t_gestation0 READ get_t_gestation0 WRITE set_t_gestation0 NOTIFY t_gestation0Changed)
    Q_PROPERTY(bool t_gestation1 READ get_t_gestation1 WRITE set_t_gestation1 NOTIFY t_gestation1Changed)
    Q_PROPERTY(bool t_gestation2 READ get_t_gestation2 WRITE set_t_gestation2 NOTIFY t_gestation2Changed)

    Q_PROPERTY(bool CountImages READ getCountImages WRITE setCountImages NOTIFY CountImagesChanged)
    Q_PROPERTY(bool CountVideo READ getCountVideo WRITE setCountVideo NOTIFY CountVideoChanged)

public:
    explicit DocReportEcho(QWidget *parent = nullptr);
    ~DocReportEcho();

    void setItNew(bool ItNew) {m_itNew = ItNew; emit ItNewChanged();}
    bool getItNew() {return m_itNew;}

    void setId(int Id) {m_id = Id; emit IdChanged();}
    int getId() const {return m_id;}

    void setIdPacient(int IdPacient) {m_idPacient = IdPacient; emit IdPacientChanged();}
    int getIdPacient() const {return m_idPacient;}

    void setIdDocOrderEcho(int IdDocOrderEcho) {m_id_docOrderEcho = IdDocOrderEcho; emit IdDocOrderEchoChanged();}
    int getIdDocOrderEcho() const {return m_id_docOrderEcho;}

    void set_t_organs_internal(bool t_organs_internal)
    {
        m_organs_internal = t_organs_internal;
        setDefaultDataTableLiver();
        setDefaultDataTableCholecist();
        setDefaultDataTablePancreas();
        setDefaultDataTableSpleen();
        setDefaultDataTableIntestinalLoop();
        emit t_organs_internalChanged();
    }
    bool get_t_organs_internal() const {return m_organs_internal;}

    void set_t_urinary_system(bool t_urinary_system)
    {
        m_urinary_system = t_urinary_system;
        setDefaultDataKidney();
        setDefaultDataBladder();
        emit t_urinary_systemChanged();
    }
    bool get_t_urinary_system() const {return m_urinary_system;}

    void set_t_prostate(bool t_prostate) {m_prostate = t_prostate; setDefaultDataProstate(); emit t_prostateChanged();}
    bool get_t_prostate() const {return m_prostate;}

    void set_t_gynecology(bool t_gynecology) {m_gynecology = t_gynecology; setDefaultDataGynecology(); emit t_gynecologyChanged();}
    bool get_t_gynecology() const {return m_gynecology;}

    void set_t_breast(bool t_breast) {m_breast = t_breast; setDefaultDataBreast(); emit t_breastChanged();}
    bool get_t_breast() const {return m_breast;}

    void set_t_thyroide(bool t_thyroide) {m_thyroide = t_thyroide; setDefaultDataThyroid(); emit t_thyroideChanged();}
    bool get_t_thyroide() const {return m_thyroide;}

    void set_t_gestation0(bool t_gestation0) {m_gestation0 = t_gestation0; setDefaultDataGestation0(); emit t_gestation0Changed();}
    bool get_t_gestation0() const {return m_gestation0;}

    void set_t_gestation1(bool t_gestation1) {m_gestation1 = t_gestation1; setDefaultDataGestation1(); emit t_gestation1Changed();}
    bool get_t_gestation1() const {return m_gestation1;}

    void set_t_gestation2(bool t_gestation2) {m_gestation2 = t_gestation2; setDefaultDataGestation2();emit t_gestation2Changed();}
    bool get_t_gestation2() const {return m_gestation2;}

    void setIdUser(int IdUser) {m_idUser = IdUser; emit IdUserChanged();}
    int getIdUser() const {return m_idUser;}

    void setPost(int Post) {m_post = Post; emit PostChanged();}
    int getPost() const {return m_post;}

    void setCountImages(int CountImages){m_count_images = CountImages; emit CountImagesChanged();}
    int getCountImages() const {return m_count_images;}

    void setCountVideo(int CountVideo) {m_count_video = CountVideo; emit CountVideoChanged();}
    int getCountVideo() const {return m_count_video;}

    void onPrintDocument(Enums::TYPE_PRINT _typeReport); // functiile exportate pu solicitarea din alte clase

    void m_onWritingData();

signals:
    void ItNewChanged();
    void IdChanged();
    void IdPacientChanged();
    void IdDocOrderEchoChanged();
    void t_organs_internalChanged();
    void t_urinary_systemChanged();
    void t_prostateChanged();
    void t_gynecologyChanged();
    void t_breastChanged();
    void t_thyroideChanged();
    void t_gestation0Changed();
    void t_gestation1Changed();
    void t_gestation2Changed();
    void IdUserChanged();
    void PostChanged();
    void PostDocument(); // pu actualizarea listei documentelor
    void mCloseThisForm();
    void CountImagesChanged(); // pu determinarea cantitatii imiginilor
    void CountVideoChanged();

private slots:
    void dataWasModified();    // modificarea datelor formei
    void updateTimer();        // actualizarea datei si orei in regim real
    void onDateTimeChanged();

    void onDateLMPChanged(QDate m_date);

    bool loadFile(const QString &fileName, const int numberImage); // atasarea imaginilor
    void loadImageOpeningDocument();

    void onLinkActivatedForOpenImage1(const QString &link);
    void onLinkActivatedForOpenImage2(const QString &link);
    void onLinkActivatedForOpenImage3(const QString &link);
    void onLinkActivatedForOpenImage4(const QString &link);
    void onLinkActivatedForOpenImage5(const QString &link);
    void insertImageIntoTableimagesReports(const QString &fileName, const int numberImage);
    void updateImageIntoTableimagesReports(const QString &fileName, const int numberImage);
    void removeImageIntoTableimagesReports(const int numberImage);
    void updateCommentIntoTableimagesReports();

    void initConnections();  // initierea conectarilor + setarea stylului
    void openParameters();
    void openCatPatient();
    void openHistoryPatient();
    void openDocOrderEcho();

    void openTempletsBySystem(const QString name_system);
    void openCatalogWithSystemTamplets(const QString name_system);
    void addDescriptionFormation(const QString name_system);

    void clickBtnOrgansInternal();
    void clickBtnUrinarySystem();
    void clickBtnProstate();
    void clickBtnGynecology();
    void clickBtnBreast();
    void clickBtnThyroid();
    void clickBtnGestation0();
    void clickBtnGestation1();
    void clickBtnGestation2();
    void clickBtnComment();
    void clickBtnImages();
    void clickBtnVideo();
    void clickBtnNormograms();
    void createMenuPrint();         // meniu print
    void clickOpenDesignerReport(); // deschidem designer
    void clickOpenPreviewReport();  // deschidem preview
    void clickBtnClearImage1();
    void clickBtnClearImage2();
    void clickBtnClearImage3();
    void clickBtnClearImage4();
    void clickBtnClearImage5();
    void clickBtnAddConcluzionTemplates();
    void clickBtnSelectConcluzionTemplates();

    void clickAddVideo();
    void clickRemoveVideo();
    void setUrl(const QUrl &url);
    void clickPlayVideo();
    void setPositionSlider(int seconds);
    void mediaStateChanged(QMediaPlayer::PlaybackState state);
    void positionChanged(qint64 progress);
    void durationChanged(qint64 duration);
    void updateDurationInfo(qint64 currentInfo);
    void handleError();
    void changedCurrentRowPlayList(int row);

    void connections_tags();               // conexiunea la tag
    void disconnections_tags();
    void connections_organs_internal();    // conexiunea la bloc 'organs_internal'
    void disconnections_organs_internal();
    void connections_urinary_system();     // conexiunea la bloc 'urinary_system'
    void disconnections_urinary_system();
    void connections_prostate();           // conexiunea la bloc 'prostate'
    void disconnections_prostate();
    void connections_gynecology();         // conexiunea la bloc 'gynecology'
    void disconnections_gynecology();
    void connections_breast();             // conexiunea la bloc 'breast'
    void disconnections_breast();
    void connections_thyroid();            // conexiunea la bloc 'thyroid'
    void disconnections_thyroid();
    void connections_gestation0();         // conexiunea la bloc 'gestation 0'
    void disconnections_gestation0();
    void connections_gestation1();         // conexiunea la bloc 'gestation 1'
    void disconnections_gestation1();
    void connections_gestation2();         // conexiunea la bloc 'gestation 2'
    void disconnections_gestation2();

    void clickedGestation2Trimestru(const int id_button);
    void getPercentageByDoppler();

    void slot_ItNewChanged();
    void slot_IdChanged();
    void slot_IdPacientChanged();
    void slot_IdDocOrderEchoChanged();
    void slot_IdUserChanged();
    void slot_CountImagesChanged();
    void slot_CountVideoChanged();

    void updateTextDateMenstruation();    // actualizam textul cate zile de la cicl.menstr.

    void activatedItemCompleter(const QModelIndex &index); // activarea 'completer'

    bool controlRequiredObjects();       // controlul completarii obiectelor obligatorii
    void onPrint(Enums::TYPE_PRINT _typeReport); // printare -> _typeReport = variabila pu determinarea lansarii designer ori preview din list...
    bool onWritingData();                // 'btnWrite'
    void onWritingDataClose();           // 'btnOk'
    void onClose();                      // 'btnClose'

private:
    enum IndexPage
    {
        page_unknow          = -1,
        page_organs_internal = 0,
        page_urinary_system  = 1,
        page_prostate        = 2,
        page_gynecology      = 3,
        page_breast          = 4,
        page_thyroid         = 5,
        page_gestation0      = 6,
        page_gestation1      = 7,
        page_gestation2      = 8,
        page_images          = 9,
        page_video           = 10
    };

    enum IndexNumImage
    {
        n_image1 = 1,
        n_image2 = 2,
        n_image3 = 3,
        n_image4 = 4,
        n_image5 = 5
    };

    QString calculateGestationalAge(const QDate &lmp);
    QDate calculateDueDate(const QDate &lmp);
    QDate calculateDueDateFromFetalAge(int fetalAgeWeeks, int fetalAgeDays = 0);
    void setDueDateGestation2();
    void extractWeeksAndDays(const QString &str_vg, int &weeks, int &days);

    void constructionFormVideo();
    void findVideoFiles();

    void setDefaultDataTableLiver();      // setarea datelor implicite initial
    void setDefaultDataTableCholecist();  // in dependenta de bloc + tag ale documentului
    void setDefaultDataTablePancreas();
    void setDefaultDataTableSpleen();
    void setDefaultDataTableIntestinalLoop();
    void setDefaultDataKidney();
    void setDefaultDataBladder();
    void setDefaultDataProstate();
    void setDefaultDataGynecology();
    void setDefaultDataBreast();
    void setDefaultDataThyroid();
    void setDefaultDataGestation0();
    void setDefaultDataGestation1();
    void setDefaultDataGestation2();

    void initEnableBtn();                  // initierea accesului la btn + setarea focusului + paginei
    void updateStyleBtnInvestigations();
    void initSetCompleter();               // initierea completarului
    void initFooterDoc();                  // initierea btn + setarea textului autorului

    bool insertingDocumentDataIntoTables(QString &details_error);
    bool updatingDocumentDataIntoTables(QString &details_error);

    void updateDataDocOrderEcho(); // actualizarea datelor doc.Comanda ecografica - inserarea valorii atasarii imaginei

    QString getStringTablesBySystems();
    void processingRequest();
    void setDataFromSystemOrgansInternal(); // inserarea datelor in forma din tabele
    void setDataFromSystemUrinary();
    void setDataFromTableProstate();
    void setDataFromTableGynecology();
    void setDataFromTableBreast();
    void setDataFromTableThyroid();
    void setDataFromTableGestation0();
    void setDataFromTableGestation1();
    void setDataFromTableGestation2();

private:
    Ui::DocReportEcho *ui;

    bool m_itNew          = false;
    int m_id              = Enums::IDX::IDX_UNKNOW;
    int m_idPacient       = Enums::IDX::IDX_UNKNOW;
    int m_id_docOrderEcho = Enums::IDX::IDX_UNKNOW;
    int m_idUser          = Enums::IDX::IDX_UNKNOW;
    int m_post            = Enums::IDX::IDX_UNKNOW;

    bool m_organs_internal = false;
    bool m_urinary_system  = false;
    bool m_prostate        = false;
    bool m_gynecology      = false;
    bool m_breast          = false;
    bool m_thyroide        = false;
    bool m_gestation0      = false;
    bool m_gestation1      = false;
    bool m_gestation2      = false;
    bool m_gestation3      = false;

    int m_count_images = 0;

    DataBase *db;
    PopUp    *popUp;
    QTimer   *timer;

    QCompleter               *completer;
    QStandardItemModel       *modelPatients;
    BaseSortFilterProxyModel *proxyPatient;

    PatientHistory *history_patient;

    QStandardItemModel *model_logo;
    QSqlQueryModel     *modelOrganization;      // modele pu instalarea datelor
    QSqlQueryModel     *modelPatient_print;
    QSqlQueryModel     *modelOrgansInternal;
    QSqlQueryModel     *modelUrinarySystem;
    QSqlQueryModel     *modelProstate;
    QSqlQueryModel     *modelGynecology;
    QSqlQueryModel     *modelBreast;
    QSqlQueryModel     *modelThyroid;
    QSqlQueryModel     *modelGestationO;
    QSqlQueryModel     *modelGestation1;
    QSqlQueryModel     *modelGestation2;

    QButtonGroup *group_btn_prostate;       // instalarea grupelor RadioButton pe blocuri
    QButtonGroup *group_btn_gynecology;
    QButtonGroup *group_btn_gestation0;
    QButtonGroup *group_btn_gestation1;
    QButtonGroup *group_btn_gestation2;

    QString str_concluzion_organs_internal; // pentru concluziile pe blocuri
    QString str_concluzion_urinary_system;
    QString str_concluzion_prostate;
    QString str_concluzion_gynecology;
    QString str_concluzion_brest;
    QString str_concluzion_thyroid;
    QString str_concluzion_gestation0;
    QString str_concluzion_gestation1;
    QString str_concluzion_gestation2;

    LimeReport::ReportEngine *m_report;
    Normograms *normograms;

    QPushButton    *m_playButton     = nullptr;
    QMediaPlayer   *player           = nullptr;
    QListWidget    *list_play        = nullptr;
    QLabel         *txt_title_player = nullptr;
    QSlider        *m_positionSlider = nullptr;
#if defined(Q_OS_LINUX) || defined (Q_OS_WIN)
    QVideoWidget   *videoWidget      = nullptr;
#elif defined(Q_OS_MACOS)
    QGraphicsScene *scene;
    QGraphicsVideoItem *videoItem;
    QGraphicsView *view;
#endif
    QLabel         *m_labelDuration  = nullptr;
    QLabel         *m_errorLabel     = nullptr;
    qint64         m_duration;
    int            m_count_video     = 0;

    InfoWindow *info_win;

    DataPercentage *data_percentage;

protected:
    void closeEvent(QCloseEvent *event);   // controlam modificarea datelor
    bool eventFilter(QObject *obj, QEvent *event);
    void changeEvent(QEvent *event);       // contolam traducerea aplicatiei
    void keyPressEvent(QKeyEvent *event);  // procedura de bypass a elementelor
                                           // (Qt::Key_Return | Qt::Key_Enter)
};

#endif // DOCREPORTECHO_H
