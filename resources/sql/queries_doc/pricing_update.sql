UPDATE pricings SET
    deletionMark     = ?,
    numberDoc        = ?,
    dateDoc          = ?,
    id_typesPrices   = ?,
    id_organizations = ?,
    id_contracts     = ?,
    id_users         = ?,
    comment          = ?
WHERE
    id = ?
