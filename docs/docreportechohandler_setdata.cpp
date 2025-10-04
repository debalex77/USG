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

#include <docs/docreportechohandler_p.h>

static QString calculateGestationalAge(const QDate &lmp)
{
    QDate today = QDate::currentDate();
    int daysDifference = lmp.daysTo(today);

    if (daysDifference < 0) {
        return nullptr;
    }

    int weeks = daysDifference / 7;
    int remainingDays = daysDifference % 7;

    return QString("%1s. %2z.").arg(weeks).arg(remainingDays);
}

static QDate calculateDueDate(const QDate &lmp)
{
    return lmp.addDays(280);  // Adăugăm 280 de zile (40 săptămâni)
}

static void enableItemsGestation2(const int id_button, DocReportEcho& o)
{
    // determinam UI
    auto ui = o.uiPtr();

    if (id_button == 0){ // trimestru II
        ui->gestation2_nasalBones->setEnabled(true);
        ui->gestation2_nasalBones_dimens->setEnabled(true);
        ui->gestation2_nasalFold->setEnabled(true);
        ui->label_gestation2_nasalBones->setEnabled(true);
        ui->label_gestation2_nasalBones_dimens->setEnabled(true);
        ui->label_gestation2_nasalBones_mm->setEnabled(true);
        ui->label_gestation2_nasalFold->setEnabled(true);
        ui->label_gestation2_nasalFold_mm->setEnabled(true);

        ui->label_gestation2_fissureSilvius->setEnabled(true);
        ui->gestation2_fissureSilvius->setEnabled(true);
        ui->label_gestation2_ventricularEjectionPathLeft->setEnabled(true);
        ui->gestation2_ventricularEjectionPathLeft->setEnabled(true);
        ui->gestation2_ventricularEjectionPathLeft_description->setEnabled(true);
        ui->label_gestation2_ventricularEjectionPathRight->setEnabled(true);
        ui->gestation2_ventricularEjectionPathRight->setEnabled(true);
        ui->gestation2_ventricularEjectionPathRight_description->setEnabled(true);
        ui->label_gestation2_planBicav->setEnabled(true);
        ui->gestation2_planBicav->setEnabled(true);
    } else { // trimestru III
        ui->gestation2_nasalBones->setEnabled(false);
        ui->gestation2_nasalBones_dimens->setEnabled(false);
        ui->gestation2_nasalFold->setEnabled(false);
        ui->label_gestation2_nasalBones->setEnabled(false);
        ui->label_gestation2_nasalBones_dimens->setEnabled(false);
        ui->label_gestation2_nasalBones_mm->setEnabled(false);
        ui->label_gestation2_nasalFold->setEnabled(false);
        ui->label_gestation2_nasalFold_mm->setEnabled(false);

        ui->label_gestation2_fissureSilvius->setEnabled(false);
        ui->gestation2_fissureSilvius->setEnabled(false);
        ui->label_gestation2_ventricularEjectionPathLeft->setEnabled(false);
        ui->gestation2_ventricularEjectionPathLeft->setEnabled(false);
        ui->gestation2_ventricularEjectionPathLeft_description->setEnabled(false);
        ui->label_gestation2_ventricularEjectionPathRight->setEnabled(false);
        ui->gestation2_ventricularEjectionPathRight->setEnabled(false);
        ui->gestation2_ventricularEjectionPathRight_description->setEnabled(false);
        ui->label_gestation2_planBicav->setEnabled(false);
        ui->gestation2_planBicav->setEnabled(false);
    }
}

static void extractWeeksAndDays(const QString &str_vg, int &weeks, int &days)
{
    static const QRegularExpression regex(R"((\d+)s\.\s*(\d+)z\.)");
    QRegularExpressionMatch match = regex.match(str_vg);

    if (match.hasMatch()) {
        weeks = match.captured(1).toInt();  // Prima grupare (\d+) -> săptămâni
        days = match.captured(2).toInt();   // A doua grupare (\d+) -> zile
    } else {
        weeks = 0;
        days = 0;
        // qDebug() << "Format invalid!";
    }
}

static QDate calculateDueDateFromFetalAge(int fetalAgeWeeks, int fetalAgeDays)
{
    int fetalAgeTotalDays = (fetalAgeWeeks * 7) + fetalAgeDays;
    int remainingDaysToDueDate = 280 - fetalAgeTotalDays; // Restul până la 40 săptămâni

    return QDate::currentDate().addDays(remainingDaysToDueDate);
}

static void setDueDateGestation2(DocReportEcho& o)
{
    // determinam UI
    auto ui = o.uiPtr();

    int weeks, days;
    extractWeeksAndDays(ui->gestation2_fetus_age->text(), weeks, days);
    if (weeks > 0)
        ui->gestation2_probabilDateBirth->setDate(calculateDueDateFromFetalAge(weeks, days));
}

void DocReportEchoHandler::Impl::setMainTableDoc()
{
    // determinam UI
    auto ui = o.uiPtr();

    QSqlQuery qry;
    qry.prepare(R"(SELECT * FROM reportEcho WHERE id = ?)");
    qry.addBindValue(o.getId());

    if (! qry.exec()) {
        qCritical(logCritical()) << "Eroarea solicitarii determinarii documentului 'reportEcho' dupa ID.";
        return;
    }

    while (qry.next()) {
        QSqlRecord record = qry.record();
        //--- setam flag-le sistemelor
        if (qry.value(record.indexOf("t_organs_internal")).toInt() == 1)
            o.appendSectionSystem(o.OrgansInternal);
        if (qry.value(record.indexOf("t_urinary_system")).toInt() == 1)
            o.appendSectionSystem(o.UrinarySystem);
        if (qry.value(record.indexOf("t_prostate")).toInt() == 1)
            o.appendSectionSystem(o.Prostate);
        if (qry.value(record.indexOf("t_gynecology")).toInt() == 1)
            o.appendSectionSystem(o.Gynecology);
        if (qry.value(record.indexOf("t_breast")).toInt() == 1)
            o.appendSectionSystem(o.Breast);
        if (qry.value(record.indexOf("t_thyroid")).toInt() == 1)
            o.appendSectionSystem(o.Thyroid);
        if (qry.value(record.indexOf("t_gestation0")).toInt() == 1)
            o.appendSectionSystem(o.Gestation0);
        if (qry.value(record.indexOf("t_gestation1")).toInt() == 1)
            o.appendSectionSystem(o.Gestation1);
        if (qry.value(record.indexOf("t_gestation2")).toInt() == 1)
            o.appendSectionSystem(o.Gestation2);
        if (qry.value(record.indexOf("t_lymphNodes")).toInt() == 1)
            o.appendSectionSystem(o.LymphNodes);
        //--- nr. doc.
        ui->editDocNumber->setText(qry.value(record.indexOf("numberDoc")).toString());
        ui->editDocNumber->setDisabled(!ui->editDocNumber->text().isEmpty());
        //--- data doc.
        QString str_date = qry.value(record.indexOf("dateDoc")).toString();
        if (globals().thisMySQL){
            static const QRegularExpression replaceT("T");
            static const QRegularExpression removeMilliseconds("\\.000");
            str_date = str_date.replace(replaceT, " ").replace(removeMilliseconds,"");
            ui->editDocDate->setDateTime(QDateTime::fromString(str_date, "yyyy-MM-dd hh:mm:ss"));
        } else
            ui->editDocDate->setDateTime(QDateTime::fromString(str_date, "yyyy-MM-dd hh:mm:ss"));
        //--- setam titlul documentului
        o.setWindowTitle(QObject::tr("Raport ecografic (validat) %1 %2")
                           .arg("nr." + ui->editDocNumber->text() + " din " +
                                ui->editDocDate->dateTime().toString("dd.MM.yyyy hh:mm:ss"), "[*]"));
        //--- ID pacientului
        int id_pacient = qry.value(record.indexOf("id_pacients")).toInt();
        o.setIdPacient(id_pacient);
        //--- ID utilizatorului
        int id_user = qry.value(record.indexOf("id_users")).toInt();
        o.setIdUser(id_user);
        //--- ID doc.'orderEcho'
        int id_orderEcho = qry.value(record.indexOf("id_orderEcho")).toInt();
        o.setIdDocOrderEcho(id_orderEcho);
        //--- concluzia
        ui->concluzion->setPlainText(qry.value(record.indexOf("concluzion")).toString());
        //--- comentariu
        ui->comment->setPlainText(qry.value(record.indexOf("comment")).toString());
        ui->comment->setHidden(ui->comment->toPlainText().isEmpty());
    }
}

