UPDATE tableGestation2_urinarySystem SET
    kidneys              = :kidneys,
    kidneys_descriptions = :kidneys_descriptions,
    ureter               = :ureter,
    ureter_descriptions  = :ureter_descriptions,
    bladder              = :bladder
WHERE
    id_reportEcho = :id_reportEcho