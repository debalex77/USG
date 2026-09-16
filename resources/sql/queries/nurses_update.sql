UPDATE nurses SET
    deletionMark = ?,
    name         = ?,
    fName        = ?,
    mName        = ?,
    telephone    = ?,
    email        = ?,
    comment      = ?
WHERE
    id = ?;
