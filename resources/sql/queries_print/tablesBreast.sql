SELECT
    rp.docPresentationDate AS title_report,
    tb.breast_right_ecostrcture,
    tb.breast_right_duct,
    tb.breast_right_ligament,
    tb.breast_right_formations,
    tb.breast_right_ganglions,
    tb.breast_left_ecostrcture,
    tb.breast_left_duct,
    tb.breast_left_ligament,
    tb.breast_left_formations,
    tb.breast_left_ganglions,
    tb.concluzion     AS breast_concluzion,
    tb.recommendation AS recommendation,
    r.concluzion
FROM
    reportEcho r
INNER JOIN
    reportEchoPresentation rp ON rp.id_reportEcho = r.id
INNER JOIN
    tableBreast tb ON tb.id_reportEcho = r.id
WHERE
    r.deletionMark = 2 AND
    r.id = %id%
