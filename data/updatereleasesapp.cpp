#include "updatereleasesapp.h"

UpdateReleasesApp::UpdateReleasesApp(QObject *parent) : QObject(parent)
{
    db = new DataBase(this);
}

UpdateReleasesApp::~UpdateReleasesApp()
{
    delete db;
}

bool UpdateReleasesApp::execUpdateCurrentRelease(const QString current_release)
{
    if (current_release == release_1_1_3){

        QSqlQuery qry;
        qry.prepare("CREATE TABLE sqlitestudio_temp_table AS SELECT * FROM settingsUsers;");
        if (! qry.exec()){
            qCritical(logCritical()) << tr("Nu este efectuata actualizarea - ") + qry.lastError().text();
            return false;
        }

        qry.prepare("DROP TABLE settingsUsers;");
        if (! qry.exec()){
            qCritical(logCritical()) << tr("Nu este efectuata actualizarea - ") + qry.lastError().text();
            return false;
        }

        qry.prepare("CREATE TABLE settingsUsers ("
                    "    id                    INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "    id_users              INT NOT NULL,"
                    "    owner                 TEXT,"
                    "    nameOption            TEXT,"
                    "    versionApp            TEXT,"
                    "    showQuestionCloseApp  INT,"
                    "    showUserManual        INT,"
                    "    showHistoryVersion    INT,"
                    "    order_splitFullName   INT,"
                    "    updateListDoc         TEXT,"
                    "    showDesignerMenuPrint INT"
                    ");");
        if (! qry.exec()){
            qCritical(logCritical()) << tr("Nu este efectuata actualizarea - ") + qry.lastError().text();
            return false;
        }

        qry.prepare("INSERT INTO settingsUsers ("
                    "    id,"
                    "    id_users,"
                    "    owner,"
                    "    nameOption,"
                    "    versionApp,"
                    "    showQuestionCloseApp,"
                    "    showUserManual,"
                    "    showHistoryVersion,"
                    "    order_splitFullName,"
                    "    updateListDoc)"
                    "       SELECT id,"
                    "          id_users,"
                    "          owner,"
                    "          nameOption,"
                    "          versionApp,"
                    "          showQuestionCloseApp,"
                    "          showUserManual,"
                    "          showHistoryVersion,"
                    "          order_splitFullName,"
                    "         updateListDoc"
                    "       FROM sqlitestudio_temp_table;");
        if (! qry.exec()){
            qCritical(logCritical()) << tr("Nu este efectuata actualizarea - ") + qry.lastError().text();
            return false;
        }

        qry.prepare("DROP TABLE sqlitestudio_temp_table;");
        if (! qry.exec()){
            qCritical(logCritical()) << tr("Nu este efectuata actualizarea - ") + qry.lastError().text();
            return false;
        }

        qry.prepare("INSERT INTO settingsUsers (id, id_users, owner, nameOption, versionApp, showQuestionCloseApp, showUserManual, showHistoryVersion, order_splitFullName, updateListDoc, showDesignerMenuPrint) "
                                    "VALUES (?,?,?,?,?,?,?,?,?,?,?);");
        qry.addBindValue(QString::number(7));
        qry.addBindValue(QString::number(globals::idUserApp));
        qry.addBindValue("Setări a documentelor");
        qry.addBindValue("prezentarea designerului(LimeReport) în meniu de printare");
        qry.addBindValue(QVariant());
        qry.addBindValue(QVariant());
        qry.addBindValue(QVariant());
        qry.addBindValue(QVariant());
        qry.addBindValue(QVariant());
        qry.addBindValue(QVariant());
        qry.addBindValue(QString::number(0));
        if (! qry.exec()){
            qCritical(logCritical()) << tr("Eroare de inserare in tabela 'settingsUsers' in timpul actualizarii la versiunea '%1' - %2").arg(current_release, qry.lastError().text());
            return false;
        }
    }

    if (current_release == release_1_1_4) {

        QSqlQuery qry;
        qry.prepare("CREATE TABLE sqlitestudio_temp_table0 AS SELECT * FROM orderEcho;");
        if (! qry.exec()){
            qCritical(logCritical()) << tr("Eroare de actualizare a relizului '" release_1_1_4 "' (crearea tabelei 'sqlitestudio_temp_table0'): ") + qry.lastError().text();
            return false;
        }

        qry.prepare("DROP TABLE orderEcho;");
        if (! qry.exec()){
            qCritical(logCritical()) << tr("Eroare de actualizare a relizului '" release_1_1_4 "' (eliminarea tabelei 'orderEcho'): ") + qry.lastError().text();
            return false;
        }

        qry.prepare("CREATE TABLE orderEcho ("
                    "id               INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                    "deletionMark     INTEGER NOT NULL,"
                    "numberDoc        TEXT,"
                    "dateDoc          TEXT,"
                    "id_organizations INTEGER,"
                    "id_contracts     INTEGER,"
                    "id_typesPrices   INTEGER,"
                    "id_doctors       INTEGER,"
                    "id_pacients      INTEGER,"
                    "id_users         INTEGER,"
                    "sum              REAL,"
                    "comment          TEXT,"
                    "cardPayment      INTEGER,"
                    "attachedImages   INTEGER"
                ");");
        if (! qry.exec()){
            qCritical(logCritical()) << tr("Eroare de actualizare a relizului '" release_1_1_4 "' (crearea tabelei noi 'orderEcho'): ") + qry.lastError().text();
            return false;
        }

        qry.prepare("INSERT INTO orderEcho ("
                    "id,"
                    "deletionMark,"
                    "numberDoc,"
                    "dateDoc,"
                    "id_organizations,"
                    "id_contracts,"
                    "id_typesPrices,"
                    "id_doctors,"
                    "id_pacients,"
                    "id_users,"
                    "sum,"
                    "comment"
                ")"
                "SELECT id,"
                       "deletionMark,"
                       "numberDoc,"
                       "dateDoc,"
                       "id_organizations,"
                       "id_contracts,"
                       "id_typesPrices,"
                       "id_doctors,"
                       "id_pacients,"
                       "id_users,"
                       "sum,"
                       "comment "
                  "FROM sqlitestudio_temp_table0;");
        if (! qry.exec()){
            qCritical(logCritical()) << tr("Eroare de actualizare a relizului '" release_1_1_4 "' (inserarea datelor din tabela 'sqlitestudio_temp_table0' in tabela 'orderEcho'): ") + qry.lastError().text();
            return false;
        }

        qry.prepare("DROP TABLE sqlitestudio_temp_table0;");
        if (! qry.exec()){
            qCritical(logCritical()) << tr("Eroare de actualizare a relizului '" release_1_1_4 "' (eliminarea tabelei 'sqlitestudio_temp_table0'): ") + qry.lastError().text();
            return false;
        }

        QSqlQuery qry_image(db->getDatabaseImage());
        qry_image.prepare("SELECT id_orderEcho FROM imagesReports;");
        if (qry_image.exec()){
            while (qry_image.next()) {
                qry.prepare(QString("UPDATE orderEcho SET attachedImages = 1 WHERE id = '%1';").arg(qry_image.value(0).toString()));
                if (! qry.exec())
                    qCritical(logCritical()) << tr("Eroare de actualizare a documentului 'Comanda ecografica' (inserarea atasarii imaginelor): ") + qry.lastError().text();
            }

        }

        qry.prepare("UPDATE orderEcho SET attachedImages = 0 WHERE attachedImages ISNULL;");
        if (! qry.exec())
            qCritical(logCritical()) << tr("Nu a fost resetata valoarea 'o' la documente cu imaginile neatasate");


        qry.prepare("UPDATE orderEcho SET cardPayment = 1 WHERE comment IN ('card', 'achitat cu card');");
        if (! qry.exec())
            qCritical(logCritical()) << tr("Eroare de actualizare a documentului 'Comanda ecografica' (actualizarea datelor achitarii cu card): ") + qry.lastError().text();

        qry.prepare("UPDATE orderEcho SET cardPayment = 0 WHERE comment NOT IN ('card', 'achitat cu card');");
        if (! qry.exec())
            qCritical(logCritical()) << tr("Eroare de actualizare a documentului 'Comanda ecografica' (actualizarea datelor achitarii cu card): ") + qry.lastError().text();

        qry.prepare("SELECT id, id_typesPrices FROM orderEcho WHERE cardPayment = 0;");
        if (qry.exec()) {
            while (qry.next()) {
                const int id_order = qry.value(0).toInt();
                const int id_typesPrices = qry.value(1).toInt();
                int value_payment = 0;

                QSqlQuery qry_type_price;
                qry_type_price.prepare(QString("SELECT noncomercial FROM typesPrices WHERE id = '%1' AND deletionMark = '0';").arg(QString::number(id_typesPrices)));
                if (qry_type_price.exec()){
                    qry_type_price.next();
                    if (qry_type_price.value(0).toInt() > 0)
                        value_payment = 2;
                }

                QSqlQuery qry_order;
                qry_order.prepare(QString("UPDATE orderEcho SET cardPayment = '%1' WHERE id = '%2';").arg(QString::number(value_payment), QString::number(id_order)));
                if (! qry_order.exec())
                    qCritical(logCritical()) << tr("Nu a fost modificat statut de achitare a documentului cu id='%1'").arg(QString::number(id_order));
            }
        }

        //*********************************************************************

        qry.prepare("CREATE TABLE sqlitestudio_temp_table0 AS SELECT * FROM reportEcho;");
        if (! qry.exec()){
            qCritical(logCritical()) << tr("Eroare de actualizare a relizului '" release_1_1_4 "' (crearea tabelei 'sqlitestudio_temp_table0'): ") + qry.lastError().text();
            return false;
        }

        qry.prepare("DROP TABLE reportEcho;");
        if (! qry.exec()){
            qCritical(logCritical()) << tr("Eroare de actualizare a relizului '" release_1_1_4 "' (eliminarea tabelei 'orderEcho'): ") + qry.lastError().text();
            return false;
        }

        qry.prepare("CREATE TABLE reportEcho ("
                    "id                INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "deletionMark      INTEGER,"
                    "numberDoc         TEXT,"
                    "dateDoc           TEXT,"
                    "id_pacients       INTEGER,"
                    "id_orderEcho      INTEGER,"
                    "t_organs_internal INTEGER,"
                    "t_urinary_system  INTEGER,"
                    "t_prostate        INTEGER,"
                    "t_gynecology      INTEGER,"
                    "t_breast          INTEGER,"
                    "t_thyroid         INTEGER,"
                    "t_gestation0      INTEGER,"
                    "t_gestation1      INTEGER,"
                    "t_gestation2      INTEGER,"
                    "t_gestation3      INTEGER,"
                    "id_users          INTEGER,"
                    "concluzion        TEXT,"
                    "comment           TEXT,"
                    "attachedImages    INTEGER"
                ");");
        if (! qry.exec()){
            qCritical(logCritical()) << tr("Eroare de actualizare a relizului '" release_1_1_4 "' (crearea tabelei noi 'orderEcho'): ") + qry.lastError().text();
            return false;
        }

        qry.prepare("INSERT INTO reportEcho ("
                    "id,"
                    "deletionMark,"
                    "numberDoc,"
                    "dateDoc,"
                    "id_pacients,"
                    "id_orderEcho,"
                    "t_organs_internal,"
                    "t_urinary_system,"
                    "t_prostate,"
                    "t_gynecology,"
                    "t_breast,"
                    "t_thyroid,"
                    "t_gestation0,"
                    "t_gestation1,"
                    "t_gestation2,"
                    "t_gestation3,"
                    "id_users,"
                    "concluzion,"
                    "comment"
                ")"
                "SELECT id,"
                       "deletionMark,"
                       "numberDoc,"
                       "dateDoc,"
                       "id_pacients,"
                       "id_orderEcho,"
                       "t_organs_internal,"
                       "t_urinary_system,"
                       "t_prostate,"
                       "t_gynecology,"
                       "t_breast,"
                       "t_thyroid,"
                       "t_gestation0,"
                       "t_gestation1,"
                       "t_gestation2,"
                       "t_gestation3,"
                       "id_users,"
                       "concluzion,"
                       "comment "
                  "FROM sqlitestudio_temp_table0;");
        if (! qry.exec()){
            qCritical(logCritical()) << tr("Eroare de actualizare a relizului '" release_1_1_4 "' (inserarea datelor din tabela 'sqlitestudio_temp_table0' in tabela 'orderEcho'): ") + qry.lastError().text();
            return false;
        }

        qry.prepare("DROP TABLE sqlitestudio_temp_table0;");
        if (! qry.exec()){
            qCritical(logCritical()) << tr("Eroare de actualizare a relizului '" release_1_1_4 "' (eliminarea tabelei 'sqlitestudio_temp_table0'): ") + qry.lastError().text();
            return false;
        }

        qry_image.prepare("SELECT id_reportEcho FROM imagesReports;");
        if (qry_image.exec()){
            while (qry_image.next()) {
                qry.prepare(QString("UPDATE reportEcho SET attachedImages = 1 WHERE id = '%1';").arg(qry_image.value(0).toString()));
                if (! qry.exec())
                    qCritical(logCritical()) << tr("Eroare de actualizare a documentului 'Comanda ecografica' (inserarea atasarii imaginelor): ") + qry.lastError().text();
            }

        }

        qry.prepare("UPDATE reportEcho SET attachedImages = 0 WHERE attachedImages ISNULL;");
        if (! qry.exec())
            qCritical(logCritical()) << tr("Eroare de actualizare a documentului 'Comanda ecografica' (inserarea atasarii imaginelor): ") + qry.lastError().text();
    }

    if (current_release == release_1_1_5) {
        QSqlQuery qry;
        qry.prepare("CREATE TABLE conclusionTemplates ("
                    "id           INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "deletionMark INT (1) NOT NULL,"
                    "cod          TEXT (10) NOT NULL, "
                    "name         TEXT (512) NOT NULL,"
                    "system       TEXT (150)"
                    ");");
        if (! qry.exec()){
            qCritical(logCritical()) << tr("Nu a fost creata tabela 'conclusionTemplates' - ") + qry.lastError().text();
            return false;
        }
    }

    if (current_release == release_2_0_4) {

        //--------------------------------------------------
        // crearea tabelei noi

        QSqlQuery qry;
        if (globals::thisSqlite){
            qry.prepare("CREATE TABLE userPreferences ("
                        "id                    INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                        "id_users              INT NOT NULL CONSTRAINT userPreferences_users_id REFERENCES users (id) ON DELETE CASCADE,"
                        "versionApp            TEXT (8),"
                        "showQuestionCloseApp  INT,"
                        "showUserManual        INT,"
                        "showHistoryVersion    INT,"
                        "order_splitFullName   INT,"
                        "updateListDoc         TEXT (3),"
                        "showDesignerMenuPrint INT,"
                        "checkNewVersionApp    INT,"
                        "databasesArchiving    INT,"
                        "showAsistantHelper    INT"
                        ");");
        } else if (globals::thisMySQL){
            qry.prepare("CREATE TABLE userPreferences ("
                        "id                    INT NOT Null PRIMARY KEY AUTO_INCREMENT,"
                        "id_users              INT NOT Null,"
                        "versionApp            VARCHAR (8),"
                        "showQuestionCloseApp  BOOLEAN,"
                        "showUserManual        BOOLEAN,"
                        "showHistoryVersion    BOOLEAN,"
                        "order_splitFullName   BOOLEAN,"
                        "updateListDoc         VARCHAR (3),"
                        "showDesignerMenuPrint BOOLEAN,"
                        "checkNewVersionApp    BOOLEAN,"
                        "databasesArchiving    BOOLEAN,"
                        "showAsistantHelper    BOOLEAN,"
                        "KEY `userPreferences_users_id_idx` (`id_users`),"
                        "CONSTRAINT `userPreferences_users_id` FOREIGN KEY (`id_users`) REFERENCES `users` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT"
                        ");");
        } else {
            qCritical(logCritical()) << tr("Nu a fost determinată tipul bazei de date - actualizarea bazei de date la versiunea '%1' nu sa efectuat !!!")
                                               .arg(release_2_0_4);
            return false;
        }

        if (! qry.exec()){
            qCritical(logCritical()) << tr("Nu a fost creata tabela 'userPreferences' - ") + qry.lastError().text();
            return false;
        }

        //-------------------------------------------------
        // inserarea datelor
        db->updateVariableFromTableSettingsUser();
        qry.prepare("INSERT INTO userPreferences ("
                    "id,"
                    "id_users,"
                    "versionApp,"
                    "showQuestionCloseApp,"
                    "showUserManual,"
                    "showHistoryVersion,"
                    "order_splitFullName,"
                    "updateListDoc,"
                    "showDesignerMenuPrint,"
                    "checkNewVersionApp,"
                    "databasesArchiving,"
                    "showAsistantHelper) VALUES (?,?,?,?,?,?,?,?,?,?,?,?);");
        qry.addBindValue(1);
        qry.addBindValue(globals::idUserApp);
        qry.addBindValue("");
        qry.addBindValue(1);
        qry.addBindValue(0);
        qry.addBindValue(0);
        qry.addBindValue(0);
        qry.addBindValue(10);
        qry.addBindValue(0);
        qry.addBindValue(0);
        qry.addBindValue(0);
        qry.addBindValue(1);

        if (! qry.exec())
            qCritical(logCritical()) << tr("Nu au fost inserate date initiale in tabela 'userPreferences' %1.")
                                            .arg((qry.lastError().text().isEmpty()) ? "" : "- " + qry.lastError().text());

        //------------------------------------------------
        // eliminarea tabelei settingsUsers

        qry.prepare("DROP TABLE settingsUsers;");
        if (! qry.exec()){
            qCritical(logCritical()) << tr("Nu este eliminata tabela 'settingsUsers' %1 ")
                                            .arg((qry.lastError().text().isEmpty()) ? "" : "- " + qry.lastError().text());
            return false;
        }
    }

    if (current_release == release_2_0_5) {
        if (db->createTableGestation2())
            qInfo(logInfo()) << tr("Actualizarea BD '2.0.5' - a fost creata tabela 'tableGestation2'.");
        if (db->createTableGestation2_biometry())
            qInfo(logInfo()) << tr("Actualizarea BD '2.0.5' - a fost creata tabela 'tableGestation2_biometry'.");
        if (db->createTableGestation2_cranium())
            qInfo(logInfo()) << tr("Actualizarea BD '2.0.5' - a fost creata tabela 'tableGestation2_cranium'.");
        if (db->createTableGestation2_snc())
            qInfo(logInfo()) << tr("Actualizarea BD '2.0.5' - a fost creata tabela 'tableGestation2_SNC'.");
        if (db->createTableGestation2_heart())
            qInfo(logInfo()) << tr("Actualizarea BD '2.0.5' - a fost creata tabela 'tableGestation2_heart'.");
        if (db->createTableGestation2_thorax())
            qInfo(logInfo()) << tr("Actualizarea BD '2.0.5' - a fost creata tabela 'tableGestation2_thorax'.");
        if (db->createTableGestation2_abdomen())
            qInfo(logInfo()) << tr("Actualizarea BD '2.0.5' - a fost creata tabela 'tableGestation2_abdomen'.");
        if (db->createTableGestation2_urinarySystem())
            qInfo(logInfo()) << tr("Actualizarea BD '2.0.5' - a fost creata tabela 'tableGestation2_urinarySystem'.");
        if (db->createTableGestation2_other())
            qInfo(logInfo()) << tr("Actualizarea BD '2.0.5' - a fost creata tabela 'tableGestation2_other'.");
        if (db->createTableGestation2_doppler())
            qInfo(logInfo()) << tr("Actualizarea BD '2.0.5' - a fost creata tabela 'tableGestation2_doppler'.");
    }

    if (current_release == release_2_0_6) {
        if (db->createTableNormograms())
            qInfo(logInfo()) << tr("Actualizarea BD '2.0.6' - a fost creata tabela 'normograms'.");
        db->loadNormogramsFromXml();
    }

    if (current_release == release_2_0_7) {
        if (db->deleteDataFromTable("normograms"))
            db->loadNormogramsFromXml();
        else
            qInfo(logInfo()) << tr("Actualizarea BD '2.0.7' nereusita !!! Eliminarea datelor din tabela 'normograms' este nereusita.");
    }

    if (current_release == release_2_0_9) {
        if (! updateRelease_2_0_9())
            return false;
    }

    return true;
}

