#include "data/mainwindow.h"
#include "data/databaseselection.h"
#include "data/version.h"

#include <QGuiApplication>
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 1))
#include <QDesktopWidget>
#else
#include <QScreen>
#endif

#include <QApplication>
#include <QMessageBox>
#include <QDebug>

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <QTextCodec>
#endif

#include <QTranslator>

// includem librarii pentru logare
#include <QScopedPointer>
#include <QTextStream>
#include <QDateTime>
#include <QLoggingCategory>
#include <QFile>
#include <QDir>
#include <QFontDatabase>
#include <QSplashScreen>
#include <QElapsedTimer>
#include <QPainter>

#include <data/database.h>
#include <data/globals.h>
#include <data/appsettings.h>
#include <catalogs/catusers.h>
#include "data/authorizationuser.h"
#include "data/initlaunch.h"

static const int LOAD_TIME_MSEC = 5 * 1000;

//******************************************************************************************************************************

QScopedPointer<QFile>   m_logFile;  // indicator pentru logare
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg); // declaram procesarea

//******************************************************************************************************************************

int resizeMainWindow(QApplication &a)
{
    MainWindow w;

    //--------------------------------------------------
    // schimbam dimensiunea ferestrei

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 1))
    QDesktopWidget *desktop = QApplication::desktop();

    int screenWidth = desktop->screenGeometry().width();
    int screenHeight = desktop->screenGeometry().height();
#else
    QScreen *screen = QGuiApplication::primaryScreen();
    int screenWidth  = screen->geometry().width();
    int screenHeight = screen->geometry().height();
#endif
    w.resize(screenWidth * 0.8, screenHeight * 0.8);

    //--------------------------------------------------
    // setam splash pe 10 sec

    QDir dir;
    QPixmap pixmap(dir.toNativeSeparators("icons/usg_splash.png"));
#if defined(Q_OS_LINUX)
    if (pixmap.isNull())
        pixmap.load(dir.toNativeSeparators("/usr/share/pixmaps/usg/usg_splash.png"));
#endif

    if (! pixmap.isNull()){
        QPainter painter(&pixmap);
        QFont font = painter.font();
        font.setPixelSize(18);
        font.setBold(true);
        painter.setFont(font);
#if defined(Q_OS_LINUX)
        painter.drawText(464, 94, QCoreApplication::tr("versiunea ") + VER);
        painter.drawText(536, 242, QCoreApplication::tr("autor:"));
        painter.drawText(378, 259, "alovada.med@gmail.com");
#elif defined(Q_OS_WIN)
        int id_font = QFontDatabase::addApplicationFont(":/Fonts/Cantarell-Regular.ttf");
        QString family = QFontDatabase::applicationFontFamilies(id_font).at(0);
        font.setFamily(family);

        painter.drawText(446, 94, QCoreApplication::tr("versiunea ") + VER);
        painter.drawText(530, 242, QCoreApplication::tr("autor:"));
        painter.drawText(358, 259, "alovada.med@gmail.com");
#endif

        QSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);
        splash.show();
        QTimer::singleShot(LOAD_TIME_MSEC, &splash, &QWidget::close); // keep displayed for 5 seconds
        QElapsedTimer time;
        time.start();
        while( time.elapsed() < LOAD_TIME_MSEC ) {
            const int progress = static_cast< double >( time.elapsed() ) / LOAD_TIME_MSEC * 100.0;
            splash.showMessage(QCoreApplication::tr( "Încărcat: %1%" ).arg( progress ), Qt::AlignBottom | Qt::AlignRight);
        }

        w.show(); // lansam feareastra principala
        splash.finish(&w);
    } else {
        qWarning(logWarning()) << QCoreApplication::tr("Nu a fost gasit fisierul - 'icons/usg_splash.png'");
        w.show(); // lansam feareastra principala
    }

    return a.exec();
}

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts); // initializare pluginului Qt WebEngine (specificul LimeReport)
    QApplication a(argc, argv);

    qInfo(logInfo()) << "=~=~=~=~=~=~=~=~=~=~=~ VERIFICATION SUPPORT OPENSSL =~=~=~=~=~=~=~=~=~=~=~=~";
    qInfo(logInfo()) << "Support SSL: " << QSslSocket::supportsSsl();
    qInfo(logInfo()) << "sslLibraryBuildVersionString: " << QSslSocket::sslLibraryBuildVersionString();
    qInfo(logInfo()) << "sslLibraryVersionString " << QSslSocket::sslLibraryVersionString();
    qInfo(logInfo()) << "";
    qInfo(logInfo()) << "=~=~=~=~=~=~=~=~=~=~ VERIFICATION SUPPORT SQL DRIVERS =~=~=~=~=~=~=~=~=~=~=~=";
    qInfo(logInfo()) << QSqlDatabase::drivers();
    qInfo(logInfo()) << "";


#if defined(Q_OS_WIN)
    qInfo(logInfo()) << "Load fonts 'Cantarell' for Windows.";
    QFontDatabase::addApplicationFont(":/Fonts/Cantarell-Regular.ttf");
    QFontDatabase::addApplicationFont(":/Fonts/Cantarell-Bold.ttf");
    QFontDatabase::addApplicationFont(":/Fonts/Cantarell-Oblique.ttf");
    QFontDatabase::addApplicationFont(":/Fonts/Cantarell-BoldOblique.ttf");
