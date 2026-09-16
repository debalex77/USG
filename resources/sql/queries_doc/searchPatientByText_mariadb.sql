SELECT
    patients.id,
    CONCAT(
        patients.last_name, ' ',
        patients.first_name, ', ',
        DATE_FORMAT(patients.birthday, '%d.%m.%Y'),
        ', idnp: ',
        IFNULL(patients.idnp, '')
    ) AS FullName
FROM patients
WHERE patients.deletion_mark = 0
  AND (
        patients.last_name LIKE ?
     OR patients.first_name LIKE ?
     OR patients.idnp LIKE ?
     OR DATE_FORMAT(patients.birthday, '%d.%m.%Y') LIKE ?
  )
ORDER BY patients.last_name, patients.first_name
LIMIT 50
