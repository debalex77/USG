SELECT
    c.hostName,
    c.databaseName,
    c.port,
    c.connectionOption,
    c.username,
    c.password,
    c.iv,
    u.hash AS hashUser
FROM
    cloudServer c
INNER JOIN
    users u ON u.id = c.id_users
WHERE
    c.id_organizations = ? AND
    c.id_users = ?
