SELECT
    id,
    deletion_mark,
    CONCAT(last_name, ' ', IFNULL(first_name, '')) AS FullName,
    birthday,
    IFNULL(idnp, '') AS idnp,
    medical_policy,
    address,
    telephone,
    email,
    comment,
    uuid
FROM
    patients
