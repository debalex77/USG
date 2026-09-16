SELECT
    id,
    deletionMark,
    id_organizations,
    id_typesPrices,
    name,
    strftime('%d.%m.%Y', dateInit) AS dateInit,
    notValid,
    comment,
    uuid
FROM
    contracts
WHERE
    deletionMark = 0 AND
    id_organizations = ? AND
    (notValid = 0 OR id = ?)
ORDER BY
    name
