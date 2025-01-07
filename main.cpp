#include <QApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QMessageBox>
#include <QScreen>
#include <QStyleHints>
#include <QTranslator>

// includem librarii pentru logare
#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFontDatabase>
#include <QLoggingCategory>
#include <QPainter>
#include <QScopedPointer>
#include <QSplashScreen>
#include <QTextStream>

#include "data/databaseselection.h"
#include "data/mainwindow.h"
#include "data/authorizationuser.h"
#include "data/initlaunch.h"
#include <catalogs/catusers.h>
#include <data/appsettings.h>
#include <data/database.h>
#include <data/globals.h>

static const int LOAD_TIME_MSEC = 5 * 1000;

//******************************************************************************************************************************

QScopedPointer<QFile> m_logFile; // indicator pentru logare
void messageHandler(QtMsgType type,
                    const QMessageLogContext &context,
                    const QString &msg); // declaram procesarea

//******************************************************************************************************************************

#if defined(Q_OS_WIN)

bool isDarkThemeOnWindows() {
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
    return settings.value("AppsUseLightTheme", 1).toInt() == 0; // 0 înseamnă dark theme, 1 înseamnă light theme
}

#elif defined(Q_OS_MACOS)

extern void setupCustomAppDelegate();
extern bool isDarkModeEnabledMacOS();

#elif defined(Q_OS_LINUX)

bool isDarkThemeOnLinux() {
    QProcess process;
    process.start("gsettings", QStringList() << "get" << "org.gnome.desktop.interface" << "gtk-theme");
    process.waitForFinished();

    QString theme = process.readAllStandardOutput();
    return theme.contains("dark", Qt::CaseInsensitive); // Verificăm dacă tema include cuvântul „dark”
}

#endif

//******************************************************************************************************************************

