SELECT
    patients.id,
    patients.deletion_mark,
    patients.last_name,
    patients.first_name,
    patients.last_name || ' ' || patients.first_name AS FullName,
    strftime('%d.%m.%Y', patients.birthday) AS birthday,
    patients.medical_policy,
    patients.address,
    patients.telephone,
    patients.email,
    IFNULL(patients.idnp, '') AS IDNP,
    patients.comment,
    patients.uuid
FROM
    patients
WHERE
    patients.id = ? AND
    patients.deletion_mark = 0