void DocReportEchoHandler::Impl::setData_OrgansInternal()
{
    // actualizam datele flag-lor si verificam daca ales flagul
    if (! (o.getSectionsSystem() & DocReportEcho::OrgansInternal))
        return;

    // determinam UI
    auto ui = o.uiPtr();

    QString str = R"(
        SELECT
            l.id,
            l.id_reportEcho,
            l.[left] AS liver_left_lobe,
            l.[right] AS liver_right_lobe,
            l.contur AS liver_contur,
            l.parenchim AS liver_parenchim,
            l.ecogenity AS liver_ecogenity,
            l.formations AS liver_formations,
            l.ductsIntrahepatic AS liver_ductsIntrahepatic,
            l.porta AS liver_porta,
            l.lienalis AS liver_lienalis,
            l.concluzion AS liver_concluzion,
            l.recommendation AS liver_recommendation,
            c.form AS cholecist_form,
            c.dimens AS cholecist_dimens,
            c.walls AS cholecist_walls,
            c.choledoc AS cholecist_choledoc,
            c.formations AS cholecist_formations,
            p.cefal AS pancreas_cefal,
            p.corp AS pancreas_corp,
            p.tail AS pancreas_tail,
            p.texture AS pancreas_texture,
            p.ecogency AS pancreas_ecogency,
            p.formations AS pancreas_formations,
            s.dimens AS spleen_dimens,
            s.contur AS spleen_contur,
            s.parenchim AS spleen_parenchim,
            s.formations AS spleen_formations,
            i.formations AS intestinal_formation
        FROM
            tableLiver AS l
        LEFT JOIN
            tableCholecist AS c ON l.id_reportEcho = c.id_reportEcho
        LEFT JOIN
            tablePancreas AS p ON l.id_reportEcho = p.id_reportEcho
        LEFT JOIN
            tableSpleen AS s ON l.id_reportEcho = s.id_reportEcho
        LEFT JOIN
            tableIntestinalLoop AS i ON l.id_reportEcho = i.id_reportEcho
        WHERE
            l.id_reportEcho = ?;
    )";

    if (globals().thisMySQL) {
        str = str.replace("[", "`");
        str = str.replace("]", "`");
    }

    QSqlQuery qry;
    qry.prepare(str);
    qry.addBindValue(o.getId());
    if (qry.exec()) {
        while (qry.next()) {
            QSqlRecord record = qry.record();
            // liver
            ui->liver_left->setText(qry.value(record.indexOf("liver_left_lobe")).toString());
            ui->liver_right->setText(qry.value(record.indexOf("liver_right_lobe")).toString());
            ui->liver_contour->setText(qry.value(record.indexOf("liver_contur")).toString());
            ui->liver_parenchyma->setText(qry.value(record.indexOf("liver_parenchim")).toString());
            ui->liver_ecogenity->setText(qry.value(record.indexOf("liver_ecogenity")).toString());
            ui->liver_formations->setText(qry.value(record.indexOf("liver_formations")).toString());
            ui->liver_duct_hepatic->setText(qry.value(record.indexOf("liver_ductsIntrahepatic")).toString());
            ui->liver_porta->setText(qry.value(record.indexOf("liver_porta")).toString());
            ui->liver_lienalis->setText(qry.value(record.indexOf("liver_lienalis")).toString());
            ui->organsInternal_concluzion->setPlainText(qry.value(record.indexOf("liver_concluzion")).toString());
            ui->organsInternal_recommendation->setText(qry.value(record.indexOf("liver_recommendation")).toString());
            // o.appendAllRecomandation(ui->organsInternal_recommendation->text());
            // cholecist
            ui->cholecist_form->setText(qry.value(record.indexOf("cholecist_form")).toString());
            ui->cholecist_dimens->setText(qry.value(record.indexOf("cholecist_dimens")).toString());
            ui->cholecist_walls->setText(qry.value(record.indexOf("cholecist_walls")).toString());
            ui->cholecist_coledoc->setText(qry.value(record.indexOf("cholecist_choledoc")).toString());
            ui->cholecist_formations->setText(qry.value(record.indexOf("cholecist_formations")).toString());
            // pancreas
            ui->pancreas_cefal->setText(qry.value(record.indexOf("pancreas_cefal")).toString());
            ui->pancreas_corp->setText(qry.value(record.indexOf("pancreas_corp")).toString());
            ui->pancreas_tail->setText(qry.value(record.indexOf("pancreas_tail")).toString());
            ui->pancreas_parenchyma->setText(qry.value(record.indexOf("pancreas_texture")).toString());
            ui->pancreas_ecogenity->setText(qry.value(record.indexOf("pancreas_ecogency")).toString());
            ui->pancreas_formations->setText(qry.value(record.indexOf("pancreas_formations")).toString());
            // spleen
            ui->spleen_contour->setText(qry.value(record.indexOf("spleen_dimens")).toString());
            ui->spleen_dimens->setText(qry.value(record.indexOf("spleen_contur")).toString());
            ui->spleen_parenchyma->setText(qry.value(record.indexOf("spleen_parenchim")).toString());
            ui->spleen_formations->setText(qry.value(record.indexOf("spleen_formations")).toString());
            // intestinal loop
            ui->intestinalHandles->setPlainText(qry.value(record.indexOf("intestinal_formation")).toString());
        }
    }
}

