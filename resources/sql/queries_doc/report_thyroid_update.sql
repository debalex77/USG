UPDATE tableThyroid SET
    thyroid_right_dimens = :thyroid_right_dimens,
    thyroid_right_volum  = :thyroid_right_volum,
    thyroid_left_dimens  = :thyroid_left_dimens,
    thyroid_left_volum   = :thyroid_left_volum,
    thyroid_istm         = :thyroid_istm,
    thyroid_ecostructure = :thyroid_ecostructure,
    thyroid_formations   = :thyroid_formations,
    thyroid_ganglions    = :thyroid_ganglions,
    concluzion           = :concluzion,
    recommendation       = :recommendation
WHERE
    id_reportEcho = :id_reportEcho