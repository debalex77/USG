SELECT
    c.email,
    c.smtp_server,
    c.port,
    c.username,
    c.password,
    c.iv,
    u.hash AS hashUser
FROM
    contsOnline c
INNER JOIN
    users u ON u.id = c.id_users
WHERE
    c.id_organizations = ? AND
    c.id_users = ? AND
    c.email = ?
