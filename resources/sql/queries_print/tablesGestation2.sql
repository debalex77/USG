SELECT
    rp.docPresentationDate AS title_report,
    /* ----------------- main ----------------- */
    tg.gestation_age,
    tg.trimestru,
    tg.dateMenstruation,
    CASE
        tg.view_examination
        WHEN 0 THEN 'adecvată'
        WHEN 1 THEN 'limitată'
        ELSE 'dificilă'
    END as view_examination,
    CASE
        tg.single_multiple_pregnancy
        WHEN 0 THEN 'monofetală'
        ELSE 'multiplă'
    END as pregnancy,
    tg.single_multiple_pregnancy_description,
    tg.antecedent,
    tg.comment,
    tg.concluzion,
    tg.recommendation,
    /* ----------------- biometry ----------------- */
    bio.BPD,
    bio.BPD_age,
    bio.HC,
    bio.HC_age,
    bio.AC,
    bio.AC_age,
    bio.FL,
    bio.FL_age,
    bio.FetusCorresponds,
    /* ----------------- cranium ----------------- */
    CASE
        cran.calloteCranium
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as calloteCranium,
    CASE
        cran.facialeProfile
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as facialeProfile,
    CASE
        cran.nasalBones
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as nasalBones,
    cran.nasalBones_dimens,
    CASE
        cran.eyeball
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as eyeball,
    cran.eyeball_desciption,
    CASE
        cran.nasolabialTriangle
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as nasolabialTriangle,
    cran.nasolabialTriangle_description,
    cran.nasalFold,
    /* ----------------- snc ----------------- */
    CASE
        snc.hemispheres
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as hemispheres,
    CASE
        snc.fissureSilvius
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as fissureSilvius,
    CASE
        snc.corpCalos
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as corpCalos,
    CASE
        snc.ventricularSystem
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as ventricularSystem,
    snc.ventricularSystem_description,
    CASE
        snc.cavityPellucidSeptum
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as cavityPellucidSeptum,
    CASE
        snc.choroidalPlex
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as choroidalPlex,
    snc.choroidalPlex_description,
    CASE
        snc.cerebellum
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as cerebellum,
    snc.cerebellum_description,
    CASE
        snc.vertebralColumn
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as vertebralColumn,
    snc.vertebralColumn_description,
    /* ----------------- heart ----------------- */
    hrt.position,
    CASE
        hrt.heartBeat
        WHEN 0 THEN 'prezente'
        ELSE 'absente'
    END as heartBeat,
    hrt.heartBeat_frequency,
    CASE
        hrt.heartBeat_rhythm
        WHEN 0 THEN 'ritmice'
        ELSE 'aritmice'
    END as heartBeat_rhythm,
    CASE
        hrt.pericordialCollections
        WHEN 0 THEN 'absente'
        ELSE 'prezente'
    END as pericordialCollections,
    CASE
        hrt.planPatruCamere
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as planPatruCamere,
    hrt.planPatruCamere_description,
    CASE
        hrt.ventricularEjectionPathLeft
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as ventricularEjectionPathLeft,
    hrt.ventricularEjectionPathLeft_description,
    CASE
        hrt.ventricularEjectionPathRight
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as ventricularEjectionPathRight,
    hrt.ventricularEjectionPathRight_description,
    CASE
        hrt.intersectionVesselMagistral
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as intersectionVesselMagistral,
    hrt.intersectionVesselMagistral_description,
    CASE
        hrt.planTreiVase
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as planTreiVase,
    hrt.planTreiVase_description,
    CASE
        hrt.archAorta
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as archAorta,
    CASE
        hrt.planBicav
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as planBicav,
    /* ----------------- thorax ----------------- */
    CASE
        thx.pulmonaryAreas
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as pulmonaryAreas,
    thx.pulmonaryAreas_description,
    CASE
        thx.pleuralCollections
        WHEN 0 THEN 'absente'
        ELSE 'prezente'
    END as pleuralCollections,
    CASE
        thx.diaphragm
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as diaphragm,
    /* ----------------- abdomen ----------------- */
    CASE
        abd.abdominalWall
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as abdominalWall,
    CASE
        abd.abdominalCollections
        WHEN 0 THEN 'absente'
        WHEN 1 THEN 'prezente'
        ELSE 'dificil'
    END as abdominalCollections,
    CASE
        abd.stomach
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as stomach,
    abd.stomach_description,
    CASE
        abd.abdominalOrgans
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as abdominalOrgans,
    CASE
        abd.cholecist
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as cholecist,
    abd.cholecist_description,
    CASE
        abd.intestine
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as intestine,
    abd.intestine_description,
    /* ----------------- urinary system ----------------- */
    CASE
        us.kidneys
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as kidneys,
    us.kidneys_descriptions,
    CASE
        us.ureter
        WHEN 0 THEN 'nonvizibile'
        ELSE 'vizibile'
    END as ureter,
    us.ureter_descriptions,
    CASE
        us.bladder
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as bladder,
    /* ----------------- other ----------------- */
    CASE
        oth.externalGenitalOrgans
        WHEN 0 THEN 'masc.'
        WHEN 1 THEN 'fem.'
        ELSE 'dificil'
    END as externalGenitalOrgans,
    CASE
        oth.extremities
        WHEN 0 THEN 'normal'
        WHEN 1 THEN 'anormal'
        ELSE 'dificil'
    END as extremities,
    oth.extremities_descriptions,
    oth.fetusMass,
    CASE
        oth.placenta
        WHEN 0 THEN 'nonprevia'
        ELSE 'previa'
    END as placenta,
    oth.placentaLocalization,
    oth.placentaDegreeMaturation,
    oth.placentaDepth,
    CASE
        oth.placentaStructure
        WHEN 0 THEN 'omogenă'
        ELSE 'neomogenă'
    END as placentaStructure,
    oth.placentaStructure_descriptions,
    CASE
        oth.umbilicalCordon
        WHEN 0 THEN 'trei vase'
        ELSE 'două vase'
    END as umbilicalCordon,
    oth.umbilicalCordon_description,
    CASE
        oth.insertionPlacenta
        WHEN 0 THEN 'centrală'
        WHEN 1 THEN 'excentrică'
        WHEN 2 THEN 'periferică'
        WHEN 3 THEN 'marginală'
        ELSE 'velamentoasă'
    END as insertionPlacenta,
    oth.amnioticIndex,
    CASE
        oth.amnioticIndexAspect
        WHEN 0 THEN 'omogen'
        ELSE 'neomogen'
    END as amnioticIndexAspect,
    oth.amnioticBedDepth,
    oth.cervix,
    oth.cervix_description,
    /* ----------------- doppler ----------------- */
    dopp.ombilic_PI,
    dopp.ombilic_RI,
    dopp.ombilic_SD,
    CASE
        dopp.ombilic_flux
        WHEN 1 THEN 'normal'
        WHEN 2 THEN 'anormal'
        ELSE ''
    END as ombilic_flux,
    dopp.cerebral_PI,
    dopp.cerebral_RI,
    dopp.cerebral_SD,
    CASE
        dopp.cerebral_flux
        WHEN 1 THEN 'normal'
        WHEN 2 THEN 'anormal'
        ELSE ''
    END as cerebral_flux,
    dopp.uterRight_PI,
    dopp.uterRight_RI,
    dopp.uterRight_SD,
    CASE
        dopp.uterRight_flux
        WHEN 1 THEN 'normal'
        WHEN 2 THEN 'anormal'
        ELSE ''
    END as uterRight_flux,
    dopp.uterLeft_PI,
    dopp.uterLeft_RI,
    dopp.uterLeft_SD,
    CASE
        dopp.uterLeft_flux
        WHEN 1 THEN 'normal'
        WHEN 2 THEN 'anormal'
        ELSE ''
    END as uterLeft_flux,
    CASE
        dopp.ductVenos
        WHEN 1 THEN 'normal'
        WHEN 2 THEN 'anormal redus'
        WHEN 3 THEN 'anormal nul'
        WHEN 4 THEN 'anormal revers'
        WHEN 5 THEN 'dificil'
        ELSE ''
    END as ductVenos