int UpdateReleasesApp::converVersionStringToNumber(const QString current_release) const
{
    int num_version = 0;
    QString str_version = current_release;
    str_version.replace(1,1,"");
    str_version.replace(2,1,"");
    num_version = str_version.toInt();
    return num_version;
}

bool UpdateReleasesApp::updateRelease_2_0_9()
{
    QSqlQuery qry;

    // crearea tabelei temporare
    qry.prepare("CREATE TABLE sqlitestudio_temp_table0 AS SELECT * FROM userPreferences;");
    if (! qry.exec()){
        qCritical(logCritical()) << tr("Eroare de actualizare a relizului '" release_2_0_9 "' (crearea tabelei 'sqlitestudio_temp_table0'): ") + qry.lastError().text();
        return false;
    }

    // eliminarea tabelei vechi
    qry.prepare("DROP TABLE userPreferences;");
    if (! qry.exec()){
        qCritical(logCritical()) << tr("Eroare de actualizare a relizului '" release_2_0_9 "' (eliminarea tabelei 'userPreferences'): ") + qry.lastError().text();
        return false;
    }

    // crearea tabelei noi cu modificari
    db->createTableUserPreferences();

    // inserarea datelor
    qry.prepare("INSERT INTO userPreferences ("
                " id,"
                " id_users,"
                " versionApp,"
                " showQuestionCloseApp,"
                " showUserManual,"
                " showHistoryVersion,"
                " order_splitFullName,"
                " updateListDoc,"
                " showDesignerMenuPrint,"
                " checkNewVersionApp,"
                " databasesArchiving,"
                " showAsistantHelper ) "
                "SELECT "
                " id,"
                " id_users,"
                " versionApp,"
                " showQuestionCloseApp,"
                " showUserManual,"
                " showHistoryVersion,"
                " order_splitFullName,"
                " updateListDoc,"
                " showDesignerMenuPrint,"
                " checkNewVersionApp,"
                " databasesArchiving,"
                " showAsistantHelper FROM sqlitestudio_temp_table0;");
    if (! qry.exec()){
        qCritical(logCritical()) << tr("Eroare de actualizare a relizului '" release_2_0_9 "' (inserarea datelor din tabela 'sqlitestudio_temp_table0' in tabela 'userPreferences'): ") + qry.lastError().text();
        return false;
    }

    // eliminarea tabelei temporare
    qry.prepare("DROP TABLE sqlitestudio_temp_table0;");
    if (! qry.exec()){
        qCritical(logCritical()) << tr("Eroare de actualizare a relizului '" release_2_0_9 "' (eliminarea tabelei 'sqlitestudio_temp_table0'): ") + qry.lastError().text();
        return false;
    }

    return true;
}
