UPDATE users SET
    deletionMark   = ?,
    name           = ?,
    password       = ?,
    hash           = ?,
    lastConnection = ?
WHERE
    id = ?
