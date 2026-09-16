UPDATE patients SET
    deletion_mark  = ?,
    idnp          = ?,
    last_name          = ?,
    first_name         = ?,
    middle_name         = ?,
    medical_policy = ?,
    birthday      = ?,
    address       = ?,
    telephone     = ?,
    email         = ?,
    comment       = ?
WHERE
    id = ?;
