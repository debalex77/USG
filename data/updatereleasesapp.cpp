/*****************************************************************************
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2025 Codreanu Alexandru <alovada.med@gmail.com>
 *
 * This file is part of the USG project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include "updatereleasesapp.h"
#include <common/version.h>

UpdateReleasesApp::UpdateReleasesApp(QObject *parent) : QObject(parent)
{
    db = new DataBase(this);
    initializeUpdateFunctions();
}

UpdateReleasesApp::~UpdateReleasesApp()
{
    delete db;
}

bool UpdateReleasesApp::execUpdateCurrentRelease(const QString current_release)
{
    if (globals().firstLaunch)
        return true;

    if (current_release.isEmpty())
        return true;

    int version_major   = 0;
    int version_minor   = 0;
    int version_release = 0;

    QStringList versionParts = current_release.split('.');
    if (versionParts.size() == 3) {
        version_major   = versionParts[0].toInt(); // Prima componentă
        version_minor   = versionParts[1].toInt(); // A doua componentă
        version_release = versionParts[2].toInt(); // A treia componentă
    } else {
        qCritical(logCritical()) << "Format de versiune invalid - " << versionParts << " !!!";
        return false;
    }

    if (version_major != VERSION_MAJOR) {
        qWarning(logWarning()) << "Versiunea majoră curentă este diferită de cea suportată!";
        updateRelease_3_0_1(); // cum corect de organizat
        return true;
    }

    if (version_major == VERSION_MAJOR &&
        version_minor == VERSION_MINOR) {

        if (version_release < VERSION_RELEASE) {
            // Apelăm funcțiile de actualizare necesare
            for (int i = version_release + 1; i <= VERSION_RELEASE; ++i) {
                if (updateFunctions[version_major][version_minor].contains(i)) {
                    qInfo(logInfo()) << "Se executa actualizare pana la versiunea:"
                                     << VERSION_FULL;
                    updateFunctions[version_major][version_minor][i](); // Apelăm funcția de actualizare
                } else {
                    qInfo(logInfo()) << QString("Functia de actualizare pentru versiunea ""%1.%2.%3"" nu este definita !!!")
                                            .arg(version_major).arg(version_minor).arg(version_release);
                }
            }
        }
    } else {

    }

    return true;
}

void UpdateReleasesApp::initializeUpdateFunctions()
{
    // Inițializarea pentru 2.0.X
    updateFunctions[2][0].insert(4, [this]() { this->updateRelease_2_0_4(); });
    updateFunctions[2][0].insert(5, [this]() { this->updateRelease_2_0_5(); });
    updateFunctions[2][0].insert(6, [this]() { this->updateRelease_2_0_6(); });
    updateFunctions[2][0].insert(7, [this]() { this->updateRelease_2_0_7(); });
    updateFunctions[2][0].insert(9, [this]() { this->updateRelease_2_0_9(); });

    // Inițializarea pentru 3.0.X
    updateFunctions[3][0].insert(1, [this]() { this->updateRelease_3_0_1(); });
    updateFunctions[3][0].insert(3, [this]() { this->updateRelease_3_0_3(); });
    updateFunctions[3][0].insert(6, [this]() { this->updateRelease_3_0_6(); });
    updateFunctions[3][0].insert(7, [this]() { this->updateRelease_3_0_7(); });
}

void UpdateReleasesApp::updateRelease_2_0_4()
{
    //--------------------------------------------------
    // crearea tabelei noi

    QSqlQuery qry;
    if (globals().thisSqlite){
        qry.prepare(R"(
            CREATE TABLE userPreferences (
                id                    INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
                id_users              INT NOT NULL,
                versionApp            TEXT,
                showQuestionCloseApp  INT,
                showUserManual        INT,
                showHistoryVersion    INT,
                order_splitFullName   INT,
                updateListDoc         TEXT,
                showDesignerMenuPrint INT,
                checkNewVersionApp    INT,
                databasesArchiving    INT,
                showAsistantHelper    INT,
                CONSTRAINT userPreferences_users_id FOREIGN KEY (id_users)
                    REFERENCES users (id) ON DELETE CASCADE ON UPDATE RESTRICT
            );
        )");
    } else if (globals().thisMySQL){
        qry.prepare(R"(
            CREATE TABLE userPreferences (
                id                    INT NOT NULL PRIMARY KEY AUTO_INCREMENT,
                id_users              INT NOT NULL,
                versionApp            VARCHAR (8),
                showQuestionCloseApp  BOOLEAN,
                showUserManual        BOOLEAN,
                showHistoryVersion    BOOLEAN,
                order_splitFullName   BOOLEAN,
                updateListDoc         VARCHAR (3),
                showDesignerMenuPrint BOOLEAN,
                checkNewVersionApp    BOOLEAN,
                databasesArchiving    BOOLEAN,
                showAsistantHelper    BOOLEAN,
                KEY `userPreferences_users_id_idx` (`id_users`),
                CONSTRAINT `userPreferences_users_id` FOREIGN KEY (`id_users`)
                    REFERENCES `users` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT
            );
        )");
    } else {
        qCritical(logCritical()) << tr("Nu a fost determinată tipul bazei de date - actualizarea bazei de date la versiunea '2.0.4' nu sa efectuat !!!");
        return;
    }

    if (! qry.exec()){
        qCritical(logCritical()) << tr("Nu a fost creata tabela 'userPreferences' - ") + qry.lastError().text();
        return;
    }

    //-------------------------------------------------
    // inserarea datelor
    db->updateVariableFromTableSettingsUser();
    qry.prepare(R"(
        INSERT INTO userPreferences (
            id,
            id_users,
            versionApp,
            showQuestionCloseApp,
            showUserManual,
            showHistoryVersion,
            order_splitFullName,
            updateListDoc,
            showDesignerMenuPrint,
            checkNewVersionApp,
            databasesArchiving,
            showAsistantHelper)
        VALUES (?,?,?,?,?,?,?,?,?,?,?,?)
    )");
    qry.addBindValue(1);
    qry.addBindValue(globals().idUserApp);
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
        return;
    }
}

void UpdateReleasesApp::updateRelease_2_0_5()
{
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

void UpdateReleasesApp::updateRelease_2_0_6()
{
    if (db->createTableNormograms())
        qInfo(logInfo()) << tr("Actualizarea BD '2.0.6' - a fost creata tabela 'normograms'.");
    db->loadNormogramsFromXml();
}

void UpdateReleasesApp::updateRelease_2_0_7()
{
    if (db->deleteDataFromTable("normograms"))
        db->loadNormogramsFromXml();
    else
        qInfo(logInfo()) << tr("Actualizarea BD '2.0.7' nereusita !!! Eliminarea datelor din tabela 'normograms' este nereusita.");
}

void UpdateReleasesApp::updateRelease_2_0_9()
{
    QSqlQuery qry;

    // crearea tabelei temporare
    qry.prepare("CREATE TABLE sqlitestudio_temp_table0 AS SELECT * FROM userPreferences;");
    if (! qry.exec()){
        qCritical(logCritical()) << tr("Eroare de actualizare a relizului '2.0.9' (crearea tabelei 'sqlitestudio_temp_table0'): ") + qry.lastError().text();
        return;
    }

    // eliminarea tabelei vechi
    qry.prepare("DROP TABLE userPreferences;");
    if (! qry.exec()){
        qCritical(logCritical()) << tr("Eroare de actualizare a relizului '2.0.9' (eliminarea tabelei 'userPreferences'): ") + qry.lastError().text();
        return;
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
        qCritical(logCritical()) << tr("Eroare de actualizare a relizului '2.0.9' (inserarea datelor din tabela 'sqlitestudio_temp_table0' in tabela 'userPreferences'): ") + qry.lastError().text();
        return;
    }

    // eliminarea tabelei temporare
    qry.prepare("DROP TABLE sqlitestudio_temp_table0;");
    if (! qry.exec()){
        qCritical(logCritical()) << tr("Eroare de actualizare a relizului '" USG_VERSION_FULL "' (eliminarea tabelei 'sqlitestudio_temp_table0'): ") + qry.lastError().text();
        return;
    }

}

void UpdateReleasesApp::updateRelease_3_0_1()
{
    db->updateInvestigationFromXML_2024();
    updateTablePacients_release_3_0_1();
    updateTableKidney_release_3_0_1();
    updateTableIntestinalLoops_release_3_0_1();
    updateTableGynecology_release_3_0_1();
    updateTableformationsSystemTemplates_release_3_0_1();
    createIndex_release_3_0_1();
    createIndexForBaseImage_3_0_1();
}

void UpdateReleasesApp::updateTablePacients_release_3_0_1()
{
    QSqlQuery qry;

    if (globals().thisSqlite){

        if (qry.exec("PRAGMA foreign_keys = 0;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: foreign_keys = 0";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu este setat - foreign_keys = 0";

        if (qry.exec("CREATE TABLE sqlitestudio_temp_table AS SELECT * FROM pacients;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creata tabela 'sqlitestudio_temp_table'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu este creata tabela 'sqlitestudio_temp_table'.";

        if (qry.exec("DROP TABLE pacients;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: eliminata tabela 'pacients'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu este eliminata tabela 'pacients'.";

        if (db->createTablePacients())
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: crearea tabelei 'pacients' cu modificari noi.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu este creata tabela 'pacients' cu modificari noi";

        if (qry.exec("INSERT INTO pacients ( id, deletionMark, IDNP, name, fName, mName,  medicalPolicy, birthday, address,"
                     "telephone, email, comment ) "
                     "SELECT id, deletionMark, IDNP, name, fName, mName, medicalPolicy, birthday, address,"
                     "telephone, email, comment FROM sqlitestudio_temp_table;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: copierea datelor din tabela 'sqlitestudio_temp_table' in tabela 'pacients'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu au fost copiate datele din tabela 'sqlitestudio_temp_table' in tabela 'pacients'.";

        if (qry.exec("DROP TABLE sqlitestudio_temp_table;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: eliminata tabela 'sqlitestudio_temp_table'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu este eliminata tabela 'sqlitestudio_temp_table'.";

        if (qry.exec("CREATE TRIGGER `create_full_name_pacient` "
                     "AFTER INSERT ON `pacients` FOR EACH ROW "
                     "BEGIN"
                     "  INSERT INTO fullNamePacients (id_pacients, name, nameIDNP, nameBirthday, nameTelephone, nameBirthdayIDNP) "
                     "  VALUES (NEW.id, "
                     "    NEW.name || ' ' || NEW.fName,"
                     "    NEW.name || ' ' || NEW.fName || ', idnp: ' || NEW.IDNP,"
                     "    NEW.name || ' ' || NEW.fName || ', ' || SUBSTR(NEW.birthday, 9, 2) || '.' || SUBSTR(NEW.birthday, 6, 2) || '.' || SUBSTR(NEW.birthday, 1, 4),"
                     "    NEW.name || ' ' || NEW.fName || ', tel.: ' || NEW.telephone,"
                     "    NEW.name || ' ' || NEW.fName || ', ' || SUBSTR(NEW.birthday, 9, 2) || '.' || SUBSTR(NEW.birthday, 6, 2) || '.' || SUBSTR(NEW.birthday, 1, 4) || ', idnp: ' || NEW.IDNP"
                     "  );"
                     "END;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: crearea trigger-lui 'create_full_name_pacient'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu este creat trigger-ul 'create_full_name_pacient'.";

        if (qry.exec("CREATE TRIGGER `update_full_name_pacient` "
                     "AFTER UPDATE ON `pacients` FOR EACH ROW "
                     "BEGIN"
                     "  UPDATE fullNamePacients SET "
                     "    name = NEW.name || ' ' || NEW.fName,"
                     "    nameIDNP = NEW.name || ' ' || NEW.fName || ', idnp: ' || NEW.IDNP,"
                     "    nameBirthday = NEW.name || ' ' || NEW.fName || ', ' || SUBSTR(NEW.birthday, 9, 2) || '.' || SUBSTR(NEW.birthday, 6, 2) || '.' || SUBSTR(NEW.birthday, 1, 4),"
                     "    nameTelephone = NEW.name || ' ' || NEW.fName || ', tel.: ' || NEW.telephone,"
                     "    nameBirthdayIDNP = NEW.name || ' ' || NEW.fName || ', ' || SUBSTR(NEW.birthday, 9, 2) || '.' || SUBSTR(NEW.birthday, 6, 2) || '.' || SUBSTR(NEW.birthday, 1, 4) || ', idnp: ' || NEW.IDNP"
                     "  WHERE id_pacients = NEW.id;"
                     "END"
                     ";"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: crearea trigger-lui 'update_full_name_pacient'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu este creat trigger-ul 'update_full_name_pacient'";

        if (qry.exec("PRAGMA foreign_keys = 1;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: instalat 'foreign_keys = 1'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu este setat - foreign_keys = 1";

    } else {

        if (qry.exec("ALTER TABLE `pacients` MODIFY `medicalPolicy` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT NULL;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: modificata sectia 'medicalPolicy' in tabela 'pacients'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu s-a efectuat modifcarea tabelei 'pacients' la bd MySQL.";

    }
}

void UpdateReleasesApp::updateTableKidney_release_3_0_1()
{
    QSqlQuery qry;

    if (globals().thisSqlite){

        if (qry.exec("ALTER TABLE tableKidney ADD COLUMN contour_right TEXT CHECK(contour_right IN ('clar', 'sters', 'regulat', 'neregulat')) DEFAULT 'clar';"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: adaugate sectia noua 'contour_right' in tabela 'tableKidney'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu s-a efectuat modifcarea tabelei 'tableKidney' sectia 'contour_right' (bd sqlite).";

        if (qry.exec("ALTER TABLE tableKidney ADD COLUMN contour_left TEXT CHECK(contour_left IN ('clar', 'sters', 'regulat', 'neregulat')) DEFAULT 'clar';"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: adaugate sectia noua 'contour_left' in tabela 'tableKidney'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu s-a efectuat modifcarea tabelei 'tableKidney' sectia 'contour_left' (bd sqlite).";

        if (qry.exec("ALTER TABLE tableKidney ADD COLUMN suprarenal_formations TEXT DEFAULT NULL;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: adaugate sectia noua 'suprarenal_formations' in tabela 'tableKidney'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu s-a efectuat modifcarea tabelei 'tableKidney' sectia 'suprarenal_formations' (bd sqlite).";

    } else {

        if (qry.exec("ALTER TABLE `tableKidney` "
                     "ADD COLUMN `contour_right` ENUM('clar', 'sters', 'regulat', 'neregulat') DEFAULT 'clar', "
                     "ADD COLUMN `contour_left` ENUM('clar', 'sters', 'regulat', 'neregulat') DEFAULT 'clar', "
                     "ADD COLUMN `suprarenal_formations` VARCHAR(500) COLLATE utf8mb4_general_ci DEFAULT NULL;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: adaugate sectii noi 'contour_right', 'contour_left', 'suprarenal_formations' in tabela 'tableKidney'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu s-a efectuat modifcarea tabelei 'tableKidney' la bd MySQL.";

    }
}

void UpdateReleasesApp::updateTableIntestinalLoops_release_3_0_1()
{
    if (db->createTableIntestinalLoop())
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: a fost creata tabela 'tableIntestinalLoop'";
}

void UpdateReleasesApp::updateTableGynecology_release_3_0_1()
{
    QSqlQuery qry;

    if (globals().thisSqlite){

        if (qry.exec("ALTER TABLE tableGynecology ADD COLUMN junctional_zone TEXT CHECK(junctional_zone IN ('contur clar', 'contur sters')) DEFAULT 'contur clar';"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: adaugate sectia noua 'junctional_zone' in tabela 'tableGynecology'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu s-a efectuat modifcarea tabelei 'tableGynecology' sectia 'junctional_zone' (bd sqlite).";

        if (qry.exec("ALTER TABLE tableGynecology ADD COLUMN junctional_zone_description TEXT DEFAULT NULL;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: adaugate sectia noua 'junctional_zone_description' in tabela 'tableGynecology'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu s-a efectuat modifcarea tabelei 'tableGynecology' sectia 'junctional_zone_description' (bd sqlite).";

        if (qry.exec("ALTER TABLE tableGynecology ADD COLUMN cervical_canal TEXT CHECK(cervical_canal IN ('nedilatat', 'dilatat')) DEFAULT 'nedilatat';"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: adaugate sectia noua 'cervical_canal' in tabela 'tableGynecology'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu s-a efectuat modifcarea tabelei 'tableGynecology' sectia 'cervical_canal' (bd sqlite).";

        if (qry.exec("ALTER TABLE tableGynecology ADD COLUMN cervical_canal_formations TEXT DEFAULT NULL;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: adaugate sectia noua 'cervical_canal_formations' in tabela 'tableGynecology'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu s-a efectuat modifcarea tabelei 'tableGynecology' sectia 'cervical_canal_formations' (bd sqlite).";

        if (qry.exec("ALTER TABLE tableGynecology ADD COLUMN fallopian_tubes TEXT CHECK(fallopian_tubes IN ('nonvizibile', 'vizibile')) DEFAULT 'nonvizibile';"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: adaugate sectia noua 'fallopian_tubes' in tabela 'tableGynecology'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu s-a efectuat modifcarea tabelei 'tableGynecology' sectia 'fallopian_tubes' (bd sqlite).";

        if (qry.exec("ALTER TABLE tableGynecology ADD COLUMN fallopian_tubes_formations TEXT DEFAULT NULL;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: adaugate sectia noua 'fallopian_tubes_formations' in tabela 'tableGynecology'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu s-a efectuat modifcarea tabelei 'tableGynecology' sectia 'fallopian_tubes_formations' (bd sqlite).";

    } else {

        if (qry.exec("ALTER TABLE `tableGynecology` "
                     "ADD COLUMN `junctional_zone` ENUM('contur clar', 'contur sters') DEFAULT 'contur clar', "
                     "ADD COLUMN `junctional_zone_description` VARCHAR(256) DEFAULT NULL,"
                     "ADD COLUMN `cervical_canal` ENUM('nedilatat', 'dilatat') DEFAULT 'nedilatat',"
                     "ADD COLUMN `cervical_canal_formations` VARCHAR(256) DEFAULT NULL,"
                     "ADD COLUMN `fallopian_tubes` ENUM('nonvizibile', 'vizibile') DEFAULT 'nonvizibile',"
                     "ADD COLUMN `fallopian_tubes_formations` VARCHAR(256) DEFAULT NULL ;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: adaugate sectii noi 'junctional_zone', 'junctional_zone_description', 'cervical_canal', 'cervical_canal_formations', "
                                "'fallopian_tubes', 'fallopian_tubes_formations' in tabela 'tableGynecology'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu s-a efectuat modifcarea tabelei 'tableGynecology' la bd MySQL.";
    }
}

void UpdateReleasesApp::updateTableformationsSystemTemplates_release_3_0_1()
{
    if (db->createTableFormationsSystemTemplates())
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: a fost creata tabela 'formationsSystemTemplates'";
}

void UpdateReleasesApp::createIndex_release_3_0_1()
{
    QSqlQuery qry;

    if (qry.exec("CREATE INDEX idx_name_fName_pacients ON pacients(name, fName);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_name_fName_pacients'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_name_fName_pacients'.";

    if (qry.exec("CREATE INDEX idx_name_fName_IDNP_pacients ON pacients(name, fName, IDNP);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_name_fName_IDNP_pacients'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_name_fName_IDNP_pacients'.";

    //-----------------------------------------------------------------
    //---------------- pricing

    if (qry.exec("CREATE INDEX idx_dateDoc_pricing ON pricings(dateDoc);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_dateDoc_pricing'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_dateDoc_pricing'.";

    if (qry.exec("CREATE INDEX idx_id_organizations_pricing ON pricings(id_organizations);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_organizations_pricing'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_organizations_pricing'.";

    if (qry.exec("CREATE INDEX idx_id_contracts_pricing ON pricings(id_contracts);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_contracts_pricing'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_contracts_pricing'.";

    if (qry.exec("CREATE INDEX idx_id_users_pricing ON pricings(id_users);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_users_pricing'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_users_pricing'.";

    if (qry.exec("CREATE INDEX idx_id_typesPrices_pricing ON pricings(id_typesPrices);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_typesPrices_pricing'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_typesPrices_pricing'.";

    //-----------------------------------------------------------------
    //---------------- pricingsTable

    if (qry.exec("CREATE INDEX idx_id_pricings_pricingTable ON pricingsTable(id_pricings);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_pricings_pricingTable'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_pricings_pricingTable'.";

    //-----------------------------------------------------------------
    //---------------- orderEcho

    if (qry.exec("CREATE INDEX idx_id_organizations_order ON orderEcho(id_organizations);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_organizations_order'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_organizations_order'.";

    if (qry.exec("CREATE INDEX idx_id_contracts_order ON orderEcho(id_contracts);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_contracts_order'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_contracts_order'.";

    if (qry.exec("CREATE INDEX idx_id_pacients_order ON orderEcho(id_pacients);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_pacients_order'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_pacients_order'.";

    if (qry.exec("CREATE INDEX idx_id_doctors_order ON orderEcho(id_doctors);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_doctors_order'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_doctors_order'.";

    if (qry.exec("CREATE INDEX idx_id_users_order ON orderEcho(id_users);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_users_order'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_users_order'.";

    if (qry.exec("CREATE INDEX idx_dateDoc_order ON orderEcho(dateDoc);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_dateDoc_order'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_dateDoc_order'.";

    //----------------------------------------------------------------
    //----------------- orderEchoTable

    if (qry.exec("CREATE INDEX idx_id_orderEcho_orderTable ON orderEchoTable(id_orderEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_orderEcho_orderTable'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_orderEcho_orderTable'.";

    //----------------------------------------------------------------
    //----------------- reportEcho

    if (qry.exec("CREATE INDEX idx_dateDoc_report ON reportEcho(dateDoc);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_dateDoc_report'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_dateDoc_report'.";

    if (qry.exec("CREATE INDEX idx_id_pacients_report ON reportEcho(id_pacients);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_pacients_report'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_pacients_report'.";

    if (qry.exec("CREATE INDEX idx_id_orderEcho_report ON reportEcho(id_orderEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_orderEcho_report'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_orderEcho_report'.";

    if (qry.exec("CREATE INDEX idx_id_users_report ON reportEcho(id_users);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_users_report'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_users_report'.";

    //---------------------------------------------------------------
    //----------------- tableCholecist

    if (qry.exec("CREATE INDEX idx_id_reportEcho_cholecist ON tableCholecist(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_cholecist'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_cholecist'.";

    //---------------------------------------------------------------
    //----------------- tablePancreas

    if (qry.exec("CREATE INDEX idx_id_reportEcho_pancreas ON tablePancreas(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_pancreas'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_pancreas'.";

    //---------------------------------------------------------------
    //----------------- tableSpleen

    if (qry.exec("CREATE INDEX idx_id_reportEcho_spleen ON tableSpleen(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_spleen'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_spleen'.";


    //---------------------------------------------------------------
    //----------------- tableIntestinalLoop

    if (qry.exec("CREATE INDEX idx_id_reportEcho_intestin ON tableIntestinalLoop(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_intestin'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_intestin'.";

    //---------------------------------------------------------------
    //----------------- tableKidney

    if (qry.exec("CREATE INDEX idx_id_reportEcho_kidney ON tableKidney(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_kidney'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_kidney'.";

    //---------------------------------------------------------------
    //----------------- tableBladder

    if (qry.exec("CREATE INDEX idx_id_reportEcho_bladder ON tableBladder(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_bladder'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_bladder'.";

    //---------------------------------------------------------------
    //----------------- tableProstate

    if (qry.exec("CREATE INDEX idx_id_reportEcho_prostate ON tableProstate(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_prostate'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_prostate'.";

    //---------------------------------------------------------------
    //----------------- tableGynecology

    if (qry.exec("CREATE INDEX idx_id_reportEcho_gynecology ON tableGynecology(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_gynecology'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_gynecology'.";

    //---------------------------------------------------------------
    //----------------- tableBreast

    if (qry.exec("CREATE INDEX idx_id_reportEcho_breast ON tableBreast(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_breast'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_breast'.";

    //---------------------------------------------------------------
    //----------------- tableThyroid

    if (qry.exec("CREATE INDEX idx_id_reportEcho_thyroid ON tableThyroid(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_thyroid'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_thyroid'.";

    //---------------------------------------------------------------
    //----------------- tableGestation0

    if (qry.exec("CREATE INDEX idx_id_reportEcho_ges0 ON tableGestation0(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_ges0'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_ges0'.";

    //---------------------------------------------------------------
    //----------------- tableGestation1

    if (qry.exec("CREATE INDEX idx_id_reportEcho_ges1 ON tableGestation1(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_ges1'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_ges1'.";

    //---------------------------------------------------------------
    //----------------- tables Gestation2
    if (qry.exec("CREATE INDEX idx_id_reportEcho_ges2 ON tableGestation2(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_ges2'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_ges2'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_bio ON tableGestation2_biometry(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_bio'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_bio'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_cr ON tableGestation2_cranium(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_cr'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_cr'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_snc ON tableGestation2_SNC(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_snc'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_snc'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_heart ON tableGestation2_heart(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_heart'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_heart'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_thorax ON tableGestation2_thorax(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_thorax'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_thorax'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_abd ON tableGestation2_abdomen(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_abd'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_abd'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_us ON tableGestation2_urinarySystem(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_us'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_us'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_other ON tableGestation2_other(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_other'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_other'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_doppler ON tableGestation2_doppler(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_doppler'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_doppler'.";

    //---------------------------------------------------------------
    //----------------- formationsSystemTemplates

    if (qry.exec("CREATE INDEX idx_name_typeSystem_formationsSystemTemplates ON formationsSystemTemplates(name, typeSystem);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_name_typeSystem_formationsSystemTemplates'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_name_typeSystem_formationsSystemTemplates'.";
}

void UpdateReleasesApp::createIndexForBaseImage_3_0_1()
{
    QSqlQuery qry(db->getDatabaseImage());

    if (qry.exec("CREATE INDEX idx_id_documents_imagesReports ON imagesReports(id_reportEcho, id_orderEcho, id_patients, id_user);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_documents_imagesReports'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_documents_imagesReports'.";
}

void UpdateReleasesApp::updateRelease_3_0_3()
{
    db->createTableContOnline();
    db->createTableCloudServer();

    QSqlQuery qry;
    if (globals().thisSqlite){

        if (qry.exec("ALTER TABLE tableGestation0 ADD COLUMN lmp TEXT;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.3: adaugate sectia noua 'lmp' in tabela 'tableGestation0'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.3': nu s-a efectuat modifcarea tabelei 'tableGestation0' sectia 'lmp' (bd sqlite).";

        if (qry.exec("ALTER TABLE tableGestation1 ADD COLUMN lmp TEXT;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.3: adaugate sectia noua 'lmp' in tabela 'tableGestation1'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.3': nu s-a efectuat modifcarea tabelei 'tableGestation1' sectia 'lmp' (bd sqlite).";

    } else {

        if (qry.exec("ALTER TABLE `tableGestation0` ADD COLUMN `lmp` VARCHAR(10);"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.3: adaugate sectia noua 'lmp' in tabela 'tableGestation0'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.3': nu s-a efectuat modifcarea tabelei 'tableGestation0' sectia 'lmp' (bd MySQL).";

        if (qry.exec("ALTER TABLE `tableGestation1` ADD COLUMN `lmp` VARCHAR(10);"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.3: adaugate sectia noua 'lmp' in tabela 'tableGestation1'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.3': nu s-a efectuat modifcarea tabelei 'tableGestation1' sectia 'lmp' (bd MySQL).";

    }
}

bool UpdateReleasesApp::existErrFormationsSystemTemplates()
{
    QSqlQuery qry;
    QString ddl;

    if (globals().thisMySQL) {

        qry.prepare(R"(
            SHOW CREATE TABLE formationsSystemTemplates
        )");
        if (qry.exec() && qry.next())
            ddl = qry.value(0).toString();

        if (ddl.isEmpty()) {
            qry.prepare(R"(
                SELECT COLUMN_TYPE
                FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_SCHEMA = DATABASE()
                  AND TABLE_NAME = 'formationsSystemTemplates'
                  AND COLUMN_NAME = 'typeSystem'
            )");
            if (qry.exec() && qry.next())
                ddl = qry.value(0).toString();
        }

    } else {

        qry.prepare(R"(
            SELECT sql
              FROM sqlite_master
             WHERE type = 'table' AND
                   name = 'formationsSystemTemplates';
        )");
        if (qry.exec() && qry.next())
            ddl = qry.value(0).toString();

    }

    if (ddl.isEmpty())
        return false;

    if (ddl.contains("Rinici", Qt::CaseSensitive))
        return true;
    else
        return false;

}

void UpdateReleasesApp::updateRelease_3_0_6()
{
    QSqlQuery qry;

    if (globals().thisSqlite) {

        /** 1. gasim si corectam eroarea in tabela 'formationsSystemTemplates' */
        if (existErrFormationsSystemTemplates()) {

            // Redenumim tabela veche
            qry.exec("ALTER TABLE formationsSystemTemplates RENAME TO formationsSystemTemplates_old;");

            // Cream din nou tabela cu denumirea corecta
            db->createTableFormationsSystemTemplates();

            // Mutam datele
            qry.exec(R"(
                INSERT INTO formationsSystemTemplates (id, deletionMark, name, typeSystem)
                SELECT id, deletionMark, name,
                       CASE WHEN typeSystem='Rinici' THEN 'Rinichi' ELSE typeSystem END
                FROM formationsSystemTemplates_old;
            )");

            // Stergem tabela veche
            qry.exec("DROP TABLE formationsSystemTemplates_old;");

            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.6: s-a corectat enumerarea in tabela 'formationsSystemTemplates'.";
        }

        /** 2. cream tabela noua 'tableSofTissuesLymphNodes' */
        if (db->createTableSofTissuesLymphNodes())
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.6: a fost creata tabela 'tableSofTissuesLymphNodes'.";

        /** 3. Modificam tabela 'reportEcho' - adaugam coloana 't_lymphNodes' */
        if (qry.exec("ALTER TABLE reportEcho ADD COLUMN t_lymphNodes INTEGER DEFAULT 0;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.6: adaugata sectia noua 't_lymphNodes' in tabela 'reportEcho'.";


    } else {

        /** 1. gasim si corectam eroarea in tabela 'formationsSystemTemplates' */
        if (existErrFormationsSystemTemplates()) {

            // Modificam doar coloana
            if (! qry.exec(R"(
                ALTER TABLE formationsSystemTemplates
                MODIFY COLUMN typeSystem ENUM(
                    'Unknow', 'Ficat', 'Colecist', 'Pancreas', 'Splina', 'Intestine', 'Recomandari (org.interne)',
                    'Rinichi', 'V.urinara', 'Gl.suprarenale', 'Recomandari (s.urinar)',
                    'Prostata', 'Recomandari (prostata)',
                    'Tiroida', 'Recomandari (tiroida)',
                    'Gl.mamara (stanga)', 'Gl.mamara (dreapta)', 'Recomandari (gl.mamare)',
                    'Ginecologia (uter)', 'Ginecologia (ovar stang)', 'Ginecologia (ovar drept)', 'Recomandari (ginecologia)',
                    'Recomandari (gestatation0)', 'Recomandari (gestatation1)', 'Recomandari (gestatation2)'
                ) NOT NULL DEFAULT 'Unknow';
            )")) {
                qCritical(logCritical()) << "Eroare ALTER TABLE MySQL:" << qry.lastError().text();
            }

            // Corectăm eventualele valori greșite deja introduse
            if (! qry.exec("UPDATE formationsSystemTemplates SET typeSystem='Rinichi' WHERE typeSystem='Rinici';")) {
                qCritical(logCritical()) << "Eroare UPDATE MySQL:" << qry.lastError().text();
            } else {
                qInfo(logInfo()) << "Actualizarea la versiunea 3.0.6: s-a corectat valori greșite din tabela 'formationsSystemTemplates'.";
            }
        }

        /** 2. cream tabela noua 'tableSofTissuesLymphNodes' */
        if (db->createTableSofTissuesLymphNodes())
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.6: a fost creata tabela 'tableSofTissuesLymphNodes'.";

        /** 3. Modificam tabela 'reportEcho' - adaugam coloana 't_lymphNodes' */
        if (qry.exec("ALTER TABLE `reportEcho` ADD COLUMN `t_lymphNodes` BOOLEAN;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.6: adaugata sectia noua 't_lymphNodes' in tabela 'reportEcho'.";
    }

    qInfo(logInfo()) << "Actualizarea pana la versiunea 3.0.6 s-a finisat.";
}

void UpdateReleasesApp::updateRelease_3_0_7()
{
    /** Cerinte pentru release:
     **
     ** 1. Modularea documentului 'Comanda ecografica'
     ** 2. Crearea formei de 'Parametri' pentru documentul 'Comanda ecografica':
     **    - vizualizarea/ascunderea in tabel a investigatiilor unde este indicat
     **      costul si nu este indicat costul
     ** 3. Procesarea:
     **    -> selectarea mai multor documente
     **    -> export PDF
     **    -> transmiterea prin e-mail.
     **
     ******************************************************************************/
}
