UPDATE tableProstate SET
    dimens         = :dimens,
    volume         = :volume,
    ecostructure   = :ecostructure,
    contour        = :contour,
    ecogency       = :ecogency,
    formations     = :formations,
    transrectal    = :transrectal,
    concluzion     = :concluzion,
    recommendation = :recommendation
WHERE
    id_reportEcho = :id_reportEcho
