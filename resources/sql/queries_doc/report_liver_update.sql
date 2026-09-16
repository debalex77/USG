UPDATE tableLiver SET
    `left`     = :left,
    `right`    = :right,
    contur     = :contur,
    parenchim  = :parenchim,
    ecogenity  = :ecogenity,
    formations = :formations,
    ductsIntrahepatic = :ductsIntrahepatic,
    porta          = :porta,
    lienalis       = :lienalis,
    concluzion     = :concluzion,
    recommendation = :recommendation
WHERE
    id_reportEcho = :id_reportEcho