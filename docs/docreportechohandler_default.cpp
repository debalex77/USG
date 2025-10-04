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

void DocReportEchoHandler::Impl::setDefault_OrgansInternal()
{
    // actualizam datele flag-lor si verificam daca ales flagul
    if (! (o.getSectionsSystem() & DocReportEcho::OrgansInternal))
        return;

    // determinam UI
    auto ui = o.uiPtr();

    //--- liver
    if (ui->liver_contour->text().isEmpty())
        ui->liver_contour->setText("clar");
    if (ui->liver_parenchyma->text().isEmpty())
        ui->liver_parenchyma->setText("omogena");
    if (ui->liver_ecogenity->text().isEmpty())
        ui->liver_ecogenity->setText("medie");
    if (ui->liver_formations->text().isEmpty())
        ui->liver_formations->setText("lichidiene, solide abs.");
    if (ui->liver_duct_hepatic->text().isEmpty())
        ui->liver_duct_hepatic->setText("nu sunt dilatate");

    //--- cholecist
    if (ui->cholecist_form->text().isEmpty())
        ui->cholecist_form->setText("obisnuita");
    if (ui->cholecist_formations->text().isEmpty())
        ui->cholecist_formations->setText("abs.");

    //--- pancreas
    if (ui->pancreas_ecogenity->text().isEmpty())
        ui->pancreas_ecogenity->setText("sporita");
    if (ui->pancreas_parenchyma->text().isEmpty())
        ui->pancreas_parenchyma->setText("omogena");
    if (ui->pancreas_formations->text().isEmpty())
        ui->pancreas_formations->setText("lichidiene, solide abs.");

    //--- spleen
    if (ui->spleen_contour->text().isEmpty())
        ui->spleen_contour->setText("clar");
    if (ui->spleen_parenchyma->text().isEmpty())
        ui->spleen_parenchyma->setText("omogena");
    if (ui->spleen_formations->text().isEmpty())
        ui->spleen_formations->setText("lichidiene, solide abs.");

    //--- intestinalLoop
    if (ui->intestinalHandles->toPlainText().isEmpty())
        ui->intestinalHandles->setPlainText("formațiuni abs., ganglioni limfatici mezenteriali 5-10 mm fără aglomerări");
}

void DocReportEchoHandler::Impl::setDefault_UtinarySystem()
{
    // actualizam datele flag-lor si verificam daca ales flagul
    if (! (o.getSectionsSystem() & DocReportEcho::UrinarySystem))
        return;

    // determinam UI
    auto ui = o.uiPtr();

    //--- kidney
    if (ui->kidney_formations->text().isEmpty())
        ui->kidney_formations->setText("solide, lichide abs., sectoare hiperecogene 2-3 mm fără umbră acustică");
    if (ui->kidney_pielocaliceal_left->text().isEmpty())
        ui->kidney_pielocaliceal_left->setText("nu este dilatat");
    if (ui->kidney_pielocaliceal_right->text().isEmpty())
        ui->kidney_pielocaliceal_right->setText("nu este dilatat");
    if (ui->adrenalGlands->toPlainText().isEmpty())
        ui->adrenalGlands->setPlainText("nu sunt vizibile ecografic");

    //--- bladder
    if (ui->bladder_formations->text().isEmpty())
        ui->bladder_formations->setText("diverticuli, calculi abs.");
}

void DocReportEchoHandler::Impl::setDefault_Prostate()
{
    // actualizam datele flag-lor si verificam daca ales flagul
    if (! (o.getSectionsSystem() & DocReportEcho::Prostate))
        return;

    // determinam UI
    auto ui = o.uiPtr();

    if (ui->prostate_contur->text().isEmpty())
        ui->prostate_contur->setText("clar");
    if (ui->prostate_ecogency->text().isEmpty())
        ui->prostate_ecogency->setText("scăzută");
    if (ui->prostate_ecostructure->text().isEmpty())
        ui->prostate_ecostructure->setText("omogenă");
    if (ui->prostate_formations->text().isEmpty())
        ui->prostate_formations->setText("abs.");
    if (ui->prostate_recommendation->text().isEmpty())
        ui->prostate_recommendation->setText("consulta\310\233ia urologului");
}

void DocReportEchoHandler::Impl::setDefault_Gynecology()
{
    // actualizam datele flag-lor si verificam daca ales flagul
    if (! (o.getSectionsSystem() & DocReportEcho::Gynecology))
        return;

    // determinam UI
    auto ui = o.uiPtr();

    const int m_ID = o.getId();
    if (m_ID <= 0){
        ui->gynecology_btn_transvaginal->setChecked(true);
        ui->gynecology_dateMenstruation->setDate(QDate::currentDate());
    }
    if (ui->gynecology_antecedent->text().isEmpty())
        ui->gynecology_antecedent->setText("abs.");
    if (ui->gynecology_uterus_pozition->text().isEmpty())
        ui->gynecology_uterus_pozition->setText("anteflexie");
    if (ui->gynecology_uterus_ecostructure->text().isEmpty())
        ui->gynecology_uterus_ecostructure->setText("omogenă");
    if (ui->gynecology_uterus_formations->toPlainText().isEmpty())
        ui->gynecology_uterus_formations->setPlainText("abs.");
    if (ui->gynecology_ecou_ecostructure->text().isEmpty())
        ui->gynecology_ecou_ecostructure->setText("omogenă");
    if (ui->gynecology_cervix_ecostructure->text().isEmpty())
        ui->gynecology_cervix_ecostructure->setText("omogenă");
    if (ui->gynecology_douglas->text().isEmpty())
        ui->gynecology_douglas->setText("liber");
    if (ui->gynecology_plex_venos->text().isEmpty())
        ui->gynecology_plex_venos->setText("nu sunt dilatate");
    if (ui->gynecology_ovary_formations_right->toPlainText().isEmpty())
        ui->gynecology_ovary_formations_right->setPlainText("abs.");
    if (ui->gynecology_ovary_formations_left->toPlainText().isEmpty())
        ui->gynecology_ovary_formations_left->setPlainText("abs.");
    if (ui->gynecology_recommendation->text().isEmpty())
        ui->gynecology_recommendation->setText("consulta\310\233ia ginecologului");
}

void DocReportEchoHandler::Impl::setDefault_Breast()
{
    // actualizam datele flag-lor si verificam daca ales flagul
    if (! (o.getSectionsSystem() & DocReportEcho::Breast))
        return;

    // determinam UI
    auto ui = o.uiPtr();

    if (ui->breast_right_ecostructure->text().isEmpty())
        ui->breast_right_ecostructure->setText("glandulară, omogenă");
    if (ui->breast_right_duct->text().isEmpty())
        ui->breast_right_duct->setText("norm.");
    if (ui->breast_right_ligament->text().isEmpty())
        ui->breast_right_ligament->setText("norm");
    if (ui->breast_right_formations->toPlainText().isEmpty())
        ui->breast_right_formations->setPlainText("lichidiene, solide abs.");
    if (ui->breast_right_ganglions->text().isEmpty())
        ui->breast_right_ganglions->setText("fără modificări patologice");

    if (ui->breast_left_ecostructure->text().isEmpty())
        ui->breast_left_ecostructure->setText("glandulară, omogenă");
    if (ui->breast_left_duct->text().isEmpty())
        ui->breast_left_duct->setText("norm.");
    if (ui->breast_left_ligament->text().isEmpty())
        ui->breast_left_ligament->setText("norm");
    if (ui->breast_left_formations->toPlainText().isEmpty())
        ui->breast_left_formations->setPlainText("lichidiene, solide abs.");
    if (ui->breast_left_ganglions->text().isEmpty())
        ui->breast_left_ganglions->setText("fără modificări patologice");
}

void DocReportEchoHandler::Impl::setDefault_Thyroid()
{
    // actualizam datele flag-lor si verificam daca ales flagul
    if (! (o.getSectionsSystem() & DocReportEcho::Thyroid))
        return;

    // determinam UI
    auto ui = o.uiPtr();

    if (ui->thyroid_ecostructure->text().isEmpty())
        ui->thyroid_ecostructure->setText("omogenă");
    if (ui->thyroid_formations->toPlainText().isEmpty())
        ui->thyroid_formations->setPlainText("l.drept - forma\310\233iuni lichidiene, solide abs.\nl.st\303\242ng - forma\310\233iuni lichidiene, solide abs.");
    if (ui->thyroid_ganglions->text().isEmpty())
        ui->thyroid_ganglions->setText("fara modificări patologice");
    if (ui->thyroid_recommendation->text().isEmpty())
        ui->thyroid_recommendation->setText("consulta\310\233ia endocrinologului");
}

