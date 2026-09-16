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
    id = ?
ORDER BY
    name
