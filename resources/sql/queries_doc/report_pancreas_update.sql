UPDATE tablePancreas SET
    cefal      = :cefal,
    corp       = :corp,
    tail       = :tail,
    texture    = :texture,
    ecogency   = :ecogency,
    formations = :formations
WHERE
    id_reportEcho = :id_reportEcho