UPDATE organizations SET
    deletionMark = ?,
    IDNP         = ?,
    TVA          = ?,
    name         = ?,
    address      = ?,
    telephone    = ?,
    email        = ?,
    comment      = ?,
    id_contracts = ?,
    stamp        = ?
WHERE
    id = ?
