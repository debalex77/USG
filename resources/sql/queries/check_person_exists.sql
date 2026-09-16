SELECT
    COUNT(name)
FROM
    %1
WHERE
    name = ? AND
    fName = ? AND
    mName = ? AND
    deletionMark = 0;
