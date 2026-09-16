SELECT
    t.*
FROM
    pricingsTable t
WHERE
    t.deletionMark = 2 AND
    t.price > 0 AND
    t.id_pricings = (
        SELECT
            p.id
        FROM
            pricings p
        WHERE
            p.id_organizations = :id_organization AND
            p.id_contracts     = :id_contract AND
            p.id_typesPrices   = :id_typePrices AND
            p.deletionMark = 2
        ORDER BY
            p.id DESC
        LIMIT 1
    )
ORDER BY
    t.cod
