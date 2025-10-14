#ifndef DOCREPORTECHO_H
#define DOCREPORTECHO_H

#include <QDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QCompleter>
#include <LimeReport>
#include <QFileDialog>
#include <QMediaPlayer>
#include <QMediaFormat>
#include <QVideoWidget>
#include <common/infowindow.h>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QGraphicsView>
#include <customs/lineeditcustom.h>

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
#include <common/appmetatypes.h>

#include <QPlainTextEdit>
#include <memory> // pentru std::unique_ptr

namespace Ui {
class DocReportEcho;
}

class DocReportEchoHandler; // forward declaration

class DocReportEcho : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(bool ItNew READ getItNew WRITE setItNew NOTIFY ItNewChanged)
    Q_PROPERTY(int Id READ getId WRITE setId NOTIFY IdChanged)
    Q_PROPERTY(int IdPacient READ getIdPacient WRITE setIdPacient NOTIFY IdPacientChanged)
    Q_PROPERTY(int IdDocOrderEcho READ getIdDocOrderEcho WRITE setIdDocOrderEcho NOTIFY IdDocOrderEchoChanged)
    Q_PROPERTY(int IdUser READ getIdUser WRITE setIdUser NOTIFY IdUserChanged)
    Q_PROPERTY(int Post READ getPost WRITE setPost NOTIFY PostChanged)

    Q_PROPERTY(SectionsSystem sectionsSystem READ getSectionsSystem WRITE setSectionsSystem NOTIFY sectionsSystemChanged)

    Q_PROPERTY(bool CountImages READ getCountImages WRITE setCountImages NOTIFY CountImagesChanged)
    Q_PROPERTY(bool CountVideo READ getCountVideo WRITE setCountVideo NOTIFY CountVideoChanged)

public:

    enum SectionSystem {
        OrgansInternal = 0x1,
        UrinarySystem  = 0x2,
        Prostate       = 0x4,
        Gynecology     = 0x8,
        Breast         = 0x10,
        Thyroid        = 0x20,
        Gestation0     = 0x40,
        Gestation1     = 0x80,
        Gestation2     = 0x100,
        LymphNodes     = 0x200
    };
    Q_DECLARE_FLAGS(SectionsSystem, SectionSystem)
    Q_FLAG(SectionSystem)

    enum PageReport
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
        page_LymphNodes      = 9,
        page_images          = 10,
        page_video           = 11
    };
    Q_ENUM(PageReport)

    /**--------------------------------------------------------------------------------------*/

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

    SectionsSystem getSectionsSystem() const;
    void appendSectionSystem(SectionSystem f);    // adaugam doar un flag
    void appendSectionSystem(SectionsSystem f);   // adaugă mai multe flag-uri
    void setSectionsSystem(SectionsSystem s);  //

    void setIdUser(int IdUser) {m_idUser = IdUser; emit IdUserChanged();}
    int getIdUser() const {return m_idUser;}

    void setPost(int Post) {m_post = Post; emit PostChanged();}
    int getPost() const {return m_post;}

    void setCountImages(int CountImages){m_count_images = CountImages; emit CountImagesChanged();}
    int getCountImages() const {return m_count_images;}

    void setCountVideo(int CountVideo) {m_count_video = CountVideo; emit CountVideoChanged();}
    int getCountVideo() const {return m_count_video;}

    void onPrintDocument(Enums::TYPE_PRINT _typeReport, QString &filePDF); // functiile exportate pu solicitarea din alte clase

    QString getNumberDocReport();

    void m_onWritingData();

    Ui::DocReportEcho* uiPtr();              // acces controlat la UI
    const Ui::DocReportEcho* uiPtr() const;

    Q_SLOT void markModified(); // slot pu modificarea formei

    int getViewExaminationGestation(const int gest) const; // returneaza viewExamination (gestation0)

    void updateTextDescriptionDoppler();
    void updateDescriptionFetusWeight();
    QString getPercentageByDopplerUmbelicalArtery();
    QString getPercentageByDopplerUterineArteryLeft();
    QString getPercentageByDopplerUterineArteryRight();
    QString getPercentageByDopplerCMA();

    QStringList getAllRecommandation() const;
    void appendAllRecomandation(const QString text);

