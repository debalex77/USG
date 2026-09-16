CREATE VIEW IF NOT EXISTS v_contracts_listView_active AS
SELECT
    c.id,
    c.name,
    c.name || ' (' || o.name || ')' AS contract_owner,
    strftime('%d.%m.%Y', c.dateInit) AS contract_dateInit,
    tp.name AS name_typePrice,
    c.id_typesPrices AS id_typesPrices
FROM
    contracts c
INNER JOIN
    organizations o ON o.id_contracts = c.id
INNER JOIN
    typesPrices tp ON tp.id = c.id_typesPrices
WHERE
    c.deletionMark = 0 AND
    c.notValid = 0 AND
    o.deletionMark = 0 AND
    tp.deletionMark = 0
