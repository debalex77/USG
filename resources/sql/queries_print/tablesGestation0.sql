SELECT
    rp.docPresentationDate AS title_report,
    tg.antecedent,
    tg.gestation_age,
    tg.GS,
    tg.GS_age,
    tg.CRL,
    tg.CRL_age,
    tg.BCF,
    tg.liquid_amniotic,
    tg.miometer,
    tg.cervix,
    tg.ovary,
    tg.concluzion     AS gestation0_concluzion,
    tg.recommendation AS gestation0_recommendation,
    r.concluzion
FROM
    reportEcho r
INNER JOIN
    reportEchoPresentation rp ON rp.id_reportEcho = r.id
INNER JOIN
    tableGestation0 tg ON tg.id_reportEcho = r.id
WHERE
    r.deletionMark = 2 AND
    r.id = %id%
