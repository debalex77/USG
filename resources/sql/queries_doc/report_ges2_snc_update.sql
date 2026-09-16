UPDATE tableGestation2_SNC SET
    hemispheres                   = :hemispheres,
    fissureSilvius                = :fissureSilvius,
    corpCalos                     = :corpCalos,
    ventricularSystem             = :ventricularSystem,
    ventricularSystem_description = :ventricularSystem_description,
    cavityPellucidSeptum          = :cavityPellucidSeptum,
    choroidalPlex                 = :choroidalPlex,
    choroidalPlex_description     = :choroidalPlex_description,
    cerebellum                    = :cerebellum,
    cerebellum_description        = :cerebellum_description,
    vertebralColumn               = :vertebralColumn,
    vertebralColumn_description   = :vertebralColumn_description
WHERE
    id_reportEcho = :id_reportEcho