SELECT
    cod,
    name
FROM
    investigations
WHERE
    deletionMark = 0
    AND investigations.`use` = 1
ORDER BY
    cod;
