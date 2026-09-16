CREATE VIEW IF NOT EXISTS v_patients_completer_active AS
SELECT
  id,
  last_name || ' ' || IFNULL(first_name, '') || ', ' || strftime('%d.%m.%Y', birthday)
  || ', idnp: ' || IFNULL(idnp, '') AS full_name
FROM
    patients
WHERE
    deletion_mark = 0
ORDER BY
    full_name ASC
