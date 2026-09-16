SELECT
    r.id, r.deletionMark, COALESCE(r.attachedImages, 0), r.numberDoc, r.dateDoc,
    r.id_orderEcho, p.id, u.id,
    CONCAT_WS(' ', p.last_name, NULLIF(p.first_name, '')), COALESCE(p.idnp, ''),
    CONCAT('Comanda ecografică nr. ', o.numberDoc, ' din ', DATE_FORMAT(o.dateDoc, '%d.%m.%Y %H:%i:%s')),
    u.name, COALESCE(r.concluzion, ''), COALESCE(r.comment, ''),
    CONCAT(CONCAT_WS(' ', p.last_name, NULLIF(p.first_name, '')), ', IDNP: ',
           COALESCE(p.idnp, ''), ', ', DATE_FORMAT(p.birthday, '%d.%m.%Y')),
    r.uuid
FROM reportEcho r
INNER JOIN patients p ON p.id = r.patient_id
INNER JOIN orderEcho o ON o.id = r.id_orderEcho
INNER JOIN users u ON u.id = r.id_users
WHERE r.dateDoc BETWEEN :startDate AND :endDate
    %nr_doc%
    %id_org%
    %id_cont%
    %id_us%
    %id_pacient%
    %patient_name%
