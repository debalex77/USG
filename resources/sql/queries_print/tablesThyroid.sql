SELECT
    rp.docPresentationDate AS title_report,
    tt.thyroid_right_dimens,
    tt.thyroid_right_volum,
    tt.thyroid_left_dimens,
    tt.thyroid_left_volum,
    tt.thyroid_istm,
    tt.thyroid_ecostructure,
    tt.thyroid_formations,
    tt.thyroid_ganglions,
    tt.concluzion     AS thyroid_concluzion,
    tt.recommendation AS thyroid_recommendation,
    r.concluzion
FROM
    reportEcho r
INNER JOIN
    reportEchoPresentation rp ON rp.id_reportEcho = r.id
INNER JOIN
    tableThyroid tt ON tt.id_reportEcho = r.id
WHERE
    r.deletionMark = 2 AND
    r.id = %id%
