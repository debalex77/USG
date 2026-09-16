SELECT
    id,
    deletionMark,
    name,
    password,
    hash,
    strftime('%d.%m.%Y %H:%M:%S', lastConnection) AS lastConnection
FROM
    users
WHERE
    deletionMark = 0
    AND id = ?