FROM
    tableGestation2 tg
INNER JOIN
    reportEcho r ON r.id = tg.id_reportEcho
LEFT JOIN reportEchoPresentation rp ON
    rp.id_reportEcho = tg.id_reportEcho
LEFT JOIN tableGestation2_biometry bio ON
    bio.id_reportEcho = tg.id_reportEcho
LEFT JOIN tableGestation2_cranium cran ON
    cran.id_reportEcho = tg.id_reportEcho
LEFT JOIN tableGestation2_SNC snc ON
    snc.id_reportEcho = tg.id_reportEcho
LEFT JOIN tableGestation2_heart hrt ON
    hrt.id_reportEcho = tg.id_reportEcho
LEFT JOIN tableGestation2_thorax thx ON
    thx.id_reportEcho = tg.id_reportEcho
LEFT JOIN tableGestation2_abdomen abd ON
    abd.id_reportEcho = tg.id_reportEcho
LEFT JOIN tableGestation2_urinarySystem us ON
    us.id_reportEcho = tg.id_reportEcho
LEFT JOIN tableGestation2_other oth ON
    oth.id_reportEcho = tg.id_reportEcho
LEFT JOIN tableGestation2_doppler dopp ON
    dopp.id_reportEcho = tg.id_reportEcho
WHERE
    r.deletionMark = 2 AND
    tg.id_reportEcho = %id%
