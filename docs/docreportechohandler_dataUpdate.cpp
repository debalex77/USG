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

static bool updateMainDocument(DocReportEcho*o, QSqlQuery &qry, QString &err)
{
    // determinam UI
    auto ui = o->uiPtr();

    // actualizam datele flag-lor
    DocReportEcho::SectionsSystem s = o->getSectionsSystem();

    qry.prepare(R"(
        UPDATE reportEcho SET
            deletionMark = ?,
            numberDoc    = ?,
            dateDoc      = ?,
            id_pacients  = ?,
            id_orderEcho = ?,
            t_organs_internal = ?,
            t_urinary_system  = ?,
            t_prostate   = ?,
            t_gynecology = ?,
            t_breast     = ?,
            t_thyroid    = ?,
            t_gestation0 = ?,
            t_gestation1 = ?,
            t_gestation2 = ?,
            t_gestation3 = ?,
            t_lymphNodes = ?,
            id_users     = ?,
            concluzion   = ?,
            comment      = ?,
            attachedImages = ?
        WHERE
            id = ?;
    )");

    qry.addBindValue(o->getPost());
    qry.addBindValue(ui->editDocNumber->text());
    qry.addBindValue(ui->editDocDate->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
    qry.addBindValue(o->getIdPacient());
    qry.addBindValue(o->getIdDocOrderEcho());

    auto b = [=](bool val) { return globals().thisMySQL ? val : int(val); };
    qry.addBindValue(b(s & DocReportEcho::OrgansInternal));
    qry.addBindValue(b(s & DocReportEcho::UrinarySystem));
    qry.addBindValue(b(s & DocReportEcho::Prostate));
    qry.addBindValue(b(s & DocReportEcho::Gynecology));
    qry.addBindValue(b(s & DocReportEcho::Breast));
    qry.addBindValue(b(s & DocReportEcho::Thyroid));
    qry.addBindValue(b(s & DocReportEcho::Gestation0));
    qry.addBindValue(b(s & DocReportEcho::Gestation1));
    qry.addBindValue(b(s & DocReportEcho::Gestation2));
    qry.addBindValue(QVariant());
    qry.addBindValue(b(s & DocReportEcho::LymphNodes));

    qry.addBindValue(o->getIdUser());
    qry.addBindValue(ui->concluzion->toPlainText());
    qry.addBindValue((ui->comment->toPlainText().isEmpty())
                         ? QVariant()
                         : ui->comment->toPlainText());
    qry.addBindValue((o->getCountImages() == 0) ? QVariant() : 1);
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'reportEcho' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

static bool updateOrgansInternal(DocReportEcho*o, QSqlQuery &qry, QString &err)
{
    // determinam UI
    auto ui = o->uiPtr();

    //*********************************************
    // -- tableLiver
    qry.prepare(R"(
        UPDATE
            tableLiver
        SET
            `left` = ?,
            `right` = ?,
            contur = ?,
            parenchim = ?,
            ecogenity = ?,
            formations = ?,
            ductsIntrahepatic = ?,
            porta = ?,
            lienalis = ?,
            concluzion = ?,
            recommendation = ?
        WHERE
            id_reportEcho = ?;
    )");

    qry.addBindValue(ui->liver_left->text());
    qry.addBindValue(ui->liver_right->text());
    qry.addBindValue(ui->liver_contour->text());
    qry.addBindValue(ui->liver_parenchyma->text());
    qry.addBindValue(ui->liver_ecogenity->text());
    qry.addBindValue(ui->liver_formations->text());
    qry.addBindValue(ui->liver_duct_hepatic->text());
    qry.addBindValue(ui->liver_porta->text());
    qry.addBindValue(ui->liver_lienalis->text());
    qry.addBindValue(ui->organsInternal_concluzion->toPlainText());
    qry.addBindValue((ui->organsInternal_recommendation->text().isEmpty())
                         ? QVariant()
                         : ui->organsInternal_recommendation->text());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableLiver' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    //*********************************************
    // -- tableCholecist
    qry.prepare(R"(
        UPDATE
            tableCholecist
        SET
            form = ?,
            dimens = ?,
            walls = ?,
            choledoc = ?,
            formations = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->cholecist_form->text());
    qry.addBindValue(ui->cholecist_dimens->text().isEmpty() ? QVariant() : ui->cholecist_dimens->text());
    qry.addBindValue(ui->cholecist_walls->text().isEmpty() ? QVariant() : ui->cholecist_walls->text());
    qry.addBindValue(ui->cholecist_coledoc->text().isEmpty() ? QVariant() : ui->cholecist_coledoc->text());
    qry.addBindValue(ui->cholecist_formations->text().isEmpty() ? QVariant() : ui->cholecist_formations->text());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableCholecist' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    //*********************************************
    // -- tablePancreas
    qry.prepare(R"(
        UPDATE
            tablePancreas
        SET
            cefal = ?,
            corp = ?,
            tail = ?,
            texture = ?,
            ecogency = ?,
            formations = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->pancreas_cefal->text());
    qry.addBindValue(ui->pancreas_corp->text());
    qry.addBindValue(ui->pancreas_tail->text());
    qry.addBindValue(ui->pancreas_parenchyma->text());
    qry.addBindValue(ui->pancreas_ecogenity->text());
    qry.addBindValue(ui->pancreas_formations->text());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tablePancreas' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    //*********************************************
    // -- tableSpleen
    qry.prepare(R"(
        UPDATE
            tableSpleen
        SET
            dimens = ?,
            contur = ?,
            parenchim = ?,
            formations = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->spleen_dimens->text());
    qry.addBindValue(ui->spleen_contour->text());
    qry.addBindValue(ui->spleen_parenchyma->text());
    qry.addBindValue(ui->spleen_formations->text());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableSpleen' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    //*********************************************
    // -- tableIntestinalLoop
    qry.prepare(R"(
        UPDATE
            tableIntestinalLoop
        SET
            formations = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->intestinalHandles->toPlainText());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableIntestinalLoop' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

static bool updateUrinarySystem(DocReportEcho*o, QSqlQuery &qry, QString &err)
{
    // determinam UI
    auto ui = o->uiPtr();

    //*********************************************
    // -- tableKidney
    qry.prepare(R"(
        UPDATE
            tableKidney
        SET
            contour_right = ?,
            contour_left = ?,
            dimens_right = ?,
            dimens_left = ?,
            corticomed_right = ?,
            corticomed_left = ?,
            pielocaliceal_right = ?,
            pielocaliceal_left = ?,
            formations = ?,
            suprarenal_formations = ?,
            concluzion = ?,
            recommendation = ?
        WHERE
            id_reportEcho = ?;
    )");

    qry.addBindValue(ui->kidney_contur_right->currentText());
    qry.addBindValue(ui->kidney_contur_left->currentText());
    qry.addBindValue(ui->kidney_right->text());
    qry.addBindValue(ui->kidney_left->text());
    qry.addBindValue(ui->kidney_corticomed_right->text());
    qry.addBindValue(ui->kidney_corticomed_left->text());
    qry.addBindValue(ui->kidney_pielocaliceal_right->text());
    qry.addBindValue(ui->kidney_pielocaliceal_left->text());
    qry.addBindValue(ui->kidney_formations->text());
    qry.addBindValue(ui->adrenalGlands->toPlainText());
    qry.addBindValue(ui->urinary_system_concluzion->toPlainText());
    qry.addBindValue((ui->urinary_system_recommendation->text().isEmpty())
                         ? QVariant()
                         : ui->urinary_system_recommendation->text());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableKidney' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    //*********************************************
    // -- tableBladder
    qry.prepare(R"(
        UPDATE
            tableBladder
        SET
            volum = ?,
            walls = ?,
            formations = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->bladder_volum->text());
    qry.addBindValue(ui->bladder_walls->text());
    qry.addBindValue(ui->bladder_formations->text());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableBladder' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

static bool updateProstate(DocReportEcho*o, QSqlQuery &qry, QString &err)
{
    // determinam UI
    auto ui = o->uiPtr();

    qry.prepare(R"(
        UPDATE
            tableProstate
        SET
            dimens = ?,
            volume = ?,
            ecostructure = ?,
            contour = ?,
            ecogency = ?,
            formations = ?,
            transrectal = ?,
            concluzion = ?,
            recommendation = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->prostate_dimens->text());
    qry.addBindValue(ui->prostate_volum->text());
    qry.addBindValue(ui->prostate_ecostructure->text());
    qry.addBindValue(ui->prostate_contur->text());
    qry.addBindValue(ui->prostate_ecogency->text());
    qry.addBindValue(ui->prostate_formations->text());
    qry.addBindValue(globals().thisMySQL
                         ? QVariant(ui->prostate_radioBtn_transrectal->isChecked())
                         : QVariant(int(ui->prostate_radioBtn_transrectal->isChecked())));
    qry.addBindValue(ui->prostate_concluzion->toPlainText());
    qry.addBindValue((ui->prostate_recommendation->text().isEmpty())
                         ? QVariant()
                         : ui->prostate_recommendation->text());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableProstate' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

static bool updateGynecology(DocReportEcho*o, QSqlQuery &qry, QString &err)
{
    // determinam UI
    auto ui = o->uiPtr();

    qry.prepare(R"(
        UPDATE
            tableGynecology
        SET
            transvaginal = ?,
            dateMenstruation = ?,
            antecedent = ?,
            uterus_dimens = ?,
            uterus_pozition = ?,
            uterus_ecostructure = ?,
            uterus_formations = ?,
            junctional_zone = ?,
            junctional_zone_description = ?,
            ecou_dimens = ?,
            ecou_ecostructure = ?,
            cervix_dimens = ?,
            cervix_ecostructure = ?,
            cervical_canal = ?,
            cervical_canal_formations = ?,
            douglas = ?,
            plex_venos = ?,
            ovary_right_dimens = ?,
            ovary_left_dimens = ?,
            ovary_right_volum = ?,
            ovary_left_volum = ?,
            ovary_right_follicle = ?,
            ovary_left_follicle = ?,
            ovary_right_formations = ?,
            ovary_left_formations = ?,
            fallopian_tubes = ?,
            fallopian_tubes_formations = ?,
            concluzion = ?,
            recommendation = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(globals().thisMySQL
                         ? QVariant(ui->gynecology_btn_transvaginal->isChecked())
                         : QVariant(int(ui->gynecology_btn_transvaginal->isChecked())));
    qry.addBindValue(ui->gynecology_dateMenstruation->date().toString("yyyy-MM-dd"));
    qry.addBindValue(ui->gynecology_antecedent->text().isEmpty()
                         ? QVariant()
                         : ui->gynecology_antecedent->text());
    qry.addBindValue(ui->gynecology_uterus_dimens->text());
    qry.addBindValue(ui->gynecology_uterus_pozition->text());
    qry.addBindValue(ui->gynecology_uterus_ecostructure->text());
    qry.addBindValue(ui->gynecology_uterus_formations->toPlainText());
    qry.addBindValue(ui->gynecology_combo_jonctional_zone->currentText());
    qry.addBindValue(ui->gynecology_jonctional_zone_description->text().isEmpty()
                         ? QVariant()
                         : ui->gynecology_jonctional_zone_description->text());
    qry.addBindValue(ui->gynecology_ecou_dimens->text());
    qry.addBindValue(ui->gynecology_ecou_ecostructure->text());
    qry.addBindValue(ui->gynecology_cervix_dimens->text());
    qry.addBindValue(ui->gynecology_cervix_ecostructure->text());
    qry.addBindValue(ui->gynecology_combo_canal_cervical->currentText());
    qry.addBindValue(ui->gynecology_canal_cervical_formations->text().isEmpty()
                         ? QVariant()
                         : ui->gynecology_canal_cervical_formations->text());
    qry.addBindValue(ui->gynecology_douglas->text());
    qry.addBindValue(ui->gynecology_plex_venos->text());
    qry.addBindValue(ui->gynecology_ovary_right_dimens->text());
    qry.addBindValue(ui->gynecology_ovary_left_dimens->text());
    qry.addBindValue(ui->gynecology_ovary_right_volum->text());
    qry.addBindValue(ui->gynecology_ovary_left_volum->text());
    qry.addBindValue(ui->gynecology_follicule_right->text());
    qry.addBindValue(ui->gynecology_follicule_left->text());
    qry.addBindValue(ui->gynecology_ovary_formations_right->toPlainText());
    qry.addBindValue(ui->gynecology_ovary_formations_left->toPlainText());
    qry.addBindValue(ui->gynecology_combo_fallopian_tubes->currentText());
    qry.addBindValue(ui->gynecology_fallopian_tubes_formations->text().isEmpty()
                         ? QVariant()
                         : ui->gynecology_fallopian_tubes_formations->text());
    qry.addBindValue(ui->gynecology_concluzion->toPlainText());
    qry.addBindValue(ui->gynecology_recommendation->text().isEmpty()
                         ? QVariant()
                         : ui->gynecology_recommendation->text());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGynecology' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

static bool updateBreast(DocReportEcho*o, QSqlQuery &qry, QString &err)
{
    // determinam UI
    auto ui = o->uiPtr();

    qry.prepare(R"(
        UPDATE
            tableBreast
        SET
            breast_right_ecostrcture = ?,
            breast_right_duct = ?,
            breast_right_ligament = ?,
            breast_right_formations = ?,
            breast_right_ganglions = ?,
            breast_left_ecostrcture = ?,
            breast_left_duct = ?,
            breast_left_ligament = ?,
            breast_left_formations = ?,
            breast_left_ganglions = ?,
            concluzion = ?,
            recommendation = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->breast_right_ecostructure->text());
    qry.addBindValue(ui->breast_right_duct->text());
    qry.addBindValue(ui->breast_right_ligament->text());
    qry.addBindValue(ui->breast_right_formations->toPlainText());
    qry.addBindValue(ui->breast_right_ganglions->text());
    qry.addBindValue(ui->breast_left_ecostructure->text());
    qry.addBindValue(ui->breast_left_duct->text());
    qry.addBindValue(ui->breast_left_ligament->text());
    qry.addBindValue(ui->breast_left_formations->toPlainText());
    qry.addBindValue(ui->breast_left_ganglions->text());
    qry.addBindValue(ui->breast_concluzion->toPlainText());
    qry.addBindValue(ui->breast_recommendation->text().isEmpty()
                         ? QVariant()
                         : ui->breast_recommendation->text());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableBreast' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

static bool updateThyroid(DocReportEcho*o, QSqlQuery &qry, QString &err)
{
    // determinam UI
    auto ui = o->uiPtr();

    qry.prepare(R"(
        UPDATE
            tableThyroid
        SET
            thyroid_right_dimens = ?,
            thyroid_right_volum = ?,
            thyroid_left_dimens = ?,
            thyroid_left_volum = ?,
            thyroid_istm = ?,
            thyroid_ecostructure = ?,
            thyroid_formations = ?,
            thyroid_ganglions = ?,
            concluzion = ?,
            recommendation = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->thyroid_right_dimens->text());
    qry.addBindValue(ui->thyroid_right_volum->text());
    qry.addBindValue(ui->thyroid_left_dimens->text());
    qry.addBindValue(ui->thyroid_left_volum->text());
    qry.addBindValue(ui->thyroid_istm->text());
    qry.addBindValue(ui->thyroid_ecostructure->text());
    qry.addBindValue(ui->thyroid_formations->toPlainText());
    qry.addBindValue(ui->thyroid_ganglions->text());
    qry.addBindValue(ui->thyroid_concluzion->toPlainText());
    qry.addBindValue(ui->thyroid_recommendation->text().isEmpty()
                         ? QVariant()
                         : ui->thyroid_recommendation->text());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableThyroid' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

static bool updateGestation0(DocReportEcho*o, QSqlQuery &qry, QString &err)
{
    // determinam UI
    auto ui = o->uiPtr();

    qry.prepare(R"(
        UPDATE
            tableGestation0
        SET
            view_examination = ?,
            antecedent = ?,
            gestation_age = ?,
            GS = ?,
            GS_age = ?,
            CRL = ?,
            CRL_age = ?,
            BCF = ?,
            liquid_amniotic = ?,
            miometer = ?,
            cervix = ?,
            ovary = ?,
            concluzion = ?,
            recommendation = ?,
            lmp = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(o->getViewExaminationGestation(0)),
        qry.addBindValue(ui->gestation0_antecedent->text().isEmpty()
                             ? QVariant()
                             : ui->gestation0_antecedent->text());
    qry.addBindValue(ui->gestation0_gestation->text());
    qry.addBindValue(ui->gestation0_GS_dimens->text());
    qry.addBindValue(ui->gestation0_GS_age->text());
    qry.addBindValue(ui->gestation0_CRL_dimens->text());
    qry.addBindValue(ui->gestation0_CRL_age->text());
    qry.addBindValue(ui->gestation0_BCF->text());
    qry.addBindValue(ui->gestation0_liquid_amniotic->text());
    qry.addBindValue(ui->gestation0_miometer->text());
    qry.addBindValue(ui->gestation0_cervix->text());
    qry.addBindValue(ui->gestation0_ovary->text());
    qry.addBindValue(ui->gestation0_concluzion->toPlainText());
    qry.addBindValue(ui->gestation0_recommendation->text().isEmpty()
                         ? QVariant()
                         : ui->gestation0_recommendation->text());
    qry.addBindValue(ui->gestation0_LMP->date().toString("yyyy-MM-dd"));
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation0' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

static bool updateGestation1(DocReportEcho*o, QSqlQuery &qry, QString &err)
{
    // determinam UI
    auto ui = o->uiPtr();

    qry.prepare(R"(
        UPDATE
            tableGestation1
        SET
            view_examination = ?,
            antecedent = ?,
            lmp = ?,
            gestation_age = ?,
            CRL = ?,
            CRL_age = ?,
            BPD = ?,
            BPD_age = ?,
            NT = ?,
            NT_percent = ?,
            BN = ?,
            BN_percent = ?,
            BCF = ?,
            FL = ?,
            FL_age = ?,
            callote_cranium = ?,
            plex_choroid = ?,
            vertebral_column = ?,
            stomach = ?,
            bladder = ?,
            diaphragm = ?,
            abdominal_wall = ?,
            location_placenta = ?,
            sac_vitelin = ?,
            amniotic_liquid = ?,
            miometer = ?,
            cervix = ?,
            ovary = ?,
            concluzion = ?,
            recommendation = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(o->getViewExaminationGestation(1));
    qry.addBindValue(ui->gestation1_antecedent->text().isEmpty()
                         ? QVariant()
                         : ui->gestation1_antecedent->text());
    qry.addBindValue(ui->gestation1_LMP->date().toString("yyy-MM-dd"));
    qry.addBindValue(ui->gestation1_gestation->text());
    qry.addBindValue(ui->gestation1_CRL_dimens->text());
    qry.addBindValue(ui->gestation1_CRL_age->text());
    qry.addBindValue(ui->gestation1_BPD_dimens->text());
    qry.addBindValue(ui->gestation1_BPD_age->text());
    qry.addBindValue(ui->gestation1_NT_dimens->text());
    qry.addBindValue(ui->gestation1_NT_percent->text());
    qry.addBindValue(ui->gestation1_BN_dimens->text());
    qry.addBindValue(ui->gestation1_BN_percent->text());
    qry.addBindValue(ui->gestation1_BCF->text());
    qry.addBindValue(ui->gestation1_FL_dimens->text());
    qry.addBindValue(ui->gestation1_FL_age->text());
    qry.addBindValue(ui->gestation1_callote_cranium->text());
    qry.addBindValue(ui->gestation1_plex_choroid->text());
    qry.addBindValue(ui->gestation1_vertebral_column->text());
    qry.addBindValue(ui->gestation1_stomach->text());
    qry.addBindValue(ui->gestation1_bladder->text());
    qry.addBindValue(ui->gestation1_diaphragm->text());
    qry.addBindValue(ui->gestation1_abdominal_wall->text());
    qry.addBindValue(ui->gestation1_location_placenta->text());
    qry.addBindValue(ui->gestation1_sac_vitelin->text());
    qry.addBindValue(ui->gestation1_amniotic_liquid->text());
    qry.addBindValue(ui->gestation1_miometer->text());
    qry.addBindValue(ui->gestation1_cervix->text());
    qry.addBindValue(ui->gestation1_ovary->text());
    qry.addBindValue(ui->gestation1_concluzion->toPlainText());
    qry.addBindValue(ui->gestation1_recommendation->text().isEmpty()
                         ? QVariant()
                         : ui->gestation1_recommendation->text());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation1' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

static bool updateGestation2(DocReportEcho*o, QSqlQuery &qry, QString &err)
{
    // determinam UI
    auto ui = o->uiPtr();

    // main table
    qry.prepare(R"(
        UPDATE
            tableGestation2
        SET
            gestation_age = ?,
            trimestru = ?,
            dateMenstruation = ?,
            view_examination = ?,
            single_multiple_pregnancy = ?,
            single_multiple_pregnancy_description = ?,
            antecedent = ?,
            comment = ?,
            concluzion = ?,
            recommendation = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->gestation2_gestation_age->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_gestation_age->text());
    qry.addBindValue(ui->gestation2_trimestru2->isChecked() ? 2 : 3);
    qry.addBindValue(ui->gestation2_dateMenstruation->date().toString("yyyy-MM-dd"));
    qry.addBindValue(ui->gestation2_view_examination->currentIndex());
    qry.addBindValue(ui->gestation2_pregnancy->currentIndex());
    qry.addBindValue(ui->gestation2_pregnancy_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_pregnancy_description->text());
    qry.addBindValue(QVariant());
    qry.addBindValue(ui->gestation2_comment->toPlainText().isEmpty()
                         ? QVariant()
                         : ui->gestation2_comment->toPlainText());
    qry.addBindValue(ui->gestation2_concluzion->toPlainText());
    qry.addBindValue(ui->gestation2_recommendation->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_recommendation->text());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation2' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // biometry
    qry.prepare(R"(
        UPDATE
            tableGestation2_biometry
        SET
            BPD = ?,
            BPD_age = ?,
            HC = ?,
            HC_age = ?,
            AC = ?,
            AC_age = ?,
            FL = ?,
            FL_age = ?,
            FetusCorresponds = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->gestation2_bpd->text());
    qry.addBindValue(ui->gestation2_bpd_age->text());
    qry.addBindValue(ui->gestation2_hc->text());
    qry.addBindValue(ui->gestation2_hc_age->text());
    qry.addBindValue(ui->gestation2_ac->text());
    qry.addBindValue(ui->gestation2_ac_age->text());
    qry.addBindValue(ui->gestation2_fl->text());
    qry.addBindValue(ui->gestation2_fl_age->text());
    qry.addBindValue(ui->gestation2_fetus_age->text());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation2_biometry' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // cranium
    qry.prepare(R"(
        UPDATE
            tableGestation2_cranium
        SET
            calloteCranium = ?,
            facialeProfile = ?,
            nasalBones = ?,
            nasalBones_dimens = ?,
            eyeball = ?,
            eyeball_desciption = ?,
            nasolabialTriangle = ?,
            nasolabialTriangle_description = ?,
            nasalFold = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->gestation2_calloteCranium->currentIndex());
    qry.addBindValue(ui->gestation2_facialeProfile->currentIndex());
    qry.addBindValue(ui->gestation2_nasalBones->currentIndex());
    qry.addBindValue(ui->gestation2_nasalBones_dimens->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_nasalBones_dimens->text());
    qry.addBindValue(ui->gestation2_eyeball->currentIndex());
    qry.addBindValue(ui->gestation2_eyeball_desciption->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_eyeball_desciption->text());
    qry.addBindValue(ui->gestation2_nasolabialTriangle->currentIndex());
    qry.addBindValue(ui->gestation2_nasolabialTriangle_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_nasolabialTriangle_description->text());
    qry.addBindValue(ui->gestation2_nasalFold->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_nasalFold->text());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation2_cranium' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // SNC
    qry.prepare(R"(
        UPDATE
            tableGestation2_SNC
        SET
            hemispheres = ?,
            fissureSilvius = ?,
            corpCalos = ?,
            ventricularSystem = ?,
            ventricularSystem_description = ?,
            cavityPellucidSeptum = ?,
            choroidalPlex = ?,
            choroidalPlex_description = ?,
            cerebellum = ?,
            cerebellum_description = ?,
            vertebralColumn = ?,
            vertebralColumn_description = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->gestation2_hemispheres->currentIndex());
    qry.addBindValue(ui->gestation2_fissureSilvius->currentIndex());
    qry.addBindValue(ui->gestation2_corpCalos->currentIndex());
    qry.addBindValue(ui->gestation2_ventricularSystem->currentIndex());
    qry.addBindValue(ui->gestation2_ventricularSystem_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_ventricularSystem_description->text());
    qry.addBindValue(ui->gestation2_cavityPellucidSeptum->currentIndex());
    qry.addBindValue(ui->gestation2_choroidalPlex->currentIndex());
    qry.addBindValue(ui->gestation2_choroidalPlex_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_choroidalPlex_description->text());
    qry.addBindValue(ui->gestation2_cerebellum->currentIndex());
    qry.addBindValue(ui->gestation2_cerebellum_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_cerebellum_description->text());
    qry.addBindValue(ui->gestation2_vertebralColumn->currentIndex());
    qry.addBindValue(ui->gestation2_vertebralColumn_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_vertebralColumn_description->text());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation2_SNC' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // heart
    qry.prepare(R"(
        UPDATE
            tableGestation2_heart
        SET
            `position` = ?,
            heartBeat = ?,
            heartBeat_frequency = ?,
            heartBeat_rhythm = ?,
            pericordialCollections = ?,
            planPatruCamere = ?,
            planPatruCamere_description = ?,
            ventricularEjectionPathLeft = ?,
            ventricularEjectionPathLeft_description = ?,
            ventricularEjectionPathRight = ?,
            ventricularEjectionPathRight_description = ?,
            intersectionVesselMagistral = ?,
            intersectionVesselMagistral_description = ?,
            planTreiVase = ?,
            planTreiVase_description = ?,
            archAorta = ?,
            planBicav = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->gestation2_heartPosition->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_heartPosition->text());
    qry.addBindValue(ui->gestation2_heartBeat->currentIndex());
    qry.addBindValue(ui->gestation2_heartBeat_frequency->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_heartBeat_frequency->text());
    qry.addBindValue(ui->gestation2_heartBeat_rhythm->currentIndex());
    qry.addBindValue(ui->gestation2_pericordialCollections->currentIndex());
    qry.addBindValue(ui->gestation2_planPatruCamere->currentIndex());
    qry.addBindValue(ui->gestation2_planPatruCamere_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_planPatruCamere_description->text());
    qry.addBindValue(ui->gestation2_ventricularEjectionPathLeft->currentIndex());
    qry.addBindValue(ui->gestation2_ventricularEjectionPathLeft_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_ventricularEjectionPathLeft_description->text());
    qry.addBindValue(ui->gestation2_ventricularEjectionPathRight->currentIndex());
    qry.addBindValue(ui->gestation2_ventricularEjectionPathRight_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_ventricularEjectionPathRight_description->text());
    qry.addBindValue(ui->gestation2_intersectionVesselMagistral->currentIndex());
    qry.addBindValue(ui->gestation2_intersectionVesselMagistral_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_intersectionVesselMagistral_description->text());
    qry.addBindValue(ui->gestation2_planTreiVase->currentIndex());
    qry.addBindValue(ui->gestation2_planTreiVase_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_planTreiVase_description->text());
    qry.addBindValue(ui->gestation2_archAorta->currentIndex());
    qry.addBindValue(ui->gestation2_planBicav->currentIndex());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation2_heart' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // thorax
    qry.prepare(R"(
        UPDATE
            tableGestation2_thorax
        SET
            pulmonaryAreas = ?,
            pulmonaryAreas_description = ?,
            pleuralCollections = ?,
            diaphragm = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->gestation2_pulmonaryAreas->currentIndex());
    qry.addBindValue(ui->gestation2_pulmonaryAreas_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_pulmonaryAreas_description->text());
    qry.addBindValue(ui->gestation2_pleuralCollections->currentIndex());
    qry.addBindValue(ui->gestation2_diaphragm->currentIndex());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation2_thorax' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // abdomen
    qry.prepare(R"(
        UPDATE
            tableGestation2_abdomen
        SET
            abdominalWall = ?,
            abdominalCollections = ?,
            stomach = ?,
            stomach_description = ?,
            abdominalOrgans = ?,
            cholecist = ?,
            cholecist_description = ?,
            intestine = ?,
            intestine_description = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->gestation2_abdominalWall->currentIndex());
    qry.addBindValue(ui->gestation2_abdominalCollections->currentIndex());
    qry.addBindValue(ui->gestation2_stomach->currentIndex());
    qry.addBindValue(ui->gestation2_stomach_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_stomach_description->text());
    qry.addBindValue(ui->gestation2_abdominalOrgans->currentIndex());
    qry.addBindValue(ui->gestation2_cholecist->currentIndex());
    qry.addBindValue(ui->gestation2_cholecist_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_cholecist_description->text());
    qry.addBindValue(ui->gestation2_intestine->currentIndex());
    qry.addBindValue(ui->gestation2_intestine_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_intestine_description->text());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation2_abdomen' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // s.urinar
    qry.prepare(R"(
        UPDATE
            tableGestation2_urinarySystem
        SET
            kidneys = ?,
            kidneys_descriptions = ?,
            ureter = ?,
            ureter_descriptions = ?,
            bladder = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->gestation2_kidneys->currentIndex());
    qry.addBindValue(ui->gestation2_kidneys_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_kidneys_description->text());
    qry.addBindValue(ui->gestation2_ureter->currentIndex());
    qry.addBindValue(ui->gestation2_ureter_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_ureter_description->text());
    qry.addBindValue(ui->gestation2_bladder->currentIndex());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation2_urinarySystem' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // other
    qry.prepare(R"(
        UPDATE
            tableGestation2_other
        SET
            externalGenitalOrgans = ?,
            externalGenitalOrgans_aspect = ?,
            extremities = ?,
            extremities_descriptions = ?,
            fetusMass = ?,
            placenta = ?,
            placentaLocalization = ?,
            placentaDegreeMaturation = ?,
            placentaDepth = ?,
            placentaStructure = ?,
            placentaStructure_descriptions = ?,
            umbilicalCordon = ?,
            umbilicalCordon_description = ?,
            insertionPlacenta = ?,
            amnioticIndex = ?,
            amnioticIndexAspect = ?,
            amnioticBedDepth = ?,
            cervix = ?,
            cervix_description = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->gestation2_fetusSex->currentIndex());
    qry.addBindValue(QVariant());
    qry.addBindValue(ui->gestation2_extremities->currentIndex());
    qry.addBindValue(ui->gestation2_extremities_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_extremities_description->text());
    qry.addBindValue(ui->gestation2_fetusMass->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_fetusMass->text());
    qry.addBindValue(ui->gestation2_placenta->currentIndex());
    qry.addBindValue(ui->gestation2_placenta_localization->text().isEmpty()
                         ? QVariant()
                         : ui->gestation1_location_placenta->text());
    qry.addBindValue(ui->gestation2_placentaDegreeMaturation->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_placentaDegreeMaturation->text());
    qry.addBindValue(ui->gestation2_placentaDepth->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_placentaDepth->text());
    qry.addBindValue(ui->gestation2_placentaStructure->currentIndex());
    qry.addBindValue(ui->gestation2_placentaStructure_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_placentaStructure_description->text());
    qry.addBindValue(ui->gestation2_umbilicalCordon->currentIndex());
    qry.addBindValue(ui->gestation2_umbilicalCordon_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_umbilicalCordon_description->text());
    qry.addBindValue(ui->gestation2_insertionPlacenta->currentIndex());
    qry.addBindValue(ui->gestation2_amnioticIndex->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_amnioticIndex->text());
    qry.addBindValue(ui->gestation2_amnioticIndexAspect->currentIndex());
    qry.addBindValue(ui->gestation2_amnioticBedDepth->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_amnioticBedDepth->text());
    qry.addBindValue(ui->gestation2_cervix->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_cervix->text());
    qry.addBindValue(ui->gestation2_cervix_description->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_cervix_description->text());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation2_other' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // doppler
    qry.prepare(R"(
        UPDATE
            tableGestation2_doppler
        SET
            ombilic_PI = ?,
            ombilic_RI = ?,
            ombilic_SD = ?,
            ombilic_flux = ?,
            cerebral_PI = ?,
            cerebral_RI = ?,
            cerebral_SD = ?,
            cerebral_flux = ?,
            uterRight_PI = ?,
            uterRight_RI = ?,
            uterRight_SD = ?,
            uterRight_flux = ?,
            uterLeft_PI = ?,
            uterLeft_RI = ?,
            uterLeft_SD = ?,
            uterLeft_flux = ?,
            ductVenos = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->gestation2_ombilic_PI->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_ombilic_PI->text());
    qry.addBindValue(ui->gestation2_ombilic_RI->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_ombilic_RI->text());
    qry.addBindValue(ui->gestation2_ombilic_SD->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_ombilic_SD->text());
    qry.addBindValue(ui->gestation2_ombilic_flux->currentIndex());
    qry.addBindValue(ui->gestation2_cerebral_PI->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_cerebral_PI->text());
    qry.addBindValue(ui->gestation2_cerebral_RI->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_cerebral_RI->text());
    qry.addBindValue(ui->gestation2_cerebral_SD->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_cerebral_SD->text());
    qry.addBindValue(ui->gestation2_cerebral_flux->currentIndex());
    qry.addBindValue(ui->gestation2_uterRight_PI->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_uterRight_PI->text());
    qry.addBindValue(ui->gestation2_uterRight_RI->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_uterRight_RI->text());
    qry.addBindValue(ui->gestation2_uterRight_SD->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_uterRight_SD->text());
    qry.addBindValue(ui->gestation2_uterRight_flux->currentIndex());
    qry.addBindValue(ui->gestation2_uterLeft_PI->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_uterLeft_PI->text());
    qry.addBindValue(ui->gestation2_uterLeft_RI->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_uterLeft_RI->text());
    qry.addBindValue(ui->gestation2_uterLeft_SD->text().isEmpty()
                         ? QVariant()
                         : ui->gestation2_uterLeft_SD->text());
    qry.addBindValue(ui->gestation2_uterLeft_flux->currentIndex());
    qry.addBindValue(ui->gestation2_ductVenos->currentIndex());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation2_doppler' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

static bool updateLymphNodes(DocReportEcho*o, QSqlQuery &qry, QString &err)
{
    // determinam UI
    auto ui = o->uiPtr();

    qry.prepare(R"(
        UPDATE
            tableSofTissuesLymphNodes
        SET
            section_type = ?,
            examinedArea = ?,
            clinicalIndications = ?,
            skin_structure = ?,
            subcutaneous_tissue = ?,
            lesion_location = ?,
            lesion_size = ?,
            lesion_echogenicity = ?,
            lesion_contour = ?,
            lesion_vascularization = ?,
            ln_number = ?,
            ln_size_nodes = ?,
            ln_shape = ?,
            ln_echogenic_hilum = ?,
            ln_cortex = ?,
            ln_structure = ?,
            ln_contour = ?,
            ln_vascularization = ?,
            ln_associated_changes = ?,
            other_changes = ?,
            concluzion = ?,
            recommendation = ?
        WHERE
            id_reportEcho = ?;
    )");
    QString type_investigation;
    if (ui->ln_typeInvestig->currentIndex() == 0)
        type_investigation = "tissues_nodes";
    else if (ui->ln_typeInvestig->currentIndex() == 1)
        type_investigation = "soft_tissues";
    else
        type_investigation = "lymph_nodes";

    qry.addBindValue(type_investigation);
    qry.addBindValue(ui->ln_zoneArea->text());
    qry.addBindValue(ui->ln_indications->text());
    // soft tissues
    qry.addBindValue((ui->ln_typeInvestig->currentIndex() == 2) ? QVariant() : ui->ln_skinStructureTissue->currentText());
    qry.addBindValue((ui->ln_typeInvestig->currentIndex() == 2) ? QVariant() : ui->ln_subcutanTissue->currentText());
    qry.addBindValue((ui->ln_typeInvestig->currentIndex() == 2) ? QVariant() : ui->ln_localizationTissue->text());
    qry.addBindValue((ui->ln_typeInvestig->currentIndex() == 2) ? QVariant() : ui->ln_sizeTissue->text());
    qry.addBindValue((ui->ln_typeInvestig->currentIndex() == 2) ? QVariant() : ui->ln_ecogenityTissue->currentText());
    qry.addBindValue((ui->ln_typeInvestig->currentIndex() == 2) ? QVariant() : ui->ln_conturTissue->currentText());
    qry.addBindValue((ui->ln_typeInvestig->currentIndex() == 2) ? QVariant() : ui->ln_DopplerTissue->currentText());
    // lymph nodes
    qry.addBindValue((ui->ln_typeInvestig->currentIndex() == 1) ? QVariant() : ui->ln_nrNodes->text().toInt());
    qry.addBindValue((ui->ln_typeInvestig->currentIndex() == 1) ? QVariant() : ui->ln_sizeNode->text());
    qry.addBindValue((ui->ln_typeInvestig->currentIndex() == 1) ? QVariant() : ui->ln_formNode->currentText());
    qry.addBindValue((ui->ln_typeInvestig->currentIndex() == 1) ? QVariant() : ui->ln_hillNode->currentText());
    qry.addBindValue((ui->ln_typeInvestig->currentIndex() == 1) ? QVariant() : ui->ln_corticalNode->currentText());
    qry.addBindValue((ui->ln_typeInvestig->currentIndex() == 1) ? QVariant() : ui->ln_structureNode->currentText());
    qry.addBindValue((ui->ln_typeInvestig->currentIndex() == 1) ? QVariant() : ui->ln_contourNode->currentText());
    qry.addBindValue((ui->ln_typeInvestig->currentIndex() == 1) ? QVariant() : ui->ln_dopplerNode->currentText());
    qry.addBindValue((ui->ln_typeInvestig->currentIndex() == 1) ? QVariant() : ui->ln_otherChangeNode->toPlainText());

    if (ui->ln_typeInvestig->currentIndex() == 0 || ui->ln_typeInvestig->currentIndex() == 1 || ui->ln_typeInvestig->currentIndex() == 2)
        qry.addBindValue(ui->ln_otherChangeTissue->toPlainText().isEmpty() ? QVariant() : ui->ln_otherChangeTissue->toPlainText());

    qry.addBindValue(ui->ln_conluzion->toPlainText());
    qry.addBindValue(ui->ln_recommand->text().isEmpty() ? QVariant() : ui->ln_recommand->text());
    qry.addBindValue(o->getId());
    if (! qry.exec()) {
        err = QObject::tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableSofTissuesLymphNodes' - %2")
                  .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? QObject::tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

bool DocReportEchoHandler::Impl::updatingDocumentDataIntoTables(DataBase *db, QString &details_error)
{
    // actualizam datele flag-lor
    DocReportEcho::SectionsSystem s = o.getSectionsSystem();

    db->getDatabase().transaction();

    QSqlQuery qry;

    auto rollbackAndFail = [&](const QString &err) {
        details_error = err;
        db->getDatabase().rollback();
        return false;
    };

    try {

        if (! updateMainDocument(&o, qry, details_error))
            return rollbackAndFail(details_error);
        if ((s & DocReportEcho::OrgansInternal) && ! updateOrgansInternal(&o, qry, details_error))
            return rollbackAndFail(details_error);
        if ((s & DocReportEcho::UrinarySystem) && ! updateUrinarySystem(&o, qry, details_error))
            return rollbackAndFail(details_error);
        if ((s & DocReportEcho::Prostate) && ! updateProstate(&o, qry, details_error))
            return rollbackAndFail(details_error);
        if ((s & DocReportEcho::Gynecology) && ! updateGynecology(&o, qry, details_error))
            return rollbackAndFail(details_error);
        if ((s & DocReportEcho::Breast) && ! updateBreast(&o, qry, details_error))
            return rollbackAndFail(details_error);
        if ((s & DocReportEcho::Thyroid) && ! updateThyroid(&o, qry, details_error))
            return rollbackAndFail(details_error);
        if ((s & DocReportEcho::Gestation0) && ! updateGestation0(&o, qry, details_error))
            return rollbackAndFail(details_error);
        if ((s & DocReportEcho::Gestation1) && ! updateGestation1(&o, qry, details_error))
            return rollbackAndFail(details_error);
        if ((s & DocReportEcho::Gestation2) && ! updateGestation2(&o, qry, details_error))
            return rollbackAndFail(details_error);
        if ((s & DocReportEcho::LymphNodes) && ! updateLymphNodes(&o, qry, details_error))
            return rollbackAndFail(details_error);

        return db->getDatabase().commit();

    } catch (...) {
        return rollbackAndFail(QObject::tr("Excepție neprevăzută la actualizarea datelor documentului."));
    }
}
