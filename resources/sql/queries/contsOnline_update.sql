UPDATE contsOnline SET
    id_users      = ?,
    email         = ?,
    smtp_server   = ?,
    port          = ?,
    username      = ?,
    password      = ?,
    iv            = ?
WHERE
    id_organizations = ?