void DocReportEchoHandler::Impl::setData_UrinarySystem()
{
    // actualizam datele flag-lor si verificam daca ales flagul
    if (! (o.getSectionsSystem() & DocReportEcho::UrinarySystem))
        return;

    // determinam UI
    auto ui = o.uiPtr();

    QSqlQuery qry;
    qry.prepare(R"(
        SELECT
            k.id,
            k.id_reportEcho,
            k.contour_right AS kidney_contour_right,
            k.contour_left AS kidney_contour_left,
            k.dimens_right AS kidney_dimens_right,
            k.dimens_left AS kidney_dimens_left,
            k.corticomed_right AS kidney_corticomed_right,
            k.corticomed_left AS kidney_corticomed_left,
            k.pielocaliceal_right AS kidney_pielocaliceal_right,
            k.pielocaliceal_left AS kidney_pielocaliceal_left,
            k.formations AS kidney_formations,
            k.suprarenal_formations AS kidney_suprarenal_formations,
            k.concluzion AS kidney_concluzion,
            k.recommendation AS kidney_recommendation,
            b.volum AS bladder_volum,
            b.walls AS bladder_walls,
            b.formations AS bladder_formations
        FROM
            tableKidney k
        LEFT JOIN
            tableBladder AS b ON k.id_reportEcho = b.id_reportEcho
        WHERE
            k.id_reportEcho = ?;
    )");
    qry.addBindValue(o.getId());
    if (qry.exec()) {
        while (qry.next()) {
            QSqlRecord record = qry.record();
            // kidney
            ui->kidney_contur_right->setCurrentText(qry.value(record.indexOf("kidney_contour_right")).toString());
            ui->kidney_contur_left->setCurrentText(qry.value(record.indexOf("kidney_contour_left")).toString());
            ui->kidney_right->setText(qry.value(record.indexOf("kidney_dimens_right")).toString());
            ui->kidney_left->setText(qry.value(record.indexOf("kidney_dimens_left")).toString());
            ui->kidney_corticomed_right->setText(qry.value(record.indexOf("kidney_corticomed_right")).toString());
            ui->kidney_corticomed_left->setText(qry.value(record.indexOf("kidney_corticomed_left")).toString());
            ui->kidney_pielocaliceal_right->setText(qry.value(record.indexOf("kidney_pielocaliceal_right")).toString());
            ui->kidney_pielocaliceal_left->setText(qry.value(record.indexOf("kidney_pielocaliceal_left")).toString());
            ui->kidney_formations->setText(qry.value(record.indexOf("kidney_formations")).toString());
            ui->adrenalGlands->setPlainText(qry.value(record.indexOf("kidney_suprarenal_formations")).toString());
            ui->urinary_system_concluzion->setPlainText(qry.value(record.indexOf("kidney_concluzion")).toString());
            ui->urinary_system_recommendation->setText(qry.value(record.indexOf("kidney_recommendation")).toString());
            // o.appendAllRecomandation(ui->urinary_system_recommendation->text());
            // bladder
            ui->bladder_volum->setText(qry.value(record.indexOf("bladder_volum")).toString());
            ui->bladder_walls->setText(qry.value(record.indexOf("bladder_walls")).toString());
            ui->bladder_formations->setText(qry.value(record.indexOf("bladder_formations")).toString());
        }
    }
}

void DocReportEchoHandler::Impl::setData_Prostate()
{
    // actualizam datele flag-lor si verificam daca ales flagul
    if (! (o.getSectionsSystem() & DocReportEcho::Prostate))
        return;

    // determinam UI
    auto ui = o.uiPtr();

    QSqlQuery qry;
    qry.prepare("SELECT * FROM tableProstate WHERE id_reportEcho = ?;");
    qry.addBindValue(o.getId());
    if (qry.exec()) {
        while (qry.next()) {
            QSqlRecord record = qry.record();
            ui->prostate_radioBtn_transrectal->setChecked(qry.value(record.indexOf("transrectal")).toInt());
            ui->prostate_dimens->setText(qry.value(record.indexOf("dimens")).toString());
            ui->prostate_volum->setText(qry.value(record.indexOf("volume")).toString());
            ui->prostate_contur->setText(qry.value(record.indexOf("contour")).toString());
            ui->prostate_ecostructure->setText(qry.value(record.indexOf("ecostructure")).toString());
            ui->prostate_ecogency->setText(qry.value(record.indexOf("ecogency")).toString());
            ui->prostate_formations->setText(qry.value(record.indexOf("formations")).toString());
            ui->prostate_concluzion->setPlainText(qry.value(record.indexOf("concluzion")).toString());
            ui->prostate_recommendation->setText(qry.value(record.indexOf("recommendation")).toString());
        }
    }
}

void DocReportEchoHandler::Impl::setData_Gynecology()
{
    // actualizam datele flag-lor si verificam daca ales flagul
    if (! (o.getSectionsSystem() & DocReportEcho::Gynecology))
        return;

    // determinam UI
    auto ui = o.uiPtr();

    QSqlQuery qry;
    qry.prepare("SELECT * FROM tableGynecology WHERE id_reportEcho = ?;");
    qry.addBindValue(o.getId());
    if (qry.exec()) {
        while (qry.next()) {
            QSqlRecord record = qry.record();
            bool transvaginal_checked = qry.value(record.indexOf("transvaginal")).toBool();
            ui->gynecology_btn_transvaginal->setChecked(transvaginal_checked);
            ui->gynecology_btn_transabdom->setChecked(!transvaginal_checked);

            ui->gynecology_dateMenstruation->setDate(QDate::fromString(qry.value(record.indexOf("dateMenstruation")).toString(), "yyyy-MM-dd"));
            ui->gynecology_antecedent->setText(qry.value(record.indexOf("antecedent")).toString());
            ui->gynecology_uterus_dimens->setText(qry.value(record.indexOf("uterus_dimens")).toString());
            ui->gynecology_uterus_pozition->setText(qry.value(record.indexOf("uterus_pozition")).toString());
            ui->gynecology_uterus_ecostructure->setText(qry.value(record.indexOf("uterus_ecostructure")).toString());
            ui->gynecology_uterus_formations->setPlainText(qry.value(record.indexOf("uterus_formations")).toString());
            ui->gynecology_combo_jonctional_zone->setCurrentText(qry.value(record.indexOf("junctional_zone")).toString());
            ui->gynecology_jonctional_zone_description->setText(qry.value(record.indexOf("junctional_zone_description")).toString());
            ui->gynecology_ecou_dimens->setText(qry.value(record.indexOf("ecou_dimens")).toString());
            ui->gynecology_ecou_ecostructure->setText(qry.value(record.indexOf("ecou_ecostructure")).toString());
            ui->gynecology_cervix_dimens->setText(qry.value(record.indexOf("cervix_dimens")).toString());
            ui->gynecology_cervix_ecostructure->setText(qry.value(record.indexOf("cervix_ecostructure")).toString());
            ui->gynecology_combo_canal_cervical->setCurrentText(qry.value(record.indexOf("cervical_canal")).toString());
            ui->gynecology_canal_cervical_formations->setText(qry.value(record.indexOf("cervical_canal_formations")).toString());
            ui->gynecology_douglas->setText(qry.value(record.indexOf("douglas")).toString());
            ui->gynecology_plex_venos->setText(qry.value(record.indexOf("plex_venos")).toString());
            ui->gynecology_ovary_right_dimens->setText(qry.value(record.indexOf("ovary_right_dimens")).toString());
            ui->gynecology_ovary_left_dimens->setText(qry.value(record.indexOf("ovary_left_dimens")).toString());
            ui->gynecology_ovary_right_volum->setText(qry.value(record.indexOf("ovary_right_volum")).toString());
            ui->gynecology_ovary_left_volum->setText(qry.value(record.indexOf("ovary_left_volum")).toString());
            ui->gynecology_follicule_right->setText(qry.value(record.indexOf("ovary_right_follicle")).toString());
            ui->gynecology_follicule_left->setText(qry.value(record.indexOf("ovary_left_follicle")).toString());
            ui->gynecology_ovary_formations_right->setPlainText(qry.value(record.indexOf("ovary_right_formations")).toString());
            ui->gynecology_ovary_formations_left->setPlainText(qry.value(record.indexOf("ovary_left_formations")).toString());
            ui->gynecology_combo_fallopian_tubes->setCurrentText(qry.value(record.indexOf("fallopian_tubes")).toString());
            ui->gynecology_fallopian_tubes_formations->setText(qry.value(record.indexOf("fallopian_tubes_formations")).toString());
            ui->gynecology_concluzion->setPlainText(qry.value(record.indexOf("concluzion")).toString());
            ui->gynecology_recommendation->setText(qry.value(record.indexOf("recommendation")).toString());
        }
    }
}