int resizeMainWindow(QApplication &a)
{
    MainWindow w;

    //--------------------------------------------------
    // schimbam dimensiunea ferestrei
    QScreen *screen = QGuiApplication::primaryScreen();

#if defined(Q_OS_LINUX)

    int screenWidth = screen->geometry().width();
    int screenHeight = screen->geometry().height();
    w.resize(screenWidth * 0.8, screenHeight * 0.8);

#elif defined(Q_OS_WIN)

    int screenWidth = screen->geometry().width();
    int screenHeight = screen->geometry().height();
    w.resize(screenWidth * 0.8, screenHeight * 0.8);

#elif defined(Q_OS_MACOS)

    QRect availableGeometry = screen->availableGeometry();
    int screenWidth = availableGeometry.width();
    int screenHeight = availableGeometry.height();

    // Setează dimensiuni minime și maxime
    w.setMinimumSize(screenWidth * 0.5, screenHeight * 0.5);
    w.setMaximumSize(screenWidth, screenHeight);

#endif

    //--------------------------------------------------
    // setam splash pe 10 sec

    QDir dir;
    QPixmap pixmap;
    QDate current_date = QDate::currentDate();
    int current_year = current_date.year();
    if (current_date >= QDate(current_year, 12, 15) || current_date <= QDate(current_year, 01, 15))
        pixmap.load(":/icons/splash_santa.png");
    else if (current_date >= QDate(current_date.year(), 12, 1)
             || current_date <= QDate(current_year, 03, 01).addDays(-1))
        pixmap.load(":/icons/splash_snow.png");
    else
        pixmap.load(":/icons/usg_splash.png");

#if defined(Q_OS_LINUX)
    if (pixmap.isNull())
        pixmap.load(dir.toNativeSeparators("/usr/share/pixmaps/usg/usg_splash.png"));
#endif

    if (!pixmap.isNull()) {
        QPainter painter(&pixmap);
        QFont font = painter.font();
        font.setBold(true);

#if defined(Q_OS_LINUX)
        font.setPixelSize(18);
#elif defined(Q_OS_WIN)
        font.setFamily("Segoe UI");
#endif
        painter.setFont(font); // Setăm fontul înainte de a desena textul

#if defined(Q_OS_LINUX)
        font.setPointSize(20);
        painter.setFont(font);
        painter.drawText(96, 44, QCoreApplication::tr("USG - Evidența examinărilor ecografice"));
        font.setPointSize(11);
        painter.setFont(font);
        painter.drawText(464, 74, QCoreApplication::tr("versiunea ") + USG_VERSION_FULL);
        painter.drawText(536, 242, QCoreApplication::tr("autor:"));
        painter.drawText(400, 259, USG_COMPANY_EMAIL);
        font.setPointSize(9);
        painter.setFont(font);
        painter.drawText(290,
                         310,
                         "2021 - " + QString::number(QDate::currentDate().year())
                             + QCoreApplication::tr(" a."));
#elif defined(Q_OS_WIN)
        font.setPointSize(18);
        painter.setFont(font);
        painter.drawText(132, 48, QCoreApplication::tr("USG - Evidența examinărilor ecografice"));
        font.setPointSize(11);
        painter.setFont(font);
        painter.drawText(464, 68, QCoreApplication::tr("versiunea ") + USG_VERSION_FULL);
        painter.drawText(534, 242, QCoreApplication::tr("autor:"));
        painter.drawText(400, 259, USG_COMPANY_EMAIL);
        font.setPointSize(9);
        painter.setFont(font);
        painter.drawText(290,
                         310,
                         "2021 - " + QString::number(QDate::currentDate().year())
                             + QCoreApplication::tr(" a."));
#elif defined(Q_OS_MACOS)
        font.setPointSize(22);
        painter.setFont(font);
        painter.drawText(182, 44, QCoreApplication::tr("USG - Evidența examinărilor ecografice"));
        font.setPointSize(13);
        painter.setFont(font);
        painter.drawText(468, 74, QCoreApplication::tr("versiunea ") + USG_VERSION_FULL);
        painter.drawText(530, 242, QCoreApplication::tr("autor:"));
        painter.drawText(408, 259, USG_COMPANY_EMAIL);
        font.setPointSize(10);
        painter.setFont(font);
        painter.drawText(290,
                         310,
                         "2021 - " + QString::number(QDate::currentDate().year())
                             + QCoreApplication::tr(" a."));
#endif

        QSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);
        splash.show();
        QTimer::singleShot(LOAD_TIME_MSEC, &splash, &QWidget::close); // keep displayed for 5 seconds
        QElapsedTimer time;
        time.start();
        while (time.elapsed() < LOAD_TIME_MSEC) {
            const int progress = static_cast<double>(time.elapsed()) / LOAD_TIME_MSEC * 100.0;
            splash.showMessage(QCoreApplication::tr("Încărcat: %1%").arg(progress),
                               Qt::AlignBottom | Qt::AlignRight);
        }

        w.show(); // lansam feareastra principala

#if defined(Q_OS_MACOS)

        // Redimensionează fereastra la 80% din dimensiunea ecranului
        w.resize(screenWidth * 0.8, screenHeight * 0.8);

        // Calculează poziția pentru centrare
        int windowWidth = w.width();
        int windowHeight = w.height();
        int posX = availableGeometry.x() + (screenWidth - windowWidth) / 2;
        int posY = availableGeometry.y() + (screenHeight - windowHeight) / 2;

        // Mută fereastra în centru
        w.move(posX, posY);
#endif

        splash.finish(&w);
    } else {
        qWarning(logWarning()) << QCoreApplication::tr(
            "Nu a fost gasit fisierul - 'icons/usg_splash.png'");
        w.show(); // lansam feareastra principala
    }

    return a.exec();
}

