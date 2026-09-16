-- Audit read-only înainte de redenumirea pacients -> patients (SQLite).
-- Toate instrucțiunile sunt SELECT/PRAGMA și nu modifică baza.

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
    SUM(CASE WHEN uuid IS NULL OR length(uuid) <> 16 THEN 1 ELSE 0 END) AS invalid_uuid,
    SUM(CASE WHEN name IS NULL OR trim(name) = '' THEN 1 ELSE 0 END) AS empty_name,
    SUM(CASE WHEN fName IS NULL OR trim(fName) = '' THEN 1 ELSE 0 END) AS empty_fname,
    SUM(CASE WHEN mName IS NOT NULL AND trim(mName) <> '' THEN 1 ELSE 0 END) AS used_mname,
    SUM(CASE WHEN IDNP IS NOT NULL AND trim(IDNP) <> '' THEN 1 ELSE 0 END) AS used_idnp,
    SUM(CASE WHEN medicalPolicy IS NOT NULL AND trim(medicalPolicy) <> '' THEN 1 ELSE 0 END) AS used_medical_policy,
    SUM(CASE WHEN address IS NOT NULL AND trim(address) <> '' THEN 1 ELSE 0 END) AS used_address,
    SUM(CASE WHEN telephone IS NOT NULL AND trim(telephone) <> '' THEN 1 ELSE 0 END) AS used_telephone,
    SUM(CASE WHEN email IS NOT NULL AND trim(email) <> '' THEN 1 ELSE 0 END) AS used_email,
    SUM(CASE WHEN birthday IS NULL OR date(birthday) IS NULL THEN 1 ELSE 0 END) AS invalid_birthday
FROM pacients;

SELECT lower(hex(uuid)) AS duplicate_uuid, COUNT(*) AS occurrences
FROM pacients
GROUP BY uuid
HAVING uuid IS NULL OR COUNT(*) > 1;

SELECT id, birthday
FROM pacients
WHERE birthday IS NULL OR date(birthday) IS NULL
ORDER BY id;

SELECT
    (SELECT MAX(id) FROM pacients) AS max_patient_id,
    (SELECT MAX(id) FROM orderEcho) AS max_order_id,
    (SELECT MAX(id) FROM reportEcho) AS max_report_id,
    (SELECT MAX(id) FROM imagesReports) AS max_image_id;

PRAGMA foreign_key_check;
PRAGMA index_list('pacients');
PRAGMA index_list('fullNamePacients');
PRAGMA index_list('orderEcho');
PRAGMA index_list('reportEcho');
PRAGMA index_list('imagesReports');
