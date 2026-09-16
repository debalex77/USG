CREATE VIEW IF NOT EXISTS v_types_prices_active AS
SELECT
    id,
    name,
    discount,
    noncomercial,
    uuid
FROM
    typesPrices
WHERE
    deletionMark = 0
