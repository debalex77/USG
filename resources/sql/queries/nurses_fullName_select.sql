SELECT
    id,
    deletionMark,
    name ||' '|| fName AS FullName,
    telephone,
    email,
    comment,
    uuid
FROM
    nurses
