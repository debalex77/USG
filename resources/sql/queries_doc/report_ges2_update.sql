UPDATE tableGestation2 SET
    gestation_age    = :gestation_age,
    trimestru        = :trimestru,
    dateMenstruation = :dateMenstruation,
    view_examination = :view_examination,
    single_multiple_pregnancy = :single_multiple_pregnancy,
    single_multiple_pregnancy_description = :single_multiple_pregnancy_description,
    antecedent        = :antecedent,
    comment           = :comment,
    concluzion        = :concluzion,
    recommendation    = :recommendation,
    fetalPrezentation = :fetalPrezentation,
    multiplePregnancy = :multiplePregnancy
WHERE
    id_reportEcho = :id_reportEcho