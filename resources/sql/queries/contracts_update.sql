UPDATE contracts SET
    deletionMark     = ?,
    id_organizations = ?,
    id_typesPrices   = ?,
    name             = ?,
    dateInit         = ?,
    notValid         = ?,
    comment          = ?
WHERE
    id = ?
