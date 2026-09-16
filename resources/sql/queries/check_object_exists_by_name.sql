SELECT
    count(name)
FROM
    %1
WHERE
    name = ?
    AND deletionMark = 0
