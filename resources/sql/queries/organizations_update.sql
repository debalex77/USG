UPDATE organizations SET
    deletionMark = ?,
    IDNP         = ?,
    TVA          = ?,
    name         = ?,
    address      = ?,
    telephone    = ?,
    email        = ?,
    site         = ?,
    comment      = ?,
    id_contracts = ?,
    stamp        = ?
WHERE
    id = ?
