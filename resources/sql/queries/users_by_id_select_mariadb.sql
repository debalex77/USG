SELECT
    id,
    deletionMark,
    name,
    password,
    hash,
    DATE_FORMAT(lastConnection, '%d.%m.%Y %H:%i:%S') AS lastConnection
FROM
    users
WHERE
    deletionMark = 0
    AND id = ?
