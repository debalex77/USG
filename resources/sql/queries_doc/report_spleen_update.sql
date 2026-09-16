UPDATE tableSpleen SET
    dimens     = :dimens,
    contur     = :contur,
    parenchim  = :parenchim,
    formations = :formations
WHERE
    id_reportEcho = :id_reportEcho