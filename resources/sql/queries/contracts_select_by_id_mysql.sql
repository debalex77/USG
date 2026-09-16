SELECT
    id,
    deletionMark,
    id_organizations,
    id_typesPrices,
    name,
    DATE_FORMAT(dateInit, '%d.%m.%Y') AS dateInit,
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