signals:
    void ItNewChanged();
    void IdChanged();
    void IdPacientChanged();
    void IdDocOrderEchoChanged();
    void sectionsSystemChanged();
    void IdUserChanged();
    void PostChanged();
    void PostDocument(); // pu actualizarea listei documentelor
    void mCloseThisForm();
    void CountImagesChanged(); // pu determinarea cantitatii imiginilor
    void CountVideoChanged();

public slots:
    void enforceMaxForSender(); // fn universala pu limitarea lungimei textului
    void updateTextConcluzionBySystem();

private slots:
    void slot_sectionsSystemChanged();

    void dataWasModified();    // modificarea datelor formei
    void updateTimer();        // actualizarea datei si orei in regim real
    void onDateTimeChanged();

    void onDateLMPChanged(QDate m_date); // pentru calcularea varstei gestationale

    //------------------------------------------------------------------------------------------
    /** functiile de inserare/eliminare a imaginelor*/
    bool loadFile(const QString &fileName, const int numberImage); // atasarea imaginilor
    void loadImageOpeningDocument();                               // extragerea imaginilor din BD

    void onLinkActivatedForOpenImg(const QString &link); // dialogul pu atasarea imaginilor
    void clickClearImage();                              // eliminarea imaginilor (universala)

    void insertImageIntoTableimagesReports(const QByteArray &imageData, const int numberImage);
    void updateImageIntoTableimagesReports(const QByteArray &imageData, const int numberImage);
    void removeImageIntoTableimagesReports(const int numberImage);
    void updateCommentIntoTableimagesReports();
    //...-------------------------------------

    void initConnections();  // initierea conectarilor + setarea stylului
    void connectionTemplateConcluzion();

    void openParameters();
    void openCatPatient();
    void openHistoryPatient();
    void openDocOrderEcho();

    void handlerOpenSelectTemplate();
    void insertTemplateIntoDB(const QString text, const QString name_system);
    void handlerAddTemplate();
    void insertTemplateConcluzionIntoDB(const QString text, const QString name_system);
    void handlerSelectTemplateConcluzion();
    void handlerAddTemplateConcluzion();

    /** Procesarea btn - navigare paginelor */
    void clickNavSystem(); // functia universala pu navigare paginilor
    void clickBtnComment();
    void clickBtnNormograms();

    void createMenuPrint();         // meniu print
    void clickOpenDesignerReport(); // deschidem designer
    void clickOpenPreviewReport();  // deschidem preview

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

    void clickedGestation2Trimestru(const int id_button);

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
    void onPrint(Enums::TYPE_PRINT _typeReport, QString &filePDF); // printare -> _typeReport = variabila pu determinarea lansarii designer ori preview din list...
    bool onWritingData();                // 'btnWrite'
    void onWritingDataClose();           // 'btnOk'
    void onClose();                      // 'btnClose'

private:
    void initInstallEventFilter();
    void initRequiredStructure();
    void initGroupRadioButton();
    void initSetStyleFrame();

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

    void initEnableBtn();                  // initierea accesului la btn + setarea focusului + paginei
    void updateStyleBtnInvestigations();
    void initSetCompleter();               // initierea completarului
    void initFooterDoc();                  // initierea btn + setarea textului autorului

    void updateDataDocOrderEcho(); // actualizarea datelor doc.Comanda ecografica - inserarea valorii atasarii imaginei

    DatabaseProvider *dbProvider();
    void initSyncDocs();

private:
    Ui::DocReportEcho *ui;

    std::unique_ptr<DocReportEchoHandler> m_handler;

    bool m_itNew          = false;
    int m_id              = Enums::IDX::IDX_UNKNOW;
    int m_idPacient       = Enums::IDX::IDX_UNKNOW;
    int m_id_docOrderEcho = Enums::IDX::IDX_UNKNOW;
    int m_idUser          = Enums::IDX::IDX_UNKNOW;
    int m_post            = Enums::IDX::IDX_UNKNOW;

    bool m_organs_internal;
    bool m_urinary_system;
    bool m_prostate;
    bool m_gynecology;
    bool m_breast;
    bool m_thyroide;
    bool m_gestation0;
    bool m_gestation1;
    bool m_gestation2;
    bool m_gestation3;
    bool m_lymphNodes;

    //---------------------------------------------------
    /** structura pu legatura btn si tab-pagina */
    struct TabPageRow {
        QAbstractButton* btn;
        PageReport pageIndex;
    };
    /** se completaeza prin functia setRowsForNavPage()*/
    std::vector<TabPageRow> rows_btn_page; // lista cu btn + index page

    //---------------------------------------------------
    /** structura atasarea/eliminarea imaginilor */
    struct Idx_Img {
        QToolButton* btn;
        QLabel* label_img;
        QPlainTextEdit* comment_img;
        int nr_img;
    };
    std::vector<Idx_Img> rows_items_images;

    //---------------------------------------------------
    /** structura pu legatura cu butone de 'adaugare' si 'selectare'
     ** a sabloanelor de descrierea a formatiunilor
     ** - QAction
     ** - LineEditCustom */
    struct TemplateSystemActions {
        //--- for LineEditCustom
        QAction*        action_select = nullptr;
        QAction*        action_add = nullptr;
        LineEditCustom* item_edit = nullptr;
        QString         name_system;
        SectionSystem   section_system;
    };
    std::vector<TemplateSystemActions> rows_templateSystem_action;

    //---------------------------------------------------
    /** structura pu legatura cu butone de 'adaugare' si 'selectare'
     ** a sabloanelor de descrierea a formatiunilor
     ** - QAbstractButton
     ** - QPlainTextEdit */
    struct TemplateSystemBtn {
        QAbstractButton* btn_select = nullptr;
        QAbstractButton* btn_add = nullptr;
        QPlainTextEdit*  item_edit = nullptr;
        QString          name_system;
        SectionSystem    section_system;
    };
    std::vector<TemplateSystemBtn> rows_templateSystem_btn;

    //---------------------------------------------------
    /** structura sabloanelor concluziilor ecografice
     ** pu legatura cu butoane + nume sistemului */
    struct TemplateConcluzions {
        QAbstractButton* btn_select = nullptr;
        QAbstractButton* btn_add = nullptr;
        QPlainTextEdit*  item_edit = nullptr;
        QString          name_system;
        SectionSystem    section_system;
    };
    std::vector<TemplateConcluzions> rows_template_concluzion;

    //---------------------------------------------------
    /** structura pu gruparea tuturor recomendatiilor
     ** item_edit + nume section_system */
    struct RecommandSystem {
        QLineEdit*    item_edit;
        SectionSystem section_system;
    };
    std::vector<RecommandSystem> rows_all_recommandation;

    /** structura pu grupe radiobutton */
    struct RadioBtnGroup {
        QButtonGroup* group;
        QRadioButton* btn_1 = nullptr;
        QRadioButton* btn_2 = nullptr;
        QRadioButton* btn_3 = nullptr;
        QRadioButton* btn_checked = nullptr;
    };
    std::vector<RadioBtnGroup> rows_radionBtn;

    /** lista Flag-lor documentului ce contine systeme ale organizmului */
    SectionsSystem m_sectionsSystem; // lista cu Flags (SectionSystem)

    int m_count_images = 0;

    DataBase *db;
    DatabaseProvider m_dbProvider;
    PopUp    *popUp;
    QTimer   *timer;

    QCompleter               *completer;
    QStandardItemModel       *modelPatients;
    BaseSortFilterProxyModel *proxyPatient;

    /** variabila pu determinarea stilului btn
     **  - selectare
     **  - adaugarea descrierilor formatiunilor si a concluziilor */
    QString style_toolButton;

    PatientHistory *history_patient;

    QPointer<QButtonGroup> group_btn_prostate;
    QPointer<QButtonGroup> group_btn_gynecology;
    QPointer<QButtonGroup> group_btn_gestation0;
    QPointer<QButtonGroup> group_btn_gestation1;
    QPointer<QButtonGroup> group_btn_gestation2;

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

    QStringList err;
    QStringList all_recommandation;
    InfoWindow *info_win;

    DataPercentage *data_percentage;

protected:
    void closeEvent(QCloseEvent *event);   // controlam modificarea datelor
    bool eventFilter(QObject *obj, QEvent *event);
    void changeEvent(QEvent *event);       // contolam traducerea aplicatiei
    void keyPressEvent(QKeyEvent *event);  // procedura de bypass a elementelor
                                           // (Qt::Key_Return | Qt::Key_Enter)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DocReportEcho::SectionsSystem)

#endif // DOCREPORTECHO_H
