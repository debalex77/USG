SELECT
    r.id, r.deletionMark, COALESCE(r.attachedImages, 0), r.numberDoc, r.dateDoc,
    r.id_orderEcho, p.id, u.id,
    TRIM(p.last_name || ' ' || COALESCE(p.first_name, '')), COALESCE(p.idnp, ''),
    'Comanda ecografică nr. ' || o.numberDoc || ' din ' || strftime('%d.%m.%Y %H:%M:%S', o.dateDoc),
    u.name, COALESCE(r.concluzion, ''), COALESCE(r.comment, ''),
    TRIM(p.last_name || ' ' || COALESCE(p.first_name, '')) || ', IDNP: ' ||
        COALESCE(p.idnp, '') || ', ' || strftime('%d.%m.%Y', p.birthday),
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
