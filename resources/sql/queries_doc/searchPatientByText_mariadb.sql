SELECT
    patients.id,
    CONCAT(
        patients.last_name, ' ',
        IFNULL(patients.first_name, ''), ', ',
        IFNULL(DATE_FORMAT(patients.birthday, '%d.%m.%Y'), ''),
        ', idnp: ',
        IFNULL(patients.idnp, '')
    ) AS FullName
FROM patients
WHERE patients.deletion_mark = 0
  AND (
        CONCAT(patients.last_name, ' ', IFNULL(patients.first_name, '')) LIKE ?
     OR CONCAT(IFNULL(patients.first_name, ''), ' ', patients.last_name) LIKE ?
     OR patients.idnp LIKE ?
     OR DATE_FORMAT(patients.birthday, '%d.%m.%Y') LIKE ?
  )
ORDER BY patients.last_name, patients.first_name
LIMIT 50
