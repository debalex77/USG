SELECT
    p.id,
    p.deletionMark,
    p.numberDoc,
    p.dateDoc,
    p.id_organizations   AS idOrganizations,
    p.id_contracts       AS idContracts,
    p.id_typesprices     AS idTypesPrices,
    p.id_users           AS idUsers,
    tp.name              AS typePriceName,
    o.name               AS organizationName,
    c.name               AS contractName,
    u.name               AS userName,
    p.comment,
    p.uuid
FROM
    pricings p
INNER JOIN organizations o ON o.id = p.id_organizations
INNER JOIN contracts c     ON c.id = p.id_contracts
INNER JOIN typesPrices tp  ON tp.id = p.id_typesprices
INNER JOIN users u         ON u.id = p.id_users
WHERE
    c.notValid = 0
    %nr_doc%
    %id_org%
    %id_cont%
    %id_us%
    AND p.dateDoc BETWEEN ? AND ?
