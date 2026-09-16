SELECT
    rp.docPresentationDate AS title_report,
    tg.transvaginal,
    %lmp% AS dateMenstruation,
    tg.antecedent,
    tg.uterus_dimens,
    tg.uterus_pozition,
    tg.uterus_ecostructure,
    tg.uterus_formations,
    tg.junctional_zone,
    tg.junctional_zone_description,
    tg.ecou_dimens,
    tg.ecou_ecostructure,
    tg.cervix_dimens,
    tg.cervix_ecostructure,
    tg.cervical_canal,
    tg.cervical_canal_formations,
    tg.douglas,
    tg.plex_venos,
    tg.ovary_right_dimens,
    tg.ovary_left_dimens,
    tg.ovary_right_volum,
    tg.ovary_left_volum,
    tg.ovary_right_follicle,
    tg.ovary_left_follicle,
    tg.ovary_right_formations,
    tg.ovary_left_formations,
    tg.fallopian_tubes,
    tg.fallopian_tubes_formations,
    tg.concluzion AS gynecology_concluzion,
    tg.recommendation AS gynecology_recommendation,
    r.concluzion
FROM
    reportEcho r
INNER JOIN
    reportEchoPresentation rp ON rp.id_reportEcho = r.id
INNER JOIN
    tableGynecology tg ON tg.id_reportEcho = r.id
WHERE
    r.deletionMark = 2 AND
    r.id = %id%
