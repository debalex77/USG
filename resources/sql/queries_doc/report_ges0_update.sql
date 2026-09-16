UPDATE tableGestation0 SET
    view_examination = :view_examination,
    antecedent       = :antecedent,
    gestation_age    = :gestation_age,
    GS               = :GS,
    GS_age           = :GS_age,
    CRL              = :CRL,
    CRL_age          = :CRL_age,
    BCF              = :BCF,
    liquid_amniotic  = :liquid_amniotic,
    miometer         = :miometer,
    cervix           = :cervix,
    ovary            = :ovary,
    concluzion       = :concluzion,
    recommendation   = :recommendation,
    lmp              = :lmp
WHERE
    id_reportEcho = :id_reportEcho