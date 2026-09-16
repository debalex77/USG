CREATE OR REPLACE VIEW v_organizations_active AS
SELECT
    o.*,
    c.name AS name_contract,
    tp.id AS id_typePrice,
    tp.name AS name_typePrice
FROM
    organizations o
LEFT JOIN
    contracts c ON c.id = o.id_contracts
    AND c.deletionMark = 0
    AND c.notValid = 0
LEFT JOIN
    typesPrices tp ON tp.id = c.id_typesPrices
    AND tp.deletionMark = 0
WHERE
    o.deletionMark = 0
