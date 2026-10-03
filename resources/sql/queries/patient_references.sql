SELECT
    refs.kind,
    refs.numberDoc,
    refs.dateDoc
FROM (
    SELECT
        1 AS kind,
        numberDoc,
        dateDoc
    FROM
        orderEcho
    WHERE
        patient_id = ?

    UNION ALL

    SELECT
        2 AS kind,
        numberDoc,
        dateDoc
    FROM
        reportEcho
    WHERE
        patient_id = ?

    UNION ALL

    SELECT
        3 AS kind,
        NULL AS numberDoc,
        dateDoc
    FROM
        patientAppointments
    WHERE
        patient_id = ?
) refs
ORDER BY
    refs.kind,
    refs.dateDoc DESC
