UPDATE tableGestation2_abdomen SET
    abdominalWall         = :abdominalWall,
    abdominalCollections  = :abdominalCollections,
    stomach               = :stomach,
    stomach_description   = :stomach_description,
    abdominalOrgans       = :abdominalOrgans,
    cholecist             = :cholecist,
    cholecist_description = :cholecist_description,
    intestine             = :intestine,
    intestine_description = :intestine_description
WHERE
    id_reportEcho = :id_reportEcho