UPDATE cloudServer SET
    hostName         = ?,
    databaseName     = ?,
    port             = ?,
    connectionOption = ?,
    username         = ?,
    password         = ?,
    iv               = ?
WHERE
    id_organizations = ? AND
    id_users = ?
