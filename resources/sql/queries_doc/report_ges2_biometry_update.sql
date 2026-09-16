UPDATE tableGestation2_biometry SET
    BPD     = :BPD,
    BPD_age = :BPD_age,
    HC      = :HC,
    HC_age  = :HC_age,
    AC      = :AC,
    AC_age  = :AC_age,
    FL      = :FL,
    FL_age  = :FL_age,
    FetusCorresponds = :FetusCorresponds
WHERE
    id_reportEcho = :id_reportEcho