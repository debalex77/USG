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

void DocReportEchoHandler::Impl::setPropertyMaxLengthText()
{
    // actualizam datele flag-lor
    DocReportEcho::SectionsSystem s = o.getSectionsSystem();

    // determinam UI
    auto ui = o.uiPtr();

    // organs internal
    if (s & DocReportEcho::OrgansInternal) {
        //--- liver
        ui->liver_left->setMaxLength(5);
        ui->liver_right->setMaxLength(5);
        ui->liver_contour->setMaxLength(20);
        ui->liver_parenchyma->setMaxLength(20);
        ui->liver_ecogenity->setMaxLength(30);
        ui->liver_formations->setMaxLength(300);
        ui->liver_duct_hepatic->setMaxLength(50);
        ui->liver_porta->setMaxLength(5);
        ui->liver_lienalis->setMaxLength(5);

        //--- cholecist
        ui->cholecist_dimens->setMaxLength(15);
        ui->cholecist_form->setMaxLength(150);
        ui->cholecist_formations->setMaxLength(300);
        ui->cholecist_walls->setMaxLength(5);
        ui->cholecist_coledoc->setMaxLength(5);

        //--- pancreas
        ui->pancreas_cefal->setMaxLength(5);
        ui->pancreas_corp->setMaxLength(5);
        ui->pancreas_tail->setMaxLength(5);
        ui->pancreas_parenchyma->setMaxLength(20);
        ui->pancreas_ecogenity->setMaxLength(30);
        ui->pancreas_formations->setMaxLength(300);

        //--- spleen
        ui->spleen_dimens->setMaxLength(15);
        ui->spleen_contour->setMaxLength(20);
        ui->spleen_parenchyma->setMaxLength(30);
        ui->spleen_formations->setMaxLength(300);

        //--- intestinal loop
        ui->intestinalHandles->setPlaceholderText(QObject::tr("...maximum 300 caractere"));

        //--- concluzion
        ui->organsInternal_concluzion->setPlaceholderText(QObject::tr("... maximum 500 caractere"));

        //--- recommandation
        ui->organsInternal_recommendation->setPlaceholderText(QObject::tr("...maximum 255 caractere"));
        ui->organsInternal_recommendation->setMaxLength(255);
    }

    // urinary system
    if (s & DocReportEcho::UrinarySystem) {
        //--- kidney
        ui->kidney_right->setMaxLength(15);
        ui->kidney_left->setMaxLength(15);
        ui->kidney_corticomed_left->setMaxLength(5);
        ui->kidney_corticomed_right->setMaxLength(5);
        ui->kidney_pielocaliceal_left->setMaxLength(30);
        ui->kidney_pielocaliceal_right->setMaxLength(30);
        ui->kidney_formations->setMaxLength(500);

        //--- adrenal glands
        ui->adrenalGlands->setPlaceholderText(QObject::tr("... maximum 500 caractere"));

        //--- bladder
        ui->bladder_volum->setMaxLength(5);
        ui->bladder_walls->setMaxLength(5);
        ui->bladder_formations->setMaxLength(300);

        //--- concluzion
        ui->urinary_system_concluzion->setPlaceholderText(QObject::tr("... maximum 500 caractere"));

        //--- recommandation
        ui->urinary_system_recommendation->setPlaceholderText(QObject::tr("...maximum 255 caractere"));
        ui->urinary_system_recommendation->setMaxLength(255);
    }

    // prostate
    if (s & DocReportEcho::Prostate) {
        //--- items
        ui->prostate_dimens->setMaxLength(25);
        ui->prostate_volum->setMaxLength(5);
        ui->prostate_ecostructure->setMaxLength(30);
        ui->prostate_contur->setMaxLength(20);
        ui->prostate_ecogency->setMaxLength(30);
        ui->prostate_formations->setMaxLength(300);

        //--- concluzion
        ui->prostate_concluzion->setPlaceholderText(QObject::tr("... maximum 500 caractere"));

        //--- recommandation
        ui->prostate_recommendation->setPlaceholderText(QObject::tr("... maximum 255 caractere"));
        ui->prostate_recommendation->setMaxLength(255);
    }

    // gynecology
    if (s &DocReportEcho::Gynecology) {
        //--- items
        ui->gynecology_antecedent->setMaxLength(150);
        ui->gynecology_uterus_dimens->setMaxLength(25);
        ui->gynecology_uterus_pozition->setMaxLength(30);
        ui->gynecology_uterus_ecostructure->setMaxLength(30);
        ui->gynecology_uterus_formations->setPlaceholderText(QObject::tr("... maximum 500 caractere"));
        ui->gynecology_jonctional_zone_description->setMaxLength(256);
        ui->gynecology_jonctional_zone_description->setPlaceholderText(QObject::tr("... maximum 256 caractere"));
        ui->gynecology_canal_cervical_formations->setMaxLength(256);
        ui->gynecology_canal_cervical_formations->setPlaceholderText(QObject::tr("... maximum 256 caractere"));
        ui->gynecology_ecou_dimens->setMaxLength(5);
        ui->gynecology_ecou_ecostructure->setMaxLength(100);
        ui->gynecology_douglas->setMaxLength(100);
        ui->gynecology_plex_venos->setMaxLength(150);
        ui->gynecology_ovary_left_dimens->setMaxLength(25);
        ui->gynecology_ovary_right_dimens->setMaxLength(25);
        ui->gynecology_ovary_left_volum->setMaxLength(5);
        ui->gynecology_ovary_right_volum->setMaxLength(5);
        ui->gynecology_follicule_left->setMaxLength(100);
        ui->gynecology_follicule_right->setMaxLength(100);
        ui->gynecology_fallopian_tubes_formations->setMaxLength(256);
        ui->gynecology_fallopian_tubes_formations->setPlaceholderText(QObject::tr("... maximum 256 caractere"));
        ui->gynecology_ovary_formations_left->setPlaceholderText(QObject::tr("... maximum 300 caractere"));
        ui->gynecology_ovary_formations_right->setPlaceholderText(QObject::tr("... maximum 300 caractere"));

        //--- concluzion
        ui->gynecology_concluzion->setPlaceholderText(QObject::tr("... maximum 500 caractere"));

        //--- recommandation
        ui->gynecology_recommendation->setPlaceholderText(QObject::tr("... maximum 255 caractere"));
        ui->gynecology_recommendation->setMaxLength(255);
    }

    // breast
    if (s & DocReportEcho::Breast) {
        //--- left
        ui->breast_left_ecostructure->setMaxLength(255);
        ui->breast_left_duct->setMaxLength(20);
        ui->breast_left_ligament->setMaxLength(20);
        ui->breast_left_formations->setPlaceholderText(QObject::tr("... maximum 500 caractere"));
        ui->breast_left_ganglions->setMaxLength(300);

        //--- right
        ui->breast_right_ecostructure->setMaxLength(255);
        ui->breast_right_duct->setMaxLength(20);
        ui->breast_right_ligament->setMaxLength(20);
        ui->breast_right_formations->setPlaceholderText(QObject::tr("... maximum 500 caractere"));
        ui->breast_right_ganglions->setMaxLength(300);

        //--- concluzion
        ui->breast_concluzion->setPlaceholderText(QObject::tr("... maximum 500 caractere"));

        //--- recommandation
        ui->breast_recommendation->setPlaceholderText(QObject::tr("... maximum 255 caractere"));
        ui->breast_recommendation->setMaxLength(255);
    }

    // thyroid
    if (s & DocReportEcho::Thyroid) {
        //--- items
        ui->thyroid_left_dimens->setMaxLength(20);
        ui->thyroid_left_volum->setMaxLength(5);
        ui->thyroid_right_dimens->setMaxLength(20);
        ui->thyroid_right_volum->setMaxLength(5);
        ui->thyroid_istm->setMaxLength(4);
        ui->thyroid_ecostructure->setMaxLength(20);
        ui->thyroid_formations->setPlaceholderText(QObject::tr("... maximum 500 caractere"));
        ui->thyroid_ganglions->setPlaceholderText(QObject::tr("... maximum 300 caractere"));
        ui->thyroid_ganglions->setMaxLength(300);

        //--- concluzion
        ui->thyroid_concluzion->setPlaceholderText(QObject::tr("... maximum 500 caractere"));

        //--- recommandation
        ui->thyroid_recommendation->setPlaceholderText(QObject::tr("... maximum 255 caractere"));
        ui->thyroid_recommendation->setMaxLength(255);
    }

    // gestation0
    if (s & DocReportEcho::Gestation0) {
        //--- items
        ui->gestation0_gestation->setInputMask("99s. 9z.");
        ui->gestation0_GS_age->setInputMask("99s. 9z.");
        ui->gestation0_CRL_age->setInputMask("99s. 9z.");

        ui->gestation0_antecedent->setMaxLength(150);
        ui->gestation0_gestation->setMaxLength(20);
        ui->gestation0_GS_dimens->setMaxLength(5);
        ui->gestation0_GS_age->setMaxLength(20);
        ui->gestation0_CRL_dimens->setMaxLength(5);
        ui->gestation0_CRL_age->setMaxLength(20);
        ui->gestation0_BCF->setMaxLength(30);
        ui->gestation0_liquid_amniotic->setMaxLength(40);
        ui->gestation0_miometer->setMaxLength(200);
        ui->gestation0_cervix->setMaxLength(200);
        ui->gestation0_ovary->setMaxLength(200);

        //--- concluzion
        ui->gestation0_concluzion->setPlaceholderText(QObject::tr("... maximum 500 caractere"));

        //--- recommandation
        ui->gestation0_recommendation->setPlaceholderText(QObject::tr("... maximum 255 caractere"));
        ui->gestation0_recommendation->setMaxLength(255);
    }

    // gestation1
    if (s & DocReportEcho::Gestation1) {
        //--- items
        ui->gestation1_gestation->setInputMask("99s. 9z.");
        ui->gestation1_CRL_age->setInputMask("99s. 9z.");
        ui->gestation1_BPD_age->setInputMask("99s. 9z.");
        ui->gestation1_FL_age->setInputMask("99s. 9z.");

        ui->gestation1_antecedent->setMaxLength(150);
        ui->gestation1_gestation->setMaxLength(20);
        ui->gestation1_CRL_dimens->setMaxLength(5);
        ui->gestation1_CRL_age->setMaxLength(20);
        ui->gestation1_BPD_dimens->setMaxLength(5);
        ui->gestation1_BPD_age->setMaxLength(20);
        ui->gestation1_NT_dimens->setMaxLength(5);
        ui->gestation1_NT_percent->setMaxLength(20);
        ui->gestation1_BN_dimens->setMaxLength(5);
        ui->gestation1_BN_percent->setMaxLength(20);
        ui->gestation1_BCF->setMaxLength(30);
        ui->gestation1_FL_dimens->setMaxLength(5);
        ui->gestation1_FL_age->setMaxLength(20);
        ui->gestation1_callote_cranium->setMaxLength(50);
        ui->gestation1_plex_choroid->setMaxLength(50);
        ui->gestation1_vertebral_column->setMaxLength(50);
        ui->gestation1_stomach->setMaxLength(50);
        ui->gestation1_diaphragm->setMaxLength(50);
        ui->gestation1_bladder->setMaxLength(50);
        ui->gestation1_abdominal_wall->setMaxLength(50);
        ui->gestation1_location_placenta->setMaxLength(50);
        ui->gestation1_sac_vitelin->setMaxLength(50);
        ui->gestation1_amniotic_liquid->setMaxLength(40);
        ui->gestation1_miometer->setMaxLength(200);
        ui->gestation1_cervix->setMaxLength(200);
        ui->gestation1_ovary->setMaxLength(200);
        ui->gestation1_sac_vitelin->setMaxLength(50);
        ui->gestation1_sac_vitelin->setPlaceholderText(QObject::tr("... maximum 50 caractere"));

        //--- concluzion
        ui->gestation1_concluzion->setPlaceholderText(QObject::tr("... maximum 500 caractere"));

        //--- recommandation
        ui->gestation1_recommendation->setPlaceholderText(QObject::tr("... maximum 255 caractere"));
        ui->gestation1_recommendation->setMaxLength(255);
    }

    // gestation2
    if (s & DocReportEcho::Gestation2) {
        //--- items
        ui->gestation2_gestation_age->setMaxLength(20);
        ui->gestation2_gestation_age->setPlaceholderText(QObject::tr("... maximum 20 caractere"));
        ui->gestation2_pregnancy_description->setMaxLength(250);
        ui->gestation2_pregnancy_description->setPlaceholderText(QObject::tr("... maximum 250 caractere"));
        ui->gestation2_eyeball_desciption->setMaxLength(100);
        ui->gestation2_eyeball_desciption->setPlaceholderText(QObject::tr("... maximum 100 caractere"));
        ui->gestation2_nasolabialTriangle_description->setMaxLength(100);
        ui->gestation2_nasolabialTriangle_description->setPlaceholderText(QObject::tr("... maximum 100 caractere"));
        ui->gestation2_ventricularSystem_description->setMaxLength(70);
        ui->gestation2_ventricularSystem_description->setPlaceholderText(QObject::tr("... maximum 70 caractere"));
        ui->gestation2_choroidalPlex_description->setMaxLength(70);
        ui->gestation2_choroidalPlex_description->setPlaceholderText(QObject::tr("... maximum 70 caractere"));
        ui->gestation2_cerebellum_description->setMaxLength(70);
        ui->gestation2_cerebellum_description->setPlaceholderText(QObject::tr("... maximum 70 caractere"));
        ui->gestation2_vertebralColumn_description->setMaxLength(100);
        ui->gestation2_vertebralColumn_description->setPlaceholderText(QObject::tr("... maximum 100 caractere"));
        ui->gestation2_heartPosition->setMaxLength(50);
        ui->gestation2_heartPosition->setPlaceholderText(QObject::tr("... maximum 50 caractere"));
        ui->gestation2_planPatruCamere_description->setMaxLength(70);
        ui->gestation2_planPatruCamere_description->setPlaceholderText(QObject::tr("... maximum 70 caractere"));
        ui->gestation2_ventricularEjectionPathLeft_description->setMaxLength(70);
        ui->gestation2_ventricularEjectionPathLeft_description->setPlaceholderText(QObject::tr("... maximum 70 caractere"));
        ui->gestation2_ventricularEjectionPathRight_description->setMaxLength(70);
        ui->gestation2_ventricularEjectionPathRight_description->setPlaceholderText(QObject::tr("... maximum 70 caractere"));
        ui->gestation2_intersectionVesselMagistral_description->setMaxLength(70);
        ui->gestation2_intersectionVesselMagistral_description->setPlaceholderText(QObject::tr("... maximum 70 caractere"));
        ui->gestation2_planTreiVase_description->setMaxLength(70);
        ui->gestation2_planTreiVase_description->setPlaceholderText(QObject::tr("... maximum 70 caractere"));
        ui->gestation2_pulmonaryAreas_description->setMaxLength(70);
        ui->gestation2_pulmonaryAreas_description->setPlaceholderText(QObject::tr("... maximum 70 caractere"));
        ui->gestation2_diaphragm_description->setMaxLength(70);
        ui->gestation2_diaphragm_description->setPlaceholderText(QObject::tr("... maximum 70 caractere"));
        ui->gestation2_stomach_description->setMaxLength(50);
        ui->gestation2_stomach_description->setPlaceholderText(QObject::tr("... maximum 50 caractere"));
        ui->gestation2_cholecist_description->setMaxLength(50);
        ui->gestation2_cholecist_description->setPlaceholderText(QObject::tr("... maximum 50 caractere"));
        ui->gestation2_intestine_description->setMaxLength(70);
        ui->gestation2_intestine_description->setPlaceholderText(QObject::tr("... maximum 70 caractere"));
        ui->gestation2_kidneys_description->setMaxLength(70);
        ui->gestation2_kidneys_description->setPlaceholderText(QObject::tr("... maximum 70 caractere"));
        ui->gestation2_ureter_description->setMaxLength(70);
        ui->gestation2_ureter_description->setPlaceholderText(QObject::tr("... maximum 70 caractere"));
        ui->gestation2_extremities_description->setMaxLength(150);
        ui->gestation2_extremities_description->setPlaceholderText(QObject::tr("... maximum 150 caractere"));
        ui->gestation2_placenta_localization->setMaxLength(50);
        ui->gestation2_placenta_localization->setPlaceholderText(QObject::tr("... maximum 50 caractere"));
        ui->gestation2_placentaStructure_description->setMaxLength(150);
        ui->gestation2_placentaStructure_description->setPlaceholderText(QObject::tr("... maximum 150 caractere"));
        ui->gestation2_umbilicalCordon_description->setMaxLength(70);
        ui->gestation2_umbilicalCordon_description->setPlaceholderText(QObject::tr("... maximum 70 caractere"));
        ui->gestation2_cervix_description->setMaxLength(150);
        ui->gestation2_cervix_description->setPlaceholderText(QObject::tr("... maximum 150 caractere"));
        ui->gestation2_comment->setPlaceholderText(QObject::tr("... maximum 250 caractere"));
        ui->gestation2_gestation_age->setInputMask("99s. 9z.");
        ui->gestation2_bpd_age->setInputMask("99s. 9z.");
        ui->gestation2_hc_age->setInputMask("99s. 9z.");
        ui->gestation2_ac_age->setInputMask("99s. 9z.");
        ui->gestation2_fl_age->setInputMask("99s. 9z.");
        ui->gestation2_fetus_age->setInputMask("99s. 9z.");

        //--- concluzion
        ui->gestation2_concluzion->setPlaceholderText(QObject::tr("... maximum 500 caractere"));

        //--- recommandation
        ui->gestation2_recommendation->setPlaceholderText(QObject::tr("... maximum 255 caractere"));
        ui->gestation2_recommendation->setMaxLength(255);
    }

    // lymph nodes
    if (s & DocReportEcho::LymphNodes) {
        //--- items
        ui->ln_zoneArea->setMaxLength(50);
        ui->ln_zoneArea->setPlaceholderText(QObject::tr("reg.cervicală, axilară, inghinală ... max.50 caractere"));
        //--- tissues
        ui->ln_indications->setMaxLength(100);
        ui->ln_indications->setPlaceholderText(QObject::tr("tumefacție, durere, susp. de adenopatie, control postoper. ... max.100 caractere"));
        ui->ln_localizationTissue->setMaxLength(100);
        ui->ln_localizationTissue->setPlaceholderText(QObject::tr("... max. 100 caractere"));
        ui->ln_sizeTissue->setMaxLength(50);
        ui->ln_sizeTissue->setPlaceholderText(QObject::tr("... max.50 caractere"));
        ui->ln_otherChangeTissue->setPlaceholderText(QObject::tr("colecții, hematom, abces, lipom etc. ... maximum 250 caractere"));
        //--- nodes
        ui->ln_sizeNode->setMaxLength(50);
        ui->ln_sizeNode->setPlaceholderText(QObject::tr("... max.50 caractere"));
        ui->ln_otherChangeNode->setPlaceholderText(QObject::tr("necroză, calcificări, adenopatie suspectă ... max.250 carcatere"));

        //--- concluzion
        ui->ln_conluzion->setPlaceholderText(QObject::tr("... maximum 500 caractere"));

        //--- recommandation
        ui->ln_recommand->setMaxLength(250);
        ui->ln_recommand->setPlaceholderText(QObject::tr("... maximum 250 caractere"));
    }
}
