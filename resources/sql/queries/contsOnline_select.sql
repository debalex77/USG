SELECT
    c.email,
    c.smtp_server,
    c.port,
    c.username,
    c.password,
    c.iv,
    c.tag
FROM
    onlineAccount c
WHERE
    c.id_organizations = ? AND
    c.id_users = ? AND
    c.email = ?
