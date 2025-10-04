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

void DocReportEchoHandler::Impl::enableSystemConnections(bool enableConnection)
{
    // actualizam datele flag-lor
    DocReportEcho::SectionsSystem s = o.getSectionsSystem();

    // determinam UI
    auto ui = o.uiPtr();

    if (s & DocReportEcho::OrgansInternal) {

        if (enableConnection) {

            QList<QLineEdit*> list = ui->stackedWidget->widget(DocReportEcho::PageReport::page_organs_internal)->findChildren<QLineEdit*>();
            for (int n = 0; n < list.count(); n++) {
                QObject::connect(list[n], &QLineEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }

            QList<QTextEdit*> list_text_edit = ui->stackedWidget->widget(o.page_organs_internal)->findChildren<QTextEdit*>();
            for (int n = 0; n < list_text_edit.count(); n++) {
                QObject::connect(list_text_edit[n], &QTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }

            QList<QPlainTextEdit*> list_plain_text = ui->stackedWidget->widget(DocReportEcho::PageReport::page_organs_internal)->findChildren<QPlainTextEdit*>();
            for (int n = 0; n < list_plain_text.count(); n++) {
                QObject::connect(list_plain_text[n], &QPlainTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }

            ui->intestinalHandles->setProperty("maxLen", 300);
            QObject::connect(ui->intestinalHandles, &QPlainTextEdit::textChanged,
                             &o, &DocReportEcho::enforceMaxForSender, Qt::UniqueConnection);

            ui->organsInternal_concluzion->setProperty("maxLen", 500);
            QObject::connect(ui->organsInternal_concluzion, &QPlainTextEdit::textChanged,
                             &o, &DocReportEcho::enforceMaxForSender, Qt::UniqueConnection);

        } else {

            QList<QLineEdit*> list = ui->stackedWidget->widget(DocReportEcho::PageReport::page_organs_internal)->findChildren<QLineEdit*>();
            for (int n = 0; n < list.count(); n++) {
                QObject::disconnect(list[n], &QLineEdit::textChanged,
                                 &o, &DocReportEcho::markModified);
            }

            QList<QTextEdit*> list_text_edit = ui->stackedWidget->widget(o.page_organs_internal)->findChildren<QTextEdit*>();
            for (int n = 0; n < list_text_edit.count(); n++) {
                QObject::disconnect(list_text_edit[n], &QTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified);
            }

            QList<QPlainTextEdit*> list_plain_text = ui->stackedWidget->widget(DocReportEcho::PageReport::page_organs_internal)->findChildren<QPlainTextEdit*>();
            for (int n = 0; n < list_plain_text.count(); n++) {
                QObject::disconnect(list_plain_text[n], &QPlainTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified);
            }
        }
    }

    if (s & DocReportEcho::UrinarySystem) {

        if (enableConnection) {

            QList<QTextEdit*> list_text_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_urinary_system)->findChildren<QTextEdit*>();
            for (int n = 0; n < list_text_edit.count(); n++) {
                QObject::connect(list_text_edit[n], &QTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }

            QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_urinary_system)->findChildren<QLineEdit*>();
            for (int n = 0; n < list_line_edit.count(); n++) {
                QObject::connect(list_line_edit[n], &QLineEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }

            QList<QPlainTextEdit*> list_plain_text = ui->stackedWidget->widget(DocReportEcho::PageReport::page_urinary_system)->findChildren<QPlainTextEdit*>();
            for (int n = 0; n < list_plain_text.count(); n++) {
                QObject::connect(list_plain_text[n], &QPlainTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }

            ui->adrenalGlands->setProperty("maxLen", 500);
            QObject::connect(ui->adrenalGlands, &QPlainTextEdit::textChanged,
                             &o, &DocReportEcho::enforceMaxForSender, Qt::UniqueConnection);

            ui->urinary_system_concluzion->setProperty("maxLen", 500);
            QObject::connect(ui->urinary_system_concluzion, &QPlainTextEdit::textChanged,
                             &o, &DocReportEcho::enforceMaxForSender, Qt::UniqueConnection);

        } else {

            QList<QTextEdit*> list_text_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_urinary_system)->findChildren<QTextEdit*>();
            for (int n = 0; n < list_text_edit.count(); n++) {
                QObject::disconnect(list_text_edit[n], &QTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified);
            }

            QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_urinary_system)->findChildren<QLineEdit*>();
            for (int n = 0; n < list_line_edit.count(); n++) {
                QObject::disconnect(list_line_edit[n], &QLineEdit::textChanged,
                                 &o, &DocReportEcho::markModified);
            }

            QList<QPlainTextEdit*> list_plain_text = ui->stackedWidget->widget(DocReportEcho::PageReport::page_urinary_system)->findChildren<QPlainTextEdit*>();
            for (int n = 0; n < list_plain_text.count(); n++) {
                QObject::disconnect(list_plain_text[n], &QPlainTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified);
            }

        }
    }

    if (s & DocReportEcho::Prostate) {

        if (enableConnection) {

            QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_prostate)->findChildren<QLineEdit*>();
            for (int n = 0; n < list_line_edit.count(); n++) {
                QObject::connect(list_line_edit[n], &QLineEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }
            QList<QPlainTextEdit*> list_plain_text = ui->stackedWidget->widget(DocReportEcho::PageReport::page_prostate)->findChildren<QPlainTextEdit*>();
            for (int n = 0; n < list_plain_text.count(); n++) {
                QObject::connect(list_plain_text[n], &QPlainTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }

            ui->prostate_concluzion->setProperty("maxLen", 500);
            QObject::connect(ui->prostate_concluzion, &QPlainTextEdit::textChanged,
                             &o, &DocReportEcho::enforceMaxForSender, Qt::UniqueConnection);

        } else {

            QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_prostate)->findChildren<QLineEdit*>();
            for (int n = 0; n < list_line_edit.count(); n++) {
                QObject::disconnect(list_line_edit[n], &QLineEdit::textChanged,
                                 &o, &DocReportEcho::markModified);
            }
            QList<QPlainTextEdit*> list_plain_text = ui->stackedWidget->widget(DocReportEcho::PageReport::page_prostate)->findChildren<QPlainTextEdit*>();
            for (int n = 0; n < list_plain_text.count(); n++) {
                QObject::disconnect(list_plain_text[n], &QPlainTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified);
            }

        }
    }

    if (s & DocReportEcho::Gynecology) {

        if (enableConnection) {

            QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gynecology)->findChildren<QLineEdit*>();
            for (int n = 0; n < list_line_edit.count(); n++) {
                QObject::connect(list_line_edit[n], &QLineEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }
            QList<QPlainTextEdit*> list_plain_text = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gynecology)->findChildren<QPlainTextEdit*>();
            for (int n = 0; n < list_plain_text.count(); n++) {
                QObject::connect(list_plain_text[n], &QPlainTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }

            QObject::connect(ui->gynecology_btn_transabdom, &QRadioButton::clicked, &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            QObject::connect(ui->gynecology_btn_transvaginal, &QRadioButton::clicked, &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            QObject::connect(ui->gynecology_dateMenstruation, &QDateEdit::dateChanged, &o, &DocReportEcho::markModified, Qt::UniqueConnection);

            ui->gynecology_uterus_formations->setProperty("maxLen", 500);
            QObject::connect(ui->gynecology_uterus_formations, &QPlainTextEdit::textChanged,
                             &o, &DocReportEcho::enforceMaxForSender, Qt::UniqueConnection);

            ui->gynecology_ovary_formations_left->setProperty("maxLen", 300);
            QObject::connect(ui->gynecology_ovary_formations_left, &QPlainTextEdit::textChanged,
                             &o, &DocReportEcho::enforceMaxForSender, Qt::UniqueConnection);

            ui->gynecology_ovary_formations_right->setProperty("maxLen", 300);
            QObject::connect(ui->gynecology_ovary_formations_right, &QPlainTextEdit::textChanged,
                             &o, &DocReportEcho::enforceMaxForSender, Qt::UniqueConnection);

            ui->gynecology_concluzion->setProperty("maxLen", 500);
            QObject::connect(ui->gynecology_concluzion, &QPlainTextEdit::textChanged,
                             &o, &DocReportEcho::enforceMaxForSender, Qt::UniqueConnection);

        } else {

            QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gynecology)->findChildren<QLineEdit*>();
            for (int n = 0; n < list_line_edit.count(); n++) {
                QObject::disconnect(list_line_edit[n], &QLineEdit::textChanged,
                                 &o, &DocReportEcho::markModified);
            }
            QList<QPlainTextEdit*> list_plain_text = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gynecology)->findChildren<QPlainTextEdit*>();
            for (int n = 0; n < list_plain_text.count(); n++) {
                QObject::disconnect(list_plain_text[n], &QPlainTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified);
            }

            QObject::disconnect(ui->gynecology_btn_transabdom, &QRadioButton::clicked, &o, &DocReportEcho::markModified);
            QObject::disconnect(ui->gynecology_btn_transvaginal, &QRadioButton::clicked, &o, &DocReportEcho::markModified);
            QObject::disconnect(ui->gynecology_dateMenstruation, &QDateEdit::dateChanged, &o, &DocReportEcho::markModified);

        }
    }

    if (s & DocReportEcho::Breast) {

        if (enableConnection) {

            QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_breast)->findChildren<QLineEdit*>();
            for (int n = 0; n < list_line_edit.count(); n++) {
                QObject::connect(list_line_edit[n], &QLineEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }
            QList<QPlainTextEdit*> list_plain_text = ui->stackedWidget->widget(DocReportEcho::PageReport::page_breast)->findChildren<QPlainTextEdit*>();
            for (int n = 0; n < list_plain_text.count(); n++) {
                QObject::connect(list_plain_text[n], &QPlainTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }

            ui->breast_left_formations->setProperty("maxLen", 500);
            QObject::connect(ui->breast_left_formations, &QPlainTextEdit::textChanged,
                             &o, &DocReportEcho::enforceMaxForSender, Qt::UniqueConnection);

            ui->breast_right_formations->setProperty("maxLen", 500);
            QObject::connect(ui->breast_right_formations, &QPlainTextEdit::textChanged,
                             &o, &DocReportEcho::enforceMaxForSender, Qt::UniqueConnection);

            ui->breast_concluzion->setProperty("maxLen", 500);
            QObject::connect(ui->breast_concluzion, &QPlainTextEdit::textChanged,
                             &o, &DocReportEcho::enforceMaxForSender, Qt::UniqueConnection);

        } else {

            QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_breast)->findChildren<QLineEdit*>();
            for (int n = 0; n < list_line_edit.count(); n++) {
                QObject::disconnect(list_line_edit[n], &QLineEdit::textChanged,
                                 &o, &DocReportEcho::markModified);
            }
            QList<QPlainTextEdit*> list_plain_text = ui->stackedWidget->widget(DocReportEcho::PageReport::page_breast)->findChildren<QPlainTextEdit*>();
            for (int n = 0; n < list_plain_text.count(); n++) {
                QObject::disconnect(list_plain_text[n], &QPlainTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified);
            }

        }
    }

    if (s & DocReportEcho::Thyroid) {

        if (enableConnection) {

            QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_thyroid)->findChildren<QLineEdit*>();
            for (int n = 0; n < list_line_edit.count(); n++) {
                QObject::connect(list_line_edit[n], &QLineEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }
            QList<QPlainTextEdit*> list_plain_text_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_thyroid)->findChildren<QPlainTextEdit*>();
            for (int n = 0; n < list_plain_text_edit.count(); n++) {
                QObject::connect(list_plain_text_edit[n], &QPlainTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }

            ui->thyroid_formations->setProperty("maxLen", 500);
            QObject::connect(ui->thyroid_formations, &QPlainTextEdit::textChanged,
                             &o, &DocReportEcho::enforceMaxForSender, Qt::UniqueConnection);

            ui->thyroid_concluzion->setProperty("maxLen", 500);
            QObject::connect(ui->thyroid_concluzion, &QPlainTextEdit::textChanged,
                             &o, &DocReportEcho::enforceMaxForSender, Qt::UniqueConnection);

        } else {

            QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_thyroid)->findChildren<QLineEdit*>();
            for (int n = 0; n < list_line_edit.count(); n++) {
                QObject::disconnect(list_line_edit[n], &QLineEdit::textChanged,
                                 &o, &DocReportEcho::markModified);
            }
            QList<QPlainTextEdit*> list_plain_text_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_thyroid)->findChildren<QPlainTextEdit*>();
            for (int n = 0; n < list_plain_text_edit.count(); n++) {
                QObject::disconnect(list_plain_text_edit[n], &QPlainTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified);
            }

        }

    }

    if (s & DocReportEcho::Gestation0) {

        if (enableConnection) {

            QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gestation0)->findChildren<QLineEdit*>();
            for (int n = 0; n < list_line_edit.count(); n++) {
                QObject::connect(list_line_edit[n], &QLineEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }
            QList<QPlainTextEdit*> list_plain_text_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gestation0)->findChildren<QPlainTextEdit*>();
            for (int n = 0; n < list_plain_text_edit.count(); n++) {
                QObject::connect(list_plain_text_edit[n], &QPlainTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }

            QObject::connect(ui->gestation0_LMP, &QDateEdit::dateChanged, &o, &DocReportEcho::markModified, Qt::UniqueConnection);

            ui->gestation0_concluzion->setProperty("maxLen", 500);
            QObject::connect(ui->gestation0_concluzion, &QPlainTextEdit::textChanged,
                             &o, &DocReportEcho::enforceMaxForSender, Qt::UniqueConnection);

        } else {

            QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gestation0)->findChildren<QLineEdit*>();
            for (int n = 0; n < list_line_edit.count(); n++) {
                QObject::disconnect(list_line_edit[n], &QLineEdit::textChanged,
                                 &o, &DocReportEcho::markModified);
            }
            QList<QPlainTextEdit*> list_plain_text_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gestation0)->findChildren<QPlainTextEdit*>();
            for (int n = 0; n < list_plain_text_edit.count(); n++) {
                QObject::disconnect(list_plain_text_edit[n], &QPlainTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified);
            }

            QObject::disconnect(ui->gestation0_LMP, &QDateEdit::dateChanged, &o, &DocReportEcho::markModified);
        }

    }

    if (s & DocReportEcho::Gestation1) {

        if (enableConnection) {

            QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gestation1)->findChildren<QLineEdit*>();
            for (int n = 0; n < list_line_edit.count(); n++) {
                QObject::connect(list_line_edit[n], &QLineEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }
            QList<QPlainTextEdit*> list_plain_text_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gestation1)->findChildren<QPlainTextEdit*>();
            for (int n = 0; n < list_plain_text_edit.count(); n++) {
                QObject::connect(list_plain_text_edit[n], &QPlainTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }

            QObject::connect(ui->gestation1_LMP, &QDateEdit::dateChanged, &o, &DocReportEcho::markModified, Qt::UniqueConnection);

            ui->gestation1_concluzion->setProperty("maxLen", 500);
            QObject::connect(ui->gestation1_concluzion, &QPlainTextEdit::textChanged,
                             &o, &DocReportEcho::enforceMaxForSender, Qt::UniqueConnection);

        } else {

            QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gestation1)->findChildren<QLineEdit*>();
            for (int n = 0; n < list_line_edit.count(); n++) {
                QObject::disconnect(list_line_edit[n], &QLineEdit::textChanged,
                                    &o, &DocReportEcho::markModified);
            }
            QList<QPlainTextEdit*> list_plain_text_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gestation1)->findChildren<QPlainTextEdit*>();
            for (int n = 0; n < list_plain_text_edit.count(); n++) {
                QObject::disconnect(list_plain_text_edit[n], &QPlainTextEdit::textChanged,
                                    &o, &DocReportEcho::markModified);
            }

            QObject::disconnect(ui->gestation1_LMP, &QDateEdit::dateChanged, &o, &DocReportEcho::markModified);
        }

    }

    if (s & DocReportEcho::Gestation2) {

        if (enableConnection) {

            // line edit
            QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gestation2)->findChildren<QLineEdit*>();
            for (int n = 0; n < list_line_edit.count(); n++) {
                QObject::connect(list_line_edit[n], &QLineEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }
            //plain text edit
            QList<QPlainTextEdit*> list_plain_text_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gestation2)->findChildren<QPlainTextEdit*>();
            for (int n = 0; n < list_plain_text_edit.count(); n++) {
                QObject::connect(list_plain_text_edit[n], &QPlainTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }
            //combobox
            QList<QComboBox*> list_combo = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gestation2)->findChildren<QComboBox*>();
            for (int n = 0; n < list_combo.count(); n++) {
                QObject::connect(list_combo[n], &QComboBox::currentTextChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }

            ui->gestation2_comment->setProperty("maxLen", 255);
            QObject::connect(ui->gestation2_comment, &QPlainTextEdit::textChanged,
                             &o, &DocReportEcho::enforceMaxForSender, Qt::UniqueConnection);

            ui->gestation2_concluzion->setProperty("maxLen", 500);
            QObject::connect(ui->gestation2_concluzion, &QPlainTextEdit::textChanged,
                             &o, &DocReportEcho::enforceMaxForSender, Qt::UniqueConnection);

            QObject::connect(ui->gestation2_concluzion, &QPlainTextEdit::textChanged,
                             &o, &DocReportEcho::updateTextConcluzionBySystem, Qt::UniqueConnection);

        } else {

            // line edit
            QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gestation2)->findChildren<QLineEdit*>();
            for (int n = 0; n < list_line_edit.count(); n++) {
                QObject::disconnect(list_line_edit[n], &QLineEdit::textChanged,
                                 &o, &DocReportEcho::markModified);
            }
            //plain text edit
            QList<QPlainTextEdit*> list_plain_text_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gestation2)->findChildren<QPlainTextEdit*>();
            for (int n = 0; n < list_plain_text_edit.count(); n++) {
                QObject::disconnect(list_plain_text_edit[n], &QPlainTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified);
            }
            //combobox
            QList<QComboBox*> list_combo = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gestation2)->findChildren<QComboBox*>();
            for (int n = 0; n < list_combo.count(); n++) {
                QObject::disconnect(list_combo[n], &QComboBox::currentTextChanged,
                                 &o, &DocReportEcho::markModified);
            }

        }

    }

    if (s & DocReportEcho::LymphNodes) {

        if (enableConnection) {

            // line edit
            QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_LymphNodes)->findChildren<QLineEdit*>();
            for (int n = 0; n < list_line_edit.count(); n++) {
                QObject::connect(list_line_edit[n], &QLineEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }
            //plain text edit
            QList<QPlainTextEdit*> list_plain_text_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_LymphNodes)->findChildren<QPlainTextEdit*>();
            for (int n = 0; n < list_plain_text_edit.count(); n++) {
                QObject::connect(list_plain_text_edit[n], &QPlainTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }
            //combobox
            QList<QComboBox*> list_combo = ui->stackedWidget->widget(DocReportEcho::PageReport::page_LymphNodes)->findChildren<QComboBox*>();
            for (int n = 0; n < list_combo.count(); n++) {
                QObject::connect(list_combo[n], &QComboBox::currentTextChanged,
                                 &o, &DocReportEcho::markModified, Qt::UniqueConnection);
            }

            ui->ln_conluzion->setProperty("maxLen", 500);
            QObject::connect(ui->ln_conluzion, &QPlainTextEdit::textChanged,
                             &o, &DocReportEcho::enforceMaxForSender, Qt::UniqueConnection);

            QObject::connect(ui->ln_typeInvestig, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), &o, [ui](int index){
                ui->tabSoftTissues->setEnabled(index == 0 || index == 1);
                ui->tabLympNodes->setEnabled(index == 2 || index == 0);
                if (index == 0 || index == 1) {
                    ui->tabWidget_LymphNodes->setCurrentIndex(0);
                } else if (index == 2 || index == 0) {
                    ui->tabWidget_LymphNodes->setCurrentIndex(1);
                }
            });


        } else {

            // line edit
            QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gestation2)->findChildren<QLineEdit*>();
            for (int n = 0; n < list_line_edit.count(); n++) {
                QObject::disconnect(list_line_edit[n], &QLineEdit::textChanged,
                                 &o, &DocReportEcho::markModified);
            }
            //plain text edit
            QList<QPlainTextEdit*> list_plain_text_edit = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gestation2)->findChildren<QPlainTextEdit*>();
            for (int n = 0; n < list_plain_text_edit.count(); n++) {
                QObject::disconnect(list_plain_text_edit[n], &QPlainTextEdit::textChanged,
                                 &o, &DocReportEcho::markModified);
            }
            //combobox
            QList<QComboBox*> list_combo = ui->stackedWidget->widget(DocReportEcho::PageReport::page_gestation2)->findChildren<QComboBox*>();
            for (int n = 0; n < list_combo.count(); n++) {
                QObject::disconnect(list_combo[n], &QComboBox::currentTextChanged,
                                 &o, &DocReportEcho::markModified);
            }

        }
    }
}
