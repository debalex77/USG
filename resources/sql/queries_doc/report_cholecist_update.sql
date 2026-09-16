UPDATE tableCholecist SET
    form       = :form,
    dimens     = :dimens,
    walls      = :walls,
    choledoc   = :choledoc,
    formations = :formations
WHERE
    id_reportEcho = :id_reportEcho