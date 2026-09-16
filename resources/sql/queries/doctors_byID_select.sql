SELECT
    d.signature,
    d.stamp,
    fn.name AS fullName,
    fn.nameAbbreviated
FROM
    doctors d
INNER JOIN
    fullNameDoctors fn ON d.id = fn.id_doctors
WHERE
    d.deletionMark = 0 AND
    d.id = ?