#endif

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("cp8859"));
#endif
    //******************************************************************************************************************************
    // setam stilul a aplicatiei
    qInfo(logInfo()) << "Load style applications.";
    QFile fileStyle(":/style.css");
    fileStyle.open(QFile::ReadOnly);
    QString appStyle(fileStyle.readAll());  // fisierul cu stilul aplicatiei
    a.setStyleSheet(appStyle);              // setam stilul
    // a.setStyle("Fusion");

    //******************************************************************************************************************************
    // alegem fisierul cu setari
    qInfo(logInfo()) << "Open database selection.";
    DatabaseSelection _select;
    if (_select.exec() != QDialog::Accepted)
        return 0;

    //******************************************************************************************************************************
    // initializam setarile aplicatiei
    qInfo(logInfo()) << "Load settings application.";
    AppSettings* appSettings = new AppSettings();    // alocam memoria p-u setarile aplicatiei

    // instalam fisierul de logare
    if (QString(argv[1]) == "" && QString(argv[1]) == "/debug") { // !=
        m_logFile.reset(new QFile(globals::pathLogAppSettings));
        if (m_logFile.data()->open(QFile::Append | QFile::Text)){
            qInstallMessageHandler(messageHandler);
        }
    }

    //******************************************************************************************************************************
    // determinam modul de lansare a aplicatiei
    if (globals::unknowModeLaunch){                       // nu e determinat modul lansarii aplicatiei
        InitLaunch* initLaunch = new InitLaunch();        // initializam forma interogarii lansarii aplicatiei: 'prima lansare' sau 'baza de date a fost transferata'
        initLaunch->setAttribute(Qt::WA_DeleteOnClose);   // setam atributul de eliberare a memoriei la inchiderea ferestrei
        if (initLaunch->exec() != QDialog::Accepted){
            delete appSettings;
            return 1;
        }
        QTranslator* translator = new QTranslator();                                                                          // daca limba a fost schimbata
        if (translator->load(QLocale(globals::langApp), QLatin1String("USG"), QLatin1String("_"), QLatin1String(":/i18n"))) { // setam limba aplicatiei
            qApp->installTranslator(translator);
        }
        appSettings->setLanguageApp(); // setam limba aplicatiei daca a fost schimbata
    }

    //******************************************************************************************************************************
    // 1. Transferul BD
    // 2. Prima lansare a aplicatiei
    // 3. Exista fisierul cu BD + fisierul cu setarile principale ale aplicatiei
    if (globals::moveApp == 1){                              // 1. daca BD a fost transferata
        if (appSettings->exec() != QDialog::Accepted)        // deschidem setarile aplicatiei pu a seta drumul spre BD
            return 1;
        AuthorizationUser* authUser = new AuthorizationUser(); // lansam autorizarea
        authUser->setProperty("Id", globals::idUserApp);       // setam proprietatea 'id'
        if (authUser->exec() != QDialog::Accepted)
            return 1;
        resizeMainWindow(a);  // resetam dimensiunile ferestrei principale
    } else {
        if (globals::firstLaunch){                           // 2. daca prima lansare a aplicatie
            if (appSettings->exec() != QDialog::Accepted)    // lansam setarile principale ale aplicatiei
                return 1;
            CatUsers* catUsers = new CatUsers();             // cream utilizator nou
            catUsers->setAttribute(Qt::WA_DeleteOnClose);    // setam atributul de eliberare a memoriei la inchiderea ferestrei
            catUsers->setProperty("ItNew",   true);          // setam proprietatea 'itNew'
            catUsers->setWindowTitle(QCoreApplication::tr("Crearea administratorului aplicației %1").arg("[*]"));
            if (catUsers->exec() != QDialog::Accepted)
                return 1;
            auto db = new DataBase();
            appSettings->setKeyAndValue("on_start", "idUserApp",   db->encode_string(QString::number(globals::idUserApp))); // in fisierul cu setari pentru extragerea
            appSettings->setKeyAndValue("on_start", "nameUserApp", db->encode_string(globals::nameUserApp));                // ulterioara la logare urmatoare a aplicatiei
            appSettings->setKeyAndValue("on_start", "memoryUser",  (globals::memoryUser) ? 1 : 0);
            delete db;
            resizeMainWindow(a); // resetam dimensiunile ferestrei principale
        } else {                                             // 3. exista fisierul cu BD + fisierul cu setarile principale ale aplicatiei
            appSettings->loadSettings();                     // determinam/incarcam setarile principale ale aplicatiei

            AuthorizationUser* authUser = new AuthorizationUser();  // lansam autorizarea
            authUser->setProperty("Id", globals::idUserApp);
            if (authUser->exec() != QDialog::Accepted)
                return 1;
            auto db = new DataBase();
            appSettings->setKeyAndValue("on_start", "idUserApp",   db->encode_string(QString::number(globals::idUserApp))); // in fisierul cu setari pentru extragerea
            appSettings->setKeyAndValue("on_start", "nameUserApp", db->encode_string(globals::nameUserApp));                // ulterioara la logare urmatoare a aplicatiei
            appSettings->setKeyAndValue("on_start", "memoryUser",  (globals::memoryUser) ? 1 : 0);
            delete db;
            resizeMainWindow(a);
        }
    }
    delete appSettings;
}

// realizarea procesarii
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QTextStream out(m_logFile.data());                                        // Initerea fluxului datelor pu inscriere
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz "); // adaugam data
    // Dupa tipul debug-ului determinam nivelul mesajului
    switch (type)
    {
    case QtInfoMsg:     out << "INF "; break;
    case QtDebugMsg:    out << "DBG "; break;
    case QtWarningMsg:  out << "WRN "; break;
    case QtCriticalMsg: out << "CRT "; break;
    case QtFatalMsg:    out << "FTL "; break;
    }
    out << context.category << ": "
        << msg << Qt::endl; // Inscrim categoria si mesajul
    out.flush();            // golim datele din bufer
}