void DocReportEchoHandler::Impl::setData_Breast()
{
    // actualizam datele flag-lor si verificam daca ales flagul
    if (! (o.getSectionsSystem() & DocReportEcho::Breast))
        return;

    // determinam UI
    auto ui = o.uiPtr();

    QSqlQuery qry;
    qry.prepare("SELECT * FROM tableBreast WHERE id_reportEcho = ?;");
    qry.addBindValue(o.getId());
    if (qry.exec()) {
        while (qry.next()) {
            QSqlRecord record = qry.record();
            ui->breast_right_ecostructure->setText(qry.value(record.indexOf("breast_right_ecostrcture")).toString());
            ui->breast_right_duct->setText(qry.value(record.indexOf("breast_right_duct")).toString());
            ui->breast_right_ligament->setText(qry.value(record.indexOf("breast_right_ligament")).toString());
            ui->breast_right_formations->setPlainText(qry.value(record.indexOf("breast_right_formations")).toString());
            ui->breast_right_ganglions->setText(qry.value(record.indexOf("breast_right_ganglions")).toString());
            ui->breast_left_ecostructure->setText(qry.value(record.indexOf("breast_left_ecostrcture")).toString());
            ui->breast_left_duct->setText(qry.value(record.indexOf("breast_left_duct")).toString());
            ui->breast_left_ligament->setText(qry.value(record.indexOf("breast_left_ligament")).toString());
            ui->breast_left_formations->setPlainText(qry.value(record.indexOf("breast_left_formations")).toString());
            ui->breast_left_ganglions->setText(qry.value(record.indexOf("breast_left_ganglions")).toString());
            ui->breast_concluzion->setPlainText(qry.value(record.indexOf("concluzion")).toString());
        }
    }
}

void DocReportEchoHandler::Impl::setData_Thyroid()
{
    // actualizam datele flag-lor si verificam daca ales flagul
    if (! (o.getSectionsSystem() & DocReportEcho::Thyroid))
        return;

    // determinam UI
    auto ui = o.uiPtr();

    QSqlQuery qry;
    qry.prepare("SELECT * FROM tableThyroid WHERE id_reportEcho = ?;");
    qry.addBindValue(o.getId());
    if (qry.exec()) {
        while (qry.next()) {
            QSqlRecord record = qry.record();
            ui->thyroid_right_dimens->setText(qry.value(record.indexOf("thyroid_right_dimens")).toString());
            ui->thyroid_right_volum->setText(qry.value(record.indexOf("thyroid_right_volum")).toString());
            ui->thyroid_left_dimens->setText(qry.value(record.indexOf("thyroid_left_dimens")).toString());
            ui->thyroid_left_volum->setText(qry.value(record.indexOf("thyroid_left_volum")).toString());
            ui->thyroid_istm->setText(qry.value(record.indexOf("thyroid_istm")).toString());
            ui->thyroid_ecostructure->setText(qry.value(record.indexOf("thyroid_ecostructure")).toString());
            ui->thyroid_formations->setPlainText(qry.value(record.indexOf("thyroid_formations")).toString());
            ui->thyroid_ganglions->setText(qry.value(record.indexOf("thyroid_ganglions")).toString());
            ui->thyroid_concluzion->setPlainText(qry.value(record.indexOf("concluzion")).toString());
            ui->thyroid_recommendation->setText(qry.value(record.indexOf("recommendation")).toString());
        }
    }
}

void DocReportEchoHandler::Impl::setData_Gestation0()
{
    // actualizam datele flag-lor si verificam daca ales flagul
    if (! (o.getSectionsSystem() & DocReportEcho::Gestation0))
        return;

    // determinam UI
    auto ui = o.uiPtr();

    QSqlQuery qry;
    qry.prepare("SELECT * FROM tableGestation0 WHERE id_reportEcho = ?;");
    qry.addBindValue(o.getId());
    if (qry.exec()) {
        while (qry.next()) {
            QSqlRecord record = qry.record();
            ui->gestation0_view_good->setChecked(qry.value(record.indexOf("view_examination")).toInt() == 0);
            ui->gestation0_view_medium->setChecked(qry.value(record.indexOf("view_examination")).toInt() == 1);
            ui->gestation0_view_difficult->setChecked(qry.value(record.indexOf("view_examination")).toInt() == 2);
            ui->gestation0_antecedent->setText(qry.value(record.indexOf("antecedent")).toString());
            ui->gestation0_LMP->setDate(QDate::fromString(qry.value(record.indexOf("lmp")).toString(), "yyyy-MM-dd"));
            calculateGestationalAge(ui->gestation0_LMP->date());
            calculateDueDate(ui->gestation0_LMP->date());
            ui->gestation0_gestation->setText(qry.value(record.indexOf("gestation_age")).toString());
            ui->gestation0_GS_dimens->setText(qry.value(record.indexOf("GS")).toString());
            ui->gestation0_GS_age->setText(qry.value(record.indexOf("GS_age")).toString());
            ui->gestation0_CRL_dimens->setText(qry.value(record.indexOf("CRL")).toString());
            ui->gestation0_CRL_age->setText(qry.value(record.indexOf("CRL_age")).toString());
            ui->gestation0_BCF->setText(qry.value(record.indexOf("BCF")).toString());
            ui->gestation0_liquid_amniotic->setText(qry.value(record.indexOf("liquid_amniotic")).toString());
            ui->gestation0_miometer->setText(qry.value(record.indexOf("miometer")).toString());
            ui->gestation0_cervix->setText(qry.value(record.indexOf("cervix")).toString());
            ui->gestation0_ovary->setText(qry.value(record.indexOf("ovary")).toString());
            ui->gestation0_concluzion->setPlainText(qry.value(record.indexOf("concluzion")).toString());
            ui->gestation0_recommendation->setText(qry.value(record.indexOf("recommendation")).toString());
        }
    }
}

