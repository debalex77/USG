SELECT
    id
FROM
    pricings
WHERE
    deletionMark = 2 AND
    id_organizations = ? AND
    id_contracts = ? AND
    id_typesPrices = ?
ORDER BY
    id DESC
LIMIT 1
