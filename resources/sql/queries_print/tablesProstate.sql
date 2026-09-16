SELECT
    rp.docPresentationDate AS title_report,
    tp.dimens         AS prostate_dimens,
    tp.volume         AS prostate_volume,
    tp.ecostructure   AS prostate_ecostructure,
    tp.contour        AS prostate_contur,
    tp.ecogency       AS prostate_ecogency,
    tp.formations     AS prostate_formations,
    tp.concluzion     AS prostate_concluzion,
    tp.recommendation AS prostate_recommendation,
    r.concluzion
FROM
    reportEcho r
INNER JOIN
    reportEchoPresentation rp ON rp.id_reportEcho = r.id
INNER JOIN
    tableProstate tp ON tp.id_reportEcho = r.id
WHERE
    r.deletionMark = 2 AND
    r.id = %id%