void DocReportEchoHandler::Impl::setData_Gestation1()
{
    // actualizam datele flag-lor si verificam daca ales flagul
    if (! (o.getSectionsSystem() & DocReportEcho::Gestation1))
        return;

    // determinam UI
    auto ui = o.uiPtr();

    QSqlQuery qry;
    qry.prepare("SELECT * FROM tableGestation1 WHERE id_reportEcho = ?;");
    qry.addBindValue(o.getId());
    if (qry.exec()) {
        while (qry.next()) {
            QSqlRecord record = qry.record();
            ui->gestation1_view_good->setChecked(qry.value(record.indexOf("view_examination")).toInt() == 0);
            ui->gestation1_view_medium->setChecked(qry.value(record.indexOf("view_examination")).toInt() == 1);
            ui->gestation1_view_difficult->setChecked(qry.value(record.indexOf("view_examination")).toInt() == 2);
            ui->gestation1_antecedent->setText(qry.value(record.indexOf("antecedent")).toString());
            ui->gestation1_LMP->setDate(QDate::fromString(qry.value(record.indexOf("lmp")).toString(), "yyyy-MM-dd"));
            calculateGestationalAge(ui->gestation1_LMP->date());
            calculateDueDate(ui->gestation1_LMP->date());
            ui->gestation1_gestation->setText(qry.value(record.indexOf("gestation_age")).toString());
            ui->gestation1_CRL_dimens->setText(qry.value(record.indexOf("CRL")).toString());
            ui->gestation1_CRL_age->setText(qry.value(record.indexOf("CRL_age")).toString());
            ui->gestation1_BPD_dimens->setText(qry.value(record.indexOf("BPD")).toString());
            ui->gestation1_BPD_age->setText(qry.value(record.indexOf("BPD_age")).toString());
            ui->gestation1_NT_dimens->setText(qry.value(record.indexOf("NT")).toString());
            ui->gestation1_NT_percent->setText(qry.value(record.indexOf("NT_percent")).toString());
            ui->gestation1_BN_dimens->setText(qry.value(record.indexOf("BN")).toString());
            ui->gestation1_BN_percent->setText(qry.value(record.indexOf("BN_percent")).toString());
            ui->gestation1_BCF->setText(qry.value(record.indexOf("BCF")).toString());
            ui->gestation1_FL_dimens->setText(qry.value(record.indexOf("FL")).toString());
            ui->gestation1_FL_age->setText(qry.value(record.indexOf("FL_age")).toString());
            ui->gestation1_callote_cranium->setText(qry.value(record.indexOf("callote_cranium")).toString());
            ui->gestation1_plex_choroid->setText(qry.value(record.indexOf("plex_choroid")).toString());
            ui->gestation1_vertebral_column->setText(qry.value(record.indexOf("vertebral_column")).toString());
            ui->gestation1_stomach->setText(qry.value(record.indexOf("stomach")).toString());
            ui->gestation1_bladder->setText(qry.value(record.indexOf("bladder")).toString());
            ui->gestation1_diaphragm->setText(qry.value(record.indexOf("diaphragm")).toString());
            ui->gestation1_abdominal_wall->setText(qry.value(record.indexOf("abdominal_wall")).toString());
            ui->gestation1_location_placenta->setText(qry.value(record.indexOf("location_placenta")).toString());
            ui->gestation1_sac_vitelin->setText(qry.value(record.indexOf("sac_vitelin")).toString());
            ui->gestation1_amniotic_liquid->setText(qry.value(record.indexOf("amniotic_liquid")).toString());
            ui->gestation1_miometer->setText(qry.value(record.indexOf("miometer")).toString());
            ui->gestation1_cervix->setText(qry.value(record.indexOf("cervix")).toString());
            ui->gestation1_ovary->setText(qry.value(record.indexOf("ovary")).toString());
            ui->gestation1_concluzion->setPlainText(qry.value(record.indexOf("concluzion")).toString());
            ui->gestation1_recommendation->setText(qry.value(record.indexOf("recommendation")).toString());
        }
    }
}

