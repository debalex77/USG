SELECT
    r.id_orderEcho,
    r.numberDoc,
    r.dateDoc,
    r.deletionMark,
    o.id_organizations,
    p.last_name,
    p.first_name
FROM
    reportEcho r
INNER JOIN
    orderEcho o ON o.id = r.id_orderEcho
INNER JOIN
    patients p ON p.id = r.patient_id
WHERE
    r.id = :idReport
