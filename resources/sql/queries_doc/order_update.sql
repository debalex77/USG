UPDATE orderEcho SET
    deletionMark       = ?,
    numberDoc          = ?,
    dateDoc            = ?,
    id_organizations   = ?,
    id_contracts       = ?,
    id_typesPrices     = ?,
    id_doctors         = ?,
    id_doctors_execute = ?,
    id_nurses          = ?,
    patient_id        = ?,
    id_users           = ?,
    sum                = ?,
    comment            = ?,
    cardPayment        = ?,
    attachedImages     = ?
WHERE
    id = ?
