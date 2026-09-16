SELECT
    id,
    deletionMark,
    id_organizations,
    id_typesPrices,
    name,
    dateInit,
    notValid,
    comment,
    uuid
FROM
    contracts
WHERE
    deletionMark = 0 AND
    id_organizations = ?
ORDER BY
    name
