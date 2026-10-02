SELECT
    r.id,
    r.numberDoc,
    r.dateDoc,
    COALESCE(r.attachedImages, 0) AS attachedImages,
    p.last_name,
    p.first_name,
    COALESCE(p.idnp, '') AS idnp,
    o.numberDoc AS orderNumber
FROM
    reportEcho r
INNER JOIN
    orderEcho o ON o.id = r.id_orderEcho
INNER JOIN
    patients p ON p.id = r.patient_id
WHERE
    r.deletionMark = 2 AND
    o.id_organizations = :idOrganization AND
    r.dateDoc BETWEEN :startDate AND :endDate
ORDER BY
    r.dateDoc DESC,
    r.id DESC