void DocReportEchoHandler::Impl::setDefault_Gestation0()
{
    // actualizam datele flag-lor si verificam daca ales flagul
    if (! (o.getSectionsSystem() & DocReportEcho::Gestation0))
        return;

    // determinam UI
    auto ui = o.uiPtr();

    ui->gestation0_LMP->setDate(QDate::currentDate());

    if (ui->gestation0_antecedent->text().isEmpty())
        ui->gestation0_antecedent->setText("abs.");
    if (ui->gestation0_BCF->text().isEmpty())
        ui->gestation0_BCF->setText("prezen\310\233i, ritmici");
    if (ui->gestation0_liquid_amniotic->text().isEmpty())
        ui->gestation0_liquid_amniotic->setText("omogen, transparent");
    if (ui->gestation0_miometer->text().isEmpty())
        ui->gestation0_miometer->setText("omogen; forma\310\233iuni solide, lichidiene abs.");
    if (ui->gestation0_cervix->text().isEmpty())
        ui->gestation0_cervix->setText("omogen; forma\310\233iuni solide, lichidiene abs.; \303\256nchis, lungimea 32,9 mm");
    if (ui->gestation0_ovary->text().isEmpty())
        ui->gestation0_ovary->setText("aspect ecografic normal");
    if (ui->gestation0_recommendation->text().isEmpty())
        ui->gestation0_recommendation->setText("consulta\310\233ia ginecologului, examen ecografic la 11-12 s\304\203pt\304\203m\303\242ni a sarcinei");
}

void DocReportEchoHandler::Impl::setDefault_Gestation1()
{
    // actualizam datele flag-lor si verificam daca ales flagul
    if (! (o.getSectionsSystem() & DocReportEcho::Gestation1))
        return;

    // determinam UI
    auto ui = o.uiPtr();

    if (ui->gestation1_antecedent->text().isEmpty())
        ui->gestation1_antecedent->setText("abs.");
    if (ui->gestation1_BCF->text().isEmpty())
        ui->gestation1_BCF->setText("prezen\310\233i, ritmici");
    if (ui->gestation1_callote_cranium->text().isEmpty())
        ui->gestation1_callote_cranium->setText("norm.");
    if (ui->gestation1_plex_choroid->text().isEmpty())
        ui->gestation1_plex_choroid->setText("norm.");
    if (ui->gestation1_vertebral_column->text().isEmpty())
        ui->gestation1_vertebral_column->setText("integră");
    if (ui->gestation1_stomach->text().isEmpty())
        ui->gestation1_stomach->setText("norm.");
    if (ui->gestation1_bladder->text().isEmpty())
        ui->gestation1_bladder->setText("norm.");
    if (ui->gestation1_diaphragm->text().isEmpty())
        ui->gestation1_diaphragm->setText("norm.");
    if (ui->gestation1_abdominal_wall->text().isEmpty())
        ui->gestation1_abdominal_wall->setText("integru");
    if (ui->gestation1_location_placenta->text().isEmpty())
        ui->gestation1_location_placenta->setText("peretele anterior");
    if (ui->gestation1_amniotic_liquid->text().isEmpty())
        ui->gestation1_amniotic_liquid->setText("omogen, transparent");
    if (ui->gestation1_miometer->text().isEmpty())
        ui->gestation1_miometer->setText("omogen; forma\310\233iuni solide, lichidiene abs.");
    if (ui->gestation1_cervix->text().isEmpty())
        ui->gestation1_cervix->setText("omogen; forma\310\233iuni solide, lichidiene abs.; \303\256nchis, lungimea 32,9 mm");
    if (ui->gestation1_ovary->text().isEmpty())
        ui->gestation1_ovary->setText("aspect ecografic normal");
    if (ui->gestation1_recommendation->text().isEmpty())
        ui->gestation1_recommendation->setText("consulta\310\233ia ginecologului, examen ecografic la 18-20 s\304\203pt\304\203m\303\242ni a sarcinei");
}

void DocReportEchoHandler::Impl::setDefault_Gestation2()
{
    // actualizam datele flag-lor si verificam daca ales flagul
    if (! (o.getSectionsSystem() & DocReportEcho::Gestation2))
        return;

    // determinam UI
    auto ui = o.uiPtr();

    if (ui->gestation2_recommendation->text().isEmpty())
        ui->gestation2_recommendation->setText("consulta\310\233ia ginecologului");
}

