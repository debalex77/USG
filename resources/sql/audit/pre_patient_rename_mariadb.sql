-- Audit read-only înainte de redenumirea pacients -> patients (MariaDB).
-- Toate instrucțiunile sunt SELECT și nu modifică baza.

SELECT 'pacients' AS object_name, COUNT(*) AS row_count FROM pacients
UNION ALL SELECT 'fullNamePacients', COUNT(*) FROM fullNamePacients
UNION ALL SELECT 'orderEcho', COUNT(*) FROM orderEcho
UNION ALL SELECT 'reportEcho', COUNT(*) FROM reportEcho
UNION ALL SELECT 'imagesReports', COUNT(*) FROM imagesReports;

SELECT 'orderEcho_without_patient' AS check_name, COUNT(*) AS issue_count
FROM orderEcho o LEFT JOIN pacients p ON p.id = o.id_pacients WHERE p.id IS NULL
UNION ALL
SELECT 'reportEcho_without_patient', COUNT(*)
FROM reportEcho r LEFT JOIN pacients p ON p.id = r.id_pacients WHERE p.id IS NULL
UNION ALL
SELECT 'imagesReports_without_patient', COUNT(*)
FROM imagesReports i LEFT JOIN pacients p ON p.id = i.id_patients WHERE p.id IS NULL
UNION ALL
SELECT 'fullNamePacients_without_patient', COUNT(*)
FROM fullNamePacients f LEFT JOIN pacients p ON p.id = f.id_pacients WHERE p.id IS NULL;

SELECT
    COUNT(*) AS patients_total,
    SUM(uuid IS NULL OR OCTET_LENGTH(uuid) <> 16) AS invalid_uuid,
    SUM(name IS NULL OR TRIM(name) = '') AS empty_name,
    SUM(fName IS NULL OR TRIM(fName) = '') AS empty_fname,
    SUM(mName IS NOT NULL AND TRIM(mName) <> '') AS used_mname,
    SUM(IDNP IS NOT NULL AND TRIM(IDNP) <> '') AS used_idnp,
    SUM(medicalPolicy IS NOT NULL AND TRIM(medicalPolicy) <> '') AS used_medical_policy,
    SUM(address IS NOT NULL AND TRIM(address) <> '') AS used_address,
    SUM(telephone IS NOT NULL AND TRIM(telephone) <> '') AS used_telephone,
    SUM(email IS NOT NULL AND TRIM(email) <> '') AS used_email,
    SUM(birthday IS NULL OR birthday = '0000-00-00') AS invalid_birthday
FROM pacients;

SELECT HEX(uuid) AS duplicate_uuid, COUNT(*) AS occurrences
FROM pacients
GROUP BY uuid
HAVING uuid IS NULL OR COUNT(*) > 1;

SELECT
    (SELECT MAX(id) FROM pacients) AS max_patient_id,
    (SELECT MAX(id) FROM orderEcho) AS max_order_id,
    (SELECT MAX(id) FROM reportEcho) AS max_report_id,
    (SELECT MAX(id) FROM imagesReports) AS max_image_id;

SELECT TABLE_NAME, COLUMN_NAME, CONSTRAINT_NAME, REFERENCED_TABLE_NAME,
       REFERENCED_COLUMN_NAME
FROM information_schema.KEY_COLUMN_USAGE
WHERE TABLE_SCHEMA = DATABASE()
  AND (TABLE_NAME IN ('pacients', 'fullNamePacients', 'orderEcho', 'reportEcho', 'imagesReports')
       OR REFERENCED_TABLE_NAME = 'pacients')
ORDER BY TABLE_NAME, CONSTRAINT_NAME, ORDINAL_POSITION;

SELECT TABLE_NAME, INDEX_NAME, NON_UNIQUE,
       GROUP_CONCAT(COLUMN_NAME ORDER BY SEQ_IN_INDEX) AS columns_in_index
FROM information_schema.STATISTICS
WHERE TABLE_SCHEMA = DATABASE()
  AND TABLE_NAME IN ('pacients', 'fullNamePacients', 'orderEcho', 'reportEcho', 'imagesReports')
GROUP BY TABLE_NAME, INDEX_NAME, NON_UNIQUE
ORDER BY TABLE_NAME, INDEX_NAME;
