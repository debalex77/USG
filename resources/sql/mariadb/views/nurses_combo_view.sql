CREATE OR REPLACE VIEW v_nurses_active AS
SELECT
    id,
    name,
    CONCAT(name, ' ', LEFT(fname, 1), '.') AS display
FROM
    nurses
WHERE
    deletionMark = 0
ORDER BY
    name, fname;
