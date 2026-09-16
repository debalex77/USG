SELECT
    r.id,
    rp.docPresentation AS full_name_doc,
    r.concluzion
FROM
    reportEcho r
INNER JOIN
    reportEchoPresentation rp ON rp.id_reportEcho = r.id
WHERE
    r.patient_id = ? AND
    r.deletionMark = 2
ORDER BY r.dateDoc DESC, r.id DESC
