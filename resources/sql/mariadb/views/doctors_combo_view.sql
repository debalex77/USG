CREATE OR REPLACE VIEW v_doctors_active AS
SELECT
    id,
    name,
    fname,
    CONCAT(name, ' ', LEFT(fname, 1), '.') AS display
FROM doctors
WHERE deletionMark = 0
ORDER BY name, fname;
