SELECT
    id,
    deletionMark,
    name,
    password,
    hash,
    lastConnection,
    uuid
FROM
    users
ORDER BY
    name
