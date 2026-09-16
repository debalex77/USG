SELECT
    id,
    deletionMark,
    name,
    discount,
    noncomercial
    uuid
FROM
    typesPrices
WHERE
    deletionMark = 0
ORDER BY
    name;
