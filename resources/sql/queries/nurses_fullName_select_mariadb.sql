SELECT
    id,
    deletionMark,
    CONCAT(name, ' ', IFNULL(fName, '')) AS FullName,
    telephone,
    email,
    comment,
    uuid
FROM
    nurses
