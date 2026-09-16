UPDATE tableBladder SET
    volum      = :volum,
    walls      = :walls,
    formations = :formations
WHERE
    id_reportEcho = :id_reportEcho