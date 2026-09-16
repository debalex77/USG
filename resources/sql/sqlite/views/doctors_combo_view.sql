CREATE VIEW IF NOT EXISTS v_doctors_active AS
SELECT
    id,
    name,
    fname,
    name || ' ' || substr(fname, 1, 1) || '.' AS display
FROM doctors
WHERE deletionMark = 0
ORDER BY name, fname;
