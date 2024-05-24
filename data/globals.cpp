#include "globals.h"
#include <QCache>

namespace globals {

bool unknowModeLaunch = false;
int moveApp = -1; // unknow=-1, move=1, not_move=0

QString pathAppSettings          = nullptr;
QString pathImageBaseAppSettings = nullptr;
QString pathLogAppSettings       = nullptr;
QString pathTemplatesDocs        = nullptr;
QString pathReports              = nullptr;

QString sqliteNameBase = nullptr;
QString sqlitePathBase = nullptr;

QString mySQLhost          = nullptr;
QString mySQLnameBase      = nullptr;
QString mySQLport          = nullptr;
QString mySQLoptionConnect = nullptr;
QString mySQLuser          = nullptr;
QString mySQLpasswdUser    = nullptr;

QString fb_host          = nullptr;  // date de conectarea Firebird
QString fb_nameBase      = nullptr;
QString fb_port          = nullptr;
QString fb_optionConnect = nullptr;
QString fb_user          = nullptr;
QString fb_passwdUser    = nullptr;

bool thisMySQL  = false;
bool thisSqlite = false;

QString langApp  = nullptr;
bool firstLaunch = false;
int indexTypeSQL = -1;

bool showUserManual     = false; // prezentarea User manual
bool showHistoryVersion = false; // prezentarea History version

QString unitMeasure = nullptr;

int idUserApp = -1;
QString nameUserApp = nullptr;
bool memoryUser;

bool showQuestionCloseApp         = false;
bool order_splitFullName          = false;
int updateIntervalListDoc         = 0;
bool showDesignerMenuPrint        = false;
bool checkNewVersionApp           = false;
bool databasesArchiving           = false;
bool showAsistantHelper           = false;
bool showDocumentsInSeparatWindow = false;
bool minimizeAppToTray            = false;

int numSavedFilesLog = -1;

QString connectionMade = nullptr;
QPixmapCache cache_img;

QString pathDirectoryVideo = nullptr;

bool show_content_info_video = true;
bool show_info_reports       = true;

QString str_content_message_video = "<p style='font-family: Arial; font-size: 14px;'>"
                                    "Pentru lucru eficient cu fișiere video este necesar:<br><br>"
                                    "1. În <u>'setările aplicației'</u> de indicat <u>localizarea fișierelor video</u>.<br>"
                                    "Fișierele video se copie automat în directoriu de stocare la adăugarea în <u>lista cu video</u> din documentul "
                                    "<b><u>Raport ecografic</u></b>.<br><br>"
                                    "2. Fișiere video trebuie să fie convertate în formatul <b>'.mp4'</b>.<br><br>"
                                    "3. Denumirea fișierelor din directoriu de stocare (vezi punctul nr.1) nu trebuie să fie modificate (important !!!)."
                                    "La copierea automată, fișierelor este atașată denumirea(număr) pentru determinarea rapidă de către programa.</p>";

QString str_content_message_report = "<p style='font-family: Arial; font-size: 14px;'>"
                                     "Rapoarte:<br><br>"
                                     "1. Raport <b><u>'Lista pacienților (filtru organizații)'</u></b> - prezentarea datelor a pacienților (nume, prenume, anul nașterii și IDNP)"
                                     " , denumirea investigației ecografice cu indicarea codului și prețul investigației.<br>"
                                     "Filtrul raportului:"
                                     " <ul><li>Organizația - organizația ce a indicat examen ecografic.</li>"
                                     "     <li>Contract    - contractul organizației (exemplu - comercial sau CNAM).</li>"
                                     "     <li>Doctor      - doctorul ce a indicat examinarea ecografică.</li></ul>"
                                     "2. Raport <b><u>'Lista pacienților (filtru doctori)'</u></b> - prezentarea datelor a pacienților (nume, prenume, anul nașterii și IDNP)"
                                     " , denumirea investigației ecografice cu indicarea codului și prețul investigației.<br>"
                                     "Filtrul raportului:"
                                     " <ul><li>Doctor - doctorul ce a indicat examinarea ecografică.</li></ul>"
                                     "3. Raport <b><u>'Structura patologiilor'</u></b> - reprezintă evaluarea cantitativă a patologiilor ecografice.<br><br>"
                                     "4. Raport <b><u>'Volumul investigațiilor'</u></b> - reprezintă evaluarea cantitativă a investigațiilor ecografice.<br>"
                                     "Filtrul raportului:"
                                     " <ul><li>Organizația - organizația ce a indicat examen ecografic.</li>"
                                     "     <li>Contract    - contractul organizației (exemplu - comercial sau CNAM).</li></ul>";

}
