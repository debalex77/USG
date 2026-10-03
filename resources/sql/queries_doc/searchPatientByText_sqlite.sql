SELECT
    patients.id,
    patients.last_name || ' ' ||
    IFNULL(patients.first_name, '') || ', ' ||
    IFNULL(strftime('%d.%m.%Y', patients.birthday), '') ||
    ', idnp: ' ||
    IFNULL(patients.idnp, '') AS FullName
FROM patients
WHERE patients.deletion_mark = 0
  AND (
        (patients.last_name || ' ' || IFNULL(patients.first_name, '')) LIKE ?
     OR (IFNULL(patients.first_name, '') || ' ' || patients.last_name) LIKE ?
     OR patients.idnp LIKE ?
     OR strftime('%d.%m.%Y', patients.birthday) LIKE ?
  )
ORDER BY patients.last_name, patients.first_name
LIMIT 50
