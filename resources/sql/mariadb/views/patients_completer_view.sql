CREATE VIEW IF NOT EXISTS v_patients_completer_active AS
SELECT
  patients.id,
  CONCAT(
    patients.last_name, ' ', IFNULL(patients.first_name, ''), ', ',
    DATE_FORMAT(patients.birthday, '%d.%m.%Y'),
    ', idnp: ',
    IFNULL(patients.idnp, '')
  ) AS full_name
FROM
    patients
WHERE
    patients.deletion_mark = 0
ORDER BY
    full_name ASC;
