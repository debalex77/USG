SELECT
    patients.id,
    patients.deletion_mark,
    patients.last_name,
    patients.first_name,
    CONCAT_WS(' ', patients.last_name, NULLIF(patients.first_name, '')) AS FullName,
    DATE_FORMAT(patients.birthday, '%d.%m.%Y') AS birthday,
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
