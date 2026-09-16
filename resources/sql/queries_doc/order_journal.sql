SELECT
    doc.id,
    doc.deletionMark,
    doc.attachedImages,
    doc.cardPayment,
    doc.numberDoc,
    doc.dateDoc,
    o.id   AS idOrganization,
    c.id   AS idContract,
    p.id   AS idPacient,
    dt.id  AS idDoctor,
    u.id   AS idUser,
    o.name AS organizationName,
    c.name AS contractName,
    p.last_name || ' ' || p.first_name AS pacientFullName,
    p.idnp AS pacientIDNP,
    dt.name || ' ' || substr(dt.fName, 1, 1) || '.' AS doctorName,
    u.name AS userName,
    p.last_name || ' ' || p.first_name || ', idnp: ' || p.idnp || ', ' || strftime('%d.%m.%Y', p.birthday) AS pacientSearch,
    doc.sum,
    doc.comment,
    doc.uuid
FROM
    orderEcho doc
INNER JOIN
    organizations o ON o.id = doc.id_organizations
INNER JOIN
    contracts c ON c.id = doc.id_contracts
INNER JOIN
    patients p ON p.id = doc.patient_id
LEFT JOIN
    doctors dt ON dt.id = doc.id_doctors
INNER JOIN
    users u ON u.id = doc.id_users
WHERE
    c.notValid = 0
    %nr_doc%
    %id_org%
    %id_cont%
    %id_us%
    %id_pacient%
    AND doc.dateDoc BETWEEN :startDate AND :endDate
