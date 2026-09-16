SELECT
    id,
    deletion_mark,
    last_name || ' ' || first_name AS FullName,
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
