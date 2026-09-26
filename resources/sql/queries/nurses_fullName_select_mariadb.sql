SELECT
    id,
    deletionMark,
    CONCAT(name, ' ', fName) AS FullName,
    telephone,
    email,
    comment,
    uuid
FROM
    nurses