int main(int argc, char *argv[])
{
#if defined(Q_OS_MACOS)
    setupCustomAppDelegate(); // Configurează delegate-ul pentru a elimina eroarea - vezi fisierul AppDelegate.mm
#endif

    QApplication::setAttribute(
        Qt::AA_ShareOpenGLContexts); // initializare pluginului Qt WebEngine (specificul LimeReport)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::Floor);

    QApplication a(argc, argv);

#if defined(Q_OS_LINUX)
    int fontId = QFontDatabase::addApplicationFont(":/Fonts/segoeUI/segoeui.ttf");
    QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
    QFont font(family); // Setează mărimea dorită
    font.setPointSize(11);
    QApplication::setFont(font); // Setează fontul pentru întreaga aplicație
#elif defined(Q_OS_WIN)
    int fontId = QFontDatabase::addApplicationFont(":/Fonts/segoeUI/segoeui.ttf");
    QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
    QFont font(family); // Setează mărimea dorită
    font.setPointSize(10);
    QApplication::setFont(font); // Setează fontul pentru întreaga aplicație
#elif defined(Q_OS_MACOS)
    QFont font("San Francisco", 13); // Setează mărimea dorită
    QApplication::setFont(font);     // Setează fontul pentru întreaga aplicație
#endif

    qInfo(logInfo())
        << "=~=~=~=~=~=~=~=~=~=~=~ VERIFICATION SUPPORT OPENSSL =~=~=~=~=~=~=~=~=~=~=~=~";
    qInfo(logInfo()) << "Support SSL: " << QSslSocket::supportsSsl();
    qInfo(logInfo()) << "sslLibraryBuildVersionString: "
                     << QSslSocket::sslLibraryBuildVersionString();
    qInfo(logInfo()) << "sslLibraryVersionString " << QSslSocket::sslLibraryVersionString();
    qInfo(logInfo()) << "";
    qInfo(logInfo())
        << "=~=~=~=~=~=~=~=~=~=~ VERIFICATION SUPPORT SQL DRIVERS =~=~=~=~=~=~=~=~=~=~=~=";
    qInfo(logInfo()) << QSqlDatabase::drivers();
    qInfo(logInfo()) << "";

    //******************************************************************************************************************************
    // setam stilul a aplicatiei

    a.setStyle("Fusion");

    QFile fileStyle;

#if defined(Q_OS_LINUX)
    if (isDarkThemeOnLinux()) {
        fileStyle.setFileName(":/styles/style_dark.qss");
        globals::isSystemThemeDark = true;
    } else {
        fileStyle.setFileName(":/styles/style_main.qss");
        globals::isSystemThemeDark = false;
    }
#elif defined(Q_OS_WIN)
    if (isDarkThemeOnWindows()) {
        fileStyle.setFileName(":/styles/style_dark.qss");
        globals::isSystemThemeDark = true;
    } else {
        fileStyle.setFileName(":/styles/style_main.qss");
        globals::isSystemThemeDark = false;
    }
#elif defined(Q_OS_MACOS)
    if (isDarkModeEnabledMacOS()) {
        fileStyle.setFileName(":/styles/style_dark.qss");
        globals::isSystemThemeDark = true;
    } else {
        fileStyle.setFileName(":/styles/style_main.qss");
        globals::isSystemThemeDark = false;
    }
#endif
    fileStyle.open(QFile::ReadOnly);
    QString appStyle(fileStyle.readAll()); // fisierul cu stilul aplicatiei
    a.setStyleSheet(appStyle);             // setam stilul

    qInfo(logInfo()) << "Load style applications.";

    //******************************************************************************************************************************
    // alegem fisierul cu setari
    qInfo(logInfo()) << "Open database selection.";
    DatabaseSelection _select;
    if (_select.exec() != QDialog::Accepted)
        return 0;

    //******************************************************************************************************************************
    // initializam setarile aplicatiei
    qInfo(logInfo()) << "Load settings application.";
    AppSettings *appSettings = new AppSettings(); // alocam memoria p-u setarile aplicatiei

    // instalam fisierul de logare
    if (QString(argv[1]) == "" && QString(argv[1]) == "/debug") { // !=
        m_logFile.reset(new QFile(globals::pathLogAppSettings));
        if (m_logFile.data()->open(QFile::Append | QFile::Text)) {
            qInstallMessageHandler(messageHandler);
        }
    }

    //******************************************************************************************************************************
    // determinam modul de lansare a aplicatiei
    if (globals::unknowModeLaunch) { // nu e determinat modul lansarii aplicatiei
        InitLaunch *initLaunch
            = new InitLaunch(); // initializam forma interogarii lansarii aplicatiei: 'prima lansare' sau 'baza de date a fost transferata'
        initLaunch->setAttribute(
            Qt::WA_DeleteOnClose); // setam atributul de eliberare a memoriei la inchiderea ferestrei
        if (initLaunch->exec() != QDialog::Accepted) {
            // appSettings->deleteLater();
            return 1;
        }
        QTranslator *translator = new QTranslator(); // daca limba a fost schimbata
        if (translator->load(QLocale(globals::langApp),
                             QLatin1String("USG"),
                             QLatin1String("_"),
                             QLatin1String(":/i18n"))) { // setam limba aplicatiei
            qApp->installTranslator(translator);
        }
        appSettings->setLanguageApp(); // setam limba aplicatiei daca a fost schimbata
    }

    //******************************************************************************************************************************
    // 1. Transferul BD
    // 2. Prima lansare a aplicatiei
    // 3. Exista fisierul cu BD + fisierul cu setarile principale ale aplicatiei
    if (globals::moveApp == 1) { // 1. daca BD a fost transferata
        if (appSettings->exec()
            != QDialog::Accepted) // deschidem setarile aplicatiei pu a seta drumul spre BD
            return 1;
        AuthorizationUser *authUser = new AuthorizationUser(); // lansam autorizarea
        authUser->setProperty("Id", globals::idUserApp);       // setam proprietatea 'id'
        if (authUser->exec() != QDialog::Accepted)
            return 1;
        resizeMainWindow(a); // resetam dimensiunile ferestrei principale
    } else {
        if (globals::firstLaunch) { // 2. daca prima lansare a aplicatie
            if (appSettings->exec()
                != QDialog::Accepted) // lansam setarile principale ale aplicatiei
                return 1;
            CatUsers *catUsers = new CatUsers(); // cream utilizator nou
            catUsers->setAttribute(
                Qt::WA_DeleteOnClose); // setam atributul de eliberare a memoriei la inchiderea ferestrei
            catUsers->setProperty("ItNew", true); // setam proprietatea 'itNew'
            catUsers->setWindowTitle(
                QCoreApplication::tr("Crearea administratorului aplicației %1").arg("[*]"));
            if (catUsers->exec() != QDialog::Accepted)
                return 1;
            auto db = new DataBase();
            appSettings
                ->setKeyAndValue("on_start",
                                 "idUserApp",
                                 db->encode_string(QString::number(
                                     globals::idUserApp))); // in fisierul cu setari pentru extragerea
            appSettings->setKeyAndValue(
                "on_start",
                "nameUserApp",
                db->encode_string(
                    globals::nameUserApp)); // ulterioara la logare urmatoare a aplicatiei
            appSettings->setKeyAndValue("on_start", "memoryUser", (globals::memoryUser) ? 1 : 0);
            qDebug() << "thisSqlite: " << globals::thisSqlite;
            qDebug() << "thisMySQL: " << globals::thisMySQL;
            qDebug() << "user: " << globals::nameUserApp;
            qDebug() << "id user: " << globals::idUserApp;
            qDebug() << "lang app: " << globals::langApp;
            qDebug() << "index type SQL:" << globals::indexTypeSQL;
            resizeMainWindow(a); // resetam dimensiunile ferestrei principale
            db->deleteLater();
        } else { // 3. exista fisierul cu BD + fisierul cu setarile principale ale aplicatiei
            appSettings->loadSettings(); // determinam/incarcam setarile principale ale aplicatiei
            qDebug() << "thisSqlite: " << globals::thisSqlite;
            qDebug() << "thisMySQL: " << globals::thisMySQL;
            qDebug() << "user: " << globals::nameUserApp;
            qDebug() << "id user: " << globals::idUserApp;
            qDebug() << "lang app: " << globals::langApp;
            qDebug() << "index type SQL:" << globals::indexTypeSQL;
            AuthorizationUser *authUser = new AuthorizationUser(); // lansam autorizarea
            authUser->setProperty("Id", globals::idUserApp);
            if (authUser->exec() != QDialog::Accepted)
                return 1;
            auto db = new DataBase();
            appSettings
                ->setKeyAndValue("on_start",
                                 "idUserApp",
                                 db->encode_string(QString::number(
                                     globals::idUserApp))); // in fisierul cu setari pentru extragerea
            appSettings->setKeyAndValue(
                "on_start",
                "nameUserApp",
                db->encode_string(
                    globals::nameUserApp)); // ulterioara la logare urmatoare a aplicatiei
            appSettings->setKeyAndValue("on_start", "memoryUser", (globals::memoryUser) ? 1 : 0);
            resizeMainWindow(a);
            db->deleteLater();
        }
    }
    appSettings->deleteLater();
}

// realizarea procesarii
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QTextStream out(m_logFile.data()); // Initerea fluxului datelor pu inscriere
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz "); // adaugam data
    // Dupa tipul debug-ului determinam nivelul mesajului
    switch (type) {
    case QtInfoMsg:
        out << "INF ";
        break;
    case QtDebugMsg:
        out << "DBG ";
        break;
    case QtWarningMsg:
        out << "WRN ";
        break;
    case QtCriticalMsg:
        out << "CRT ";
        break;
    case QtFatalMsg:
        out << "FTL ";
        break;
    }
    out << context.category << ": " << msg << Qt::endl; // Inscrim categoria si mesajul
    out.flush();                                        // golim datele din bufer
}