void DocReportEchoHandler::Impl::setData_Gestation2()
{
    // actualizam datele flag-lor si verificam daca ales flagul
    if (! (o.getSectionsSystem() & DocReportEcho::Gestation2))
        return;

    // determinam UI
    auto ui = o.uiPtr();

    QSqlQuery qry;
    qry.prepare(R"(
        SELECT
            ges.id AS ges_id,
            ges.id_reportEcho,
            ges.gestation_age,
            ges.trimestru,
            ges.dateMenstruation,
            ges.view_examination,
            ges.single_multiple_pregnancy,
            ges.single_multiple_pregnancy_description,
            ges.antecedent,
            ges.comment,
            ges.concluzion,
            ges.recommendation,
            bio.BPD,
            bio.BPD_age,
            bio.HC,
            bio.HC_age,
            bio.AC,
            bio.AC_age,
            bio.FL,
            bio.FL_age,
            bio.FetusCorresponds,
            cr.calloteCranium,
            cr.facialeProfile,
            cr.nasalBones,
            cr.nasalBones_dimens,
            cr.eyeball,
            cr.eyeball_desciption,
            cr.nasolabialTriangle,
            cr.nasolabialTriangle_description,
            cr.nasalFold,
            snc.hemispheres,
            snc.fissureSilvius,
            snc.corpCalos,
            snc.ventricularSystem,
            snc.ventricularSystem_description,
            snc.cavityPellucidSeptum,
            snc.choroidalPlex,
            snc.choroidalPlex_description,
            snc.cerebellum,
            snc.cerebellum_description,
            snc.vertebralColumn,
            snc.vertebralColumn_description,
            hrt.`position`,
            hrt.heartBeat,
            hrt.heartBeat_frequency,
            hrt.heartBeat_rhythm,
            hrt.pericordialCollections,
            hrt.planPatruCamere,
            hrt.planPatruCamere_description,
            hrt.ventricularEjectionPathLeft,
            hrt.ventricularEjectionPathLeft_description,
            hrt.ventricularEjectionPathRight,
            hrt.ventricularEjectionPathRight_description,
            hrt.intersectionVesselMagistral,
            hrt.intersectionVesselMagistral_description,
            hrt.planTreiVase,
            hrt.planTreiVase_description,
            hrt.archAorta,
            hrt.planBicav,
            th.pulmonaryAreas,
            th.pulmonaryAreas_description,
            th.pleuralCollections,
            th.diaphragm,
            abd.abdominalWall,
            abd.abdominalCollections,
            abd.stomach,
            abd.stomach_description,
            abd.abdominalOrgans,
            abd.cholecist,
            abd.cholecist_description,
            abd.intestine,
            abd.intestine_description,
            us.kidneys,
            us.kidneys_descriptions,
            us.ureter,
            us.ureter_descriptions,
            us.bladder,
            oth.externalGenitalOrgans,
            oth.externalGenitalOrgans_aspect,
            oth.extremities,
            oth.extremities_descriptions,
            oth.fetusMass,
            oth.placenta,
            oth.placentaLocalization,
            oth.placentaDegreeMaturation,
            oth.placentaDepth,
            oth.placentaStructure,
            oth.placentaStructure_descriptions,
            oth.umbilicalCordon,
            oth.umbilicalCordon_description,
            oth.insertionPlacenta,
            oth.amnioticIndex,
            oth.amnioticIndexAspect,
            oth.amnioticBedDepth,
            oth.cervix,
            oth.cervix_description,
            dop.ombilic_PI,
            dop.ombilic_RI,
            dop.ombilic_SD,
            dop.ombilic_flux,
            dop.cerebral_PI,
            dop.cerebral_RI,
            dop.cerebral_SD,
            dop.cerebral_flux,
            dop.uterRight_PI,
            dop.uterRight_RI,
            dop.uterRight_SD,
            dop.uterRight_flux,
            dop.uterLeft_PI,
            dop.uterLeft_RI,
            dop.uterLeft_SD,
            dop.uterLeft_flux,
            dop.ductVenos
        FROM
            tableGestation2 AS ges
        LEFT JOIN
            tableGestation2_biometry AS bio ON bio.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_cranium AS cr ON cr.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_SNC AS snc ON snc.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_heart AS hrt ON hrt.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_thorax AS th ON th.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_abdomen AS abd ON abd.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_urinarySystem AS us ON us.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_other AS oth ON oth.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_doppler AS dop ON dop.id_reportEcho = ges.id_reportEcho
        WHERE
            ges.id_reportEcho = ?;
    )");
    qry.addBindValue(o.getId());
    if (qry.exec()) {
        while (qry.next()) {
            QSqlRecord record = qry.record();
            // tableGestation2
            ui->gestation2_dateMenstruation->setDate(QDate::fromString(qry.value(record.indexOf("dateMenstruation")).toString(), "yyyy-MM-dd"));
            ui->gestation2_gestation_age->setText(qry.value(record.indexOf("gestation_age")).toString());
            ui->gestation2_view_examination->setCurrentIndex(qry.value(record.indexOf("view_examination")).toInt());
            if (qry.value(record.indexOf("trimestru")).toInt() == 2){
                ui->gestation2_trimestru2->setChecked(true);
                enableItemsGestation2(0, o);
            } else {
                ui->gestation2_trimestru3->setChecked(true);
                enableItemsGestation2(1, o);
            }
            ui->gestation2_pregnancy->setCurrentIndex(qry.value(record.indexOf("single_multiple_pregnancy")).toInt());
            ui->gestation2_pregnancy_description->setText(qry.value(record.indexOf("single_multiple_pregnancy_description")).toString());
            ui->gestation2_comment->setPlainText(qry.value(record.indexOf("comment")).toString());
            ui->gestation2_concluzion->setPlainText(qry.value(record.indexOf("concluzion")).toString());
            ui->gestation2_recommendation->setText(qry.value(record.indexOf("recommendation")).toString());

            // tableGestation2_biometry
            ui->gestation2_bpd->setText(qry.value(record.indexOf("BPD")).toString());
            ui->gestation2_bpd_age->setText(qry.value(record.indexOf("BPD_age")).toString());
            ui->gestation2_hc->setText(qry.value(record.indexOf("HC")).toString());
            ui->gestation2_hc_age->setText(qry.value(record.indexOf("HC_age")).toString());
            ui->gestation2_ac->setText(qry.value(record.indexOf("AC")).toString());
            ui->gestation2_ac_age->setText(qry.value(record.indexOf("AC_age")).toString());
            ui->gestation2_fl->setText(qry.value(record.indexOf("FL")).toString());
            ui->gestation2_fl_age->setText(qry.value(record.indexOf("FL_age")).toString());
            ui->gestation2_fetus_age->setText(qry.value(record.indexOf("FetusCorresponds")).toString());
            setDueDateGestation2(o);

            // tableGestation2_cranium
            ui->gestation2_calloteCranium->setCurrentIndex(qry.value(record.indexOf("calloteCranium")).toInt());
            ui->gestation2_facialeProfile->setCurrentIndex(qry.value(record.indexOf("facialeProfile")).toInt());
            ui->gestation2_nasalBones->setCurrentIndex(qry.value(record.indexOf("nasalBones")).toInt());
            ui->gestation2_nasalBones_dimens->setText(qry.value(record.indexOf("nasalBones_dimens")).toString());
            ui->gestation2_eyeball->setCurrentIndex(qry.value(record.indexOf("eyeball")).toInt());
            ui->gestation2_eyeball_desciption->setText(qry.value(record.indexOf("eyeball_desciption")).toString());
            ui->gestation2_nasolabialTriangle->setCurrentIndex(qry.value(record.indexOf("nasolabialTriangle")).toInt());
            ui->gestation2_nasolabialTriangle_description->setText(qry.value(record.indexOf("nasolabialTriangle_description")).toString());
            ui->gestation2_nasalFold->setText(qry.value(record.indexOf("nasalFold")).toString());

            // tableGestation2_SNC
            ui->gestation2_hemispheres->setCurrentIndex(qry.value(record.indexOf("hemispheres")).toInt());
            ui->gestation2_fissureSilvius->setCurrentIndex(qry.value(record.indexOf("fissureSilvius")).toInt());
            ui->gestation2_corpCalos->setCurrentIndex(qry.value(record.indexOf("corpCalos")).toInt());
            ui->gestation2_ventricularSystem->setCurrentIndex(qry.value(record.indexOf("ventricularSystem")).toInt());
            ui->gestation2_ventricularSystem_description->setText(qry.value(record.indexOf("ventricularSystem_description")).toString());
            ui->gestation2_cavityPellucidSeptum->setCurrentIndex(qry.value(record.indexOf("cavityPellucidSeptum")).toInt());
            ui->gestation2_choroidalPlex->setCurrentIndex(qry.value(record.indexOf("choroidalPlex")).toInt());
            ui->gestation2_choroidalPlex_description->setText(qry.value(record.indexOf("choroidalPlex_description")).toString());
            ui->gestation2_cerebellum->setCurrentIndex(qry.value(record.indexOf("cerebellum")).toInt());
            ui->gestation2_cerebellum_description->setText(qry.value(record.indexOf("cerebellum_description")).toString());
            ui->gestation2_vertebralColumn->setCurrentIndex(qry.value(record.indexOf("vertebralColumn")).toInt());
            ui->gestation2_vertebralColumn_description->setText(qry.value(record.indexOf("vertebralColumn_description")).toString());

            // tableGestation2_heart
            ui->gestation2_heartPosition->setText(qry.value(record.indexOf("position")).toString());
            ui->gestation2_heartBeat->setCurrentIndex(qry.value(record.indexOf("heartBeat")).toInt());
            ui->gestation2_heartBeat_frequency->setText(qry.value(record.indexOf("heartBeat_frequency")).toString());
            ui->gestation2_heartBeat_rhythm->setCurrentIndex(qry.value(record.indexOf("heartBeat_rhythm")).toInt());
            ui->gestation2_pericordialCollections->setCurrentIndex(qry.value(record.indexOf("pericordialCollections")).toInt());
            ui->gestation2_planPatruCamere->setCurrentIndex(qry.value(record.indexOf("planPatruCamere")).toInt());
            ui->gestation2_planPatruCamere_description->setText(qry.value(record.indexOf("planPatruCamere_description")).toString());
            ui->gestation2_ventricularEjectionPathLeft->setCurrentIndex(qry.value(record.indexOf("ventricularEjectionPathLeft")).toInt());
            ui->gestation2_ventricularEjectionPathLeft_description->setText(qry.value(record.indexOf("ventricularEjectionPathLeft_description")).toString());
            ui->gestation2_ventricularEjectionPathRight->setCurrentIndex(qry.value(record.indexOf("ventricularEjectionPathRight")).toInt());
            ui->gestation2_ventricularEjectionPathRight_description->setText(qry.value(record.indexOf("ventricularEjectionPathRight_description")).toString());
            ui->gestation2_intersectionVesselMagistral->setCurrentIndex(qry.value(record.indexOf("intersectionVesselMagistral")).toInt());
            ui->gestation2_intersectionVesselMagistral_description->setText(qry.value(record.indexOf("intersectionVesselMagistral_description")).toString());
            ui->gestation2_planTreiVase->setCurrentIndex(qry.value(record.indexOf("planTreiVase")).toInt());
            ui->gestation2_planTreiVase_description->setText(qry.value(record.indexOf("planTreiVase_description")).toString());
            ui->gestation2_archAorta->setCurrentIndex(qry.value(record.indexOf("archAorta")).toInt());
            ui->gestation2_planBicav->setCurrentIndex(qry.value(record.indexOf("planBicav")).toInt());

            // tableGestation2_thorax
            ui->gestation2_pulmonaryAreas->setCurrentIndex(qry.value(record.indexOf("pulmonaryAreas")).toInt());
            ui->gestation2_pulmonaryAreas_description->setText(qry.value(record.indexOf("pulmonaryAreas_description")).toString());
            ui->gestation2_pleuralCollections->setCurrentIndex(qry.value(record.indexOf("pleuralCollections")).toInt());
            ui->gestation2_diaphragm->setCurrentIndex(qry.value(record.indexOf("diaphragm")).toInt());

            // tableGestation2_abdomen
            ui->gestation2_abdominalWall->setCurrentIndex(qry.value(record.indexOf("abdominalWall")).toInt());
            ui->gestation2_abdominalCollections->setCurrentIndex(qry.value(record.indexOf("abdominalCollections")).toInt());
            ui->gestation2_stomach->setCurrentIndex(qry.value(record.indexOf("stomach")).toInt());
            ui->gestation2_stomach_description->setText(qry.value(record.indexOf("stomach_description")).toString());
            ui->gestation2_abdominalOrgans->setCurrentIndex(qry.value(record.indexOf("abdominalOrgans")).toInt());
            ui->gestation2_cholecist->setCurrentIndex(qry.value(record.indexOf("cholecist")).toInt());
            ui->gestation2_cholecist_description->setText(qry.value(record.indexOf("cholecist_description")).toString());
            ui->gestation2_intestine->setCurrentIndex(qry.value(record.indexOf("intestine")).toInt());
            ui->gestation2_intestine_description->setText(qry.value(record.indexOf("intestine_description")).toString());

            // tableGestation2_urinarySystem
            ui->gestation2_kidneys->setCurrentIndex(qry.value(record.indexOf("kidneys")).toInt());
            ui->gestation2_kidneys_description->setText(qry.value(record.indexOf("kidneys_descriptions")).toString());
            ui->gestation2_ureter->setCurrentIndex(qry.value(record.indexOf("ureter")).toInt());
            ui->gestation2_ureter_description->setText(qry.value(record.indexOf("ureter_descriptions")).toString());
            ui->gestation2_bladder->setCurrentIndex(qry.value(record.indexOf("bladder")).toInt());

            // tableGestation2_other
            ui->gestation2_extremities->setCurrentIndex(qry.value(record.indexOf("extremities")).toInt());
            ui->gestation2_extremities_description->setText(qry.value(record.indexOf("extremities_descriptions")).toString());
            ui->gestation2_placenta->setCurrentIndex(qry.value(record.indexOf("placenta")).toInt());
            ui->gestation2_placenta_localization->setText(qry.value(record.indexOf("placentaLocalization")).toString());
            ui->gestation2_placentaDegreeMaturation->setText(qry.value(record.indexOf("placentaDegreeMaturation")).toString());
            ui->gestation2_placentaDepth->setText(qry.value(record.indexOf("placentaDepth")).toString());
            ui->gestation2_placentaStructure->setCurrentIndex(qry.value(record.indexOf("placentaStructure")).toInt());
            ui->gestation2_placentaStructure_description->setText(qry.value(record.indexOf("placentaStructure_descriptions")).toString());
            ui->gestation2_umbilicalCordon->setCurrentIndex(qry.value(record.indexOf("umbilicalCordon")).toInt());
            ui->gestation2_umbilicalCordon_description->setText(qry.value(record.indexOf("umbilicalCordon_description")).toString());
            ui->gestation2_insertionPlacenta->setCurrentIndex(qry.value(record.indexOf("insertionPlacenta")).toInt());
            ui->gestation2_amnioticIndex->setText(qry.value(record.indexOf("amnioticIndex")).toString());
            ui->gestation2_amnioticIndexAspect->setCurrentIndex(qry.value(record.indexOf("amnioticIndexAspect")).toInt());
            ui->gestation2_amnioticBedDepth->setText(qry.value(record.indexOf("amnioticBedDepth")).toString());
            ui->gestation2_cervix->setText(qry.value(record.indexOf("cervix")).toString());
            ui->gestation2_cervix_description->setText(qry.value(record.indexOf("cervix_description")).toString());
            ui->gestation2_fetusMass->setText(qry.value(record.indexOf("fetusMass")).toString());
            ui->gestation2_fetusSex->setCurrentIndex(qry.value(record.indexOf("externalGenitalOrgans")).toInt());
            o.updateDescriptionFetusWeight();

            // tableGestation2_doppler
            ui->gestation2_ombilic_PI->setText(qry.value(record.indexOf("ombilic_PI")).toString());
            ui->gestation2_ombilic_RI->setText(qry.value(record.indexOf("ombilic_RI")).toString());
            ui->gestation2_ombilic_SD->setText(qry.value(record.indexOf("ombilic_SD")).toString());
            ui->gestation2_ombilic_flux->setCurrentIndex(qry.value(record.indexOf("ombilic_flux")).toInt());
            ui->gestation2_cerebral_PI->setText(qry.value(record.indexOf("cerebral_PI")).toString());
            ui->gestation2_cerebral_RI->setText(qry.value(record.indexOf("cerebral_RI")).toString());
            ui->gestation2_cerebral_SD->setText(qry.value(record.indexOf("cerebral_SD")).toString());
            ui->gestation2_cerebral_flux->setCurrentIndex(qry.value(record.indexOf("cerebral_flux")).toInt());
            ui->gestation2_uterRight_PI->setText(qry.value(record.indexOf("uterRight_PI")).toString());
            ui->gestation2_uterRight_RI->setText(qry.value(record.indexOf("uterRight_RI")).toString());
            ui->gestation2_uterRight_SD->setText(qry.value(record.indexOf("uterRight_SD")).toString());
            ui->gestation2_uterRight_flux->setCurrentIndex(qry.value(record.indexOf("uterRight_flux")).toInt());
            ui->gestation2_uterLeft_PI->setText(qry.value(record.indexOf("uterLeft_PI")).toString());
            ui->gestation2_uterLeft_RI->setText(qry.value(record.indexOf("uterLeft_RI")).toString());
            ui->gestation2_uterLeft_SD->setText(qry.value(record.indexOf("uterLeft_SD")).toString());
            ui->gestation2_uterLeft_flux->setCurrentIndex(qry.value(record.indexOf("uterLeft_flux")).toInt());
            ui->gestation2_ductVenos->setCurrentIndex(qry.value(record.indexOf("ductVenos")).toInt());
            o.updateTextDescriptionDoppler();
        }
    }
}

