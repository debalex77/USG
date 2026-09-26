SELECT
    id,
    deletionMark,
    cod,
    name,
    investigations.`use`,
    owner,
    uuid
FROM
    investigations
WHERE
    deletionMark = 0
ORDER BY
    cod;
