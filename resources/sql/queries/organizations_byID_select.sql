SELECT
    name,
    IDNP,
    address,
    telephone,
    email,
    comment,
    id_contracts,
    stamp,
    uuid
FROM
    organizations
WHERE
    deletionMark = 0 AND
    id = ?
ORDER BY
    name;
