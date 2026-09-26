UPDATE onlineAccount SET
    email         = ?,
    smtp_server   = ?,
    port          = ?,
    username      = ?,
    password      = ?,
    iv            = ?,
    tag           = ?
WHERE
    id_organizations = ? AND
    id_users = ?
