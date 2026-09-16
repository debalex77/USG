SELECT
    %fullName%,
    %birthday%,
    patients.idnp,
    patients.medical_policy,
    patients.address
FROM
    patients
WHERE
    id = ? AND
    patients.deletion_mark = 0