void DocReportEchoHandler::Impl::setData_LymphNodes()
{
    // actualizam datele flag-lor si verificam daca ales flagul
    if (! (o.getSectionsSystem() & DocReportEcho::LymphNodes))
        return;

    // determinam UI
    auto ui = o.uiPtr();

    QSqlQuery qry;
    qry.prepare(R"(
        SELECT
            *
        FROM
            tableSofTissuesLymphNodes
        WHERE
            id_reportEcho = ?
    )");
    qry.addBindValue(o.getId());
    if (qry.exec()) {
        while (qry.next()) {
            QSqlRecord record = qry.record();

            ui->ln_zoneArea->setText(qry.value(record.indexOf("examinedArea")).toString());
            ui->ln_indications->setText(qry.value(record.indexOf("clinicalIndications")).toString());
            ui->ln_recommand->setText(qry.value(record.indexOf("recommendation")).toString());
            ui->ln_conluzion->setPlainText(qry.value(record.indexOf("concluzion")).toString());

            const QString name_investigation = qry.value(record.indexOf("section_type")).toString();
            if (name_investigation == "soft_tissues") {
                ui->ln_typeInvestig->setCurrentIndex(1);
                ui->tabWidget_LymphNodes->setCurrentIndex(0);
                ui->tabLympNodes->setEnabled(false);
                ui->ln_skinStructureTissue->setCurrentText(qry.value(record.indexOf("skin_structure")).toString());
                ui->ln_subcutanTissue->setCurrentText(qry.value(record.indexOf("subcutaneous_tissue")).toString());
                ui->ln_localizationTissue->setText(qry.value(record.indexOf("lesion_location")).toString());
                ui->ln_sizeTissue->setText(qry.value(record.indexOf("lesion_size")).toString());
                ui->ln_ecogenityTissue->setCurrentText(qry.value(record.indexOf("lesion_echogenicity")).toString());
                ui->ln_conturTissue->setCurrentText(qry.value(record.indexOf("lesion_contour")).toString());
                ui->ln_DopplerTissue->setCurrentText(qry.value(record.indexOf("lesion_vascularization")).toString());
                ui->ln_otherChangeTissue->setPlainText(qry.value(record.indexOf("other_changes")).toString());
            } else if (name_investigation == "lymph_nodes") {
                ui->ln_typeInvestig->setCurrentIndex(2);
                ui->tabWidget_LymphNodes->setCurrentIndex(1);
                ui->tabSoftTissues->setEnabled(false);
                ui->ln_nrNodes->setText(qry.value(record.indexOf("ln_number")).toString());
                ui->ln_sizeNode->setText(qry.value(record.indexOf("ln_size_nodes")).toString());
                ui->ln_formNode->setCurrentText(qry.value(record.indexOf("ln_shape")).toString());
                ui->ln_hillNode->setCurrentText(qry.value(record.indexOf("ln_echogenic_hilum")).toString());
                ui->ln_corticalNode->setCurrentText(qry.value(record.indexOf("ln_cortex")).toString());
                ui->ln_structureNode->setCurrentText(qry.value(record.indexOf("ln_structure")).toString());
                ui->ln_contourNode->setCurrentText(qry.value(record.indexOf("ln_contour")).toString());
                ui->ln_dopplerNode->setCurrentText(qry.value(record.indexOf("ln_vascularization")).toString());
                ui->ln_otherChangeNode->setPlainText(qry.value(record.indexOf("ln_associated_changes")).toString());
            } else {
                ui->ln_typeInvestig->setCurrentIndex(0);
                ui->ln_skinStructureTissue->setCurrentText(qry.value(record.indexOf("skin_structure")).toString());
                ui->ln_subcutanTissue->setCurrentText(qry.value(record.indexOf("subcutaneous_tissue")).toString());
                ui->ln_localizationTissue->setText(qry.value(record.indexOf("lesion_location")).toString());
                ui->ln_sizeTissue->setText(qry.value(record.indexOf("lesion_size")).toString());
                ui->ln_ecogenityTissue->setCurrentText(qry.value(record.indexOf("lesion_echogenicity")).toString());
                ui->ln_conturTissue->setCurrentText(qry.value(record.indexOf("lesion_contour")).toString());
                ui->ln_DopplerTissue->setCurrentText(qry.value(record.indexOf("lesion_vascularization")).toString());
                ui->ln_otherChangeTissue->setPlainText(qry.value(record.indexOf("other_changes")).toString());
                ui->ln_nrNodes->setText(qry.value(record.indexOf("ln_number")).toString());
                ui->ln_sizeNode->setText(qry.value(record.indexOf("ln_size_nodes")).toString());
                ui->ln_formNode->setCurrentText(qry.value(record.indexOf("ln_shape")).toString());
                ui->ln_hillNode->setCurrentText(qry.value(record.indexOf("ln_echogenic_hilum")).toString());
                ui->ln_corticalNode->setCurrentText(qry.value(record.indexOf("ln_cortex")).toString());
                ui->ln_structureNode->setCurrentText(qry.value(record.indexOf("ln_structure")).toString());
                ui->ln_contourNode->setCurrentText(qry.value(record.indexOf("ln_contour")).toString());
                ui->ln_dopplerNode->setCurrentText(qry.value(record.indexOf("ln_vascularization")).toString());
                ui->ln_otherChangeNode->setPlainText(qry.value(record.indexOf("ln_associated_changes")).toString());
            }
        }
    }
}
