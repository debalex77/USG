SELECT
    id,
    deletionMark,
    cod,
    name,
    "use",
    owner,
    uuid
FROM
    investigations
WHERE
    deletionMark = 0
ORDER BY
    cod;
