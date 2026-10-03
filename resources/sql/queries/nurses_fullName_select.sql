SELECT
    id,
    deletionMark,
    name || ' ' || IFNULL(fName, '') AS FullName,
    telephone,
    email,
    comment,
    uuid
FROM
    nurses
