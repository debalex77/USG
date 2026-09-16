CREATE VIEW IF NOT EXISTS v_nurses_active AS
SELECT
    id,
    name,
    name || ' ' || substr(fname, 1, 1) || '.' AS display
FROM
    nurses
WHERE
    deletionMark = 0
ORDER BY
    name, fname;
