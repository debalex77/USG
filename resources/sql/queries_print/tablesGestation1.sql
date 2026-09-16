SELECT
    rp.docPresentationDate AS title_report,
    tg.antecedent,
    tg.gestation_age,
    tg.CRL,
    tg.CRL_age,
    tg.BPD,
    tg.BPD_age,
    tg.NT,
    tg.NT_percent,
    tg.BN,
    tg.BN_percent,
    tg.BCF,
    tg.FL,
    tg.FL_age,
    tg.callote_cranium,
    tg.plex_choroid,
    tg.vertebral_column,
    tg.stomach,
    tg.bladder,
    tg.diaphragm,
    tg.abdominal_wall,
    tg.location_placenta,
    tg.sac_vitelin,
    tg.amniotic_liquid,
    tg.miometer,
    tg.cervix,
    tg.ovary,
    tg.concluzion     AS gestation1_concluzion,
    tg.recommendation AS gestation1_recommendation,
    r.concluzion
FROM
    reportEcho r
INNER JOIN
    reportEchoPresentation rp ON rp.id_reportEcho = r.id
INNER JOIN
    tableGestation1 tg ON tg.id_reportEcho = r.id
WHERE
    r.deletionMark = 2 AND
    r.id = %id%
