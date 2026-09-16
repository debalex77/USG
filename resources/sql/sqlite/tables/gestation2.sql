-- =====================================================
-- tableGestation2
-- =====================================================
CREATE TABLE IF NOT EXISTS tableGestation2 (
    id                                    INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_reportEcho                         INTEGER NOT NULL,
    gestation_age                         TEXT,
    trimestru                             INTEGER,
    dateMenstruation                      TEXT,
    view_examination                      INTEGER,
    single_multiple_pregnancy             INTEGER,
    single_multiple_pregnancy_description TEXT,
    antecedent                            TEXT,
    comment                               TEXT,
    concluzion                            TEXT,
    recommendation                        TEXT,
    fetalPrezentation                     INTEGER DEFAULT (0),
    multiplePregnancy                     INTEGER DEFAULT (0),
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho (id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_tableGestation2_reportEcho
ON tableGestation2(id_reportEcho);

-- =====================================================
-- tableGestation2_biometry
-- =====================================================
CREATE TABLE IF NOT EXISTS tableGestation2_biometry (
    id               INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_reportEcho    INTEGER NOT NULL,
    BPD              TEXT,
    BPD_age          TEXT,
    HC               TEXT,
    HC_age           TEXT,
    AC               TEXT,
    AC_age           TEXT,
    FL               TEXT,
    FL_age           TEXT,
    FetusCorresponds TEXT,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho (id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_tableGestation2_biometry_reportEcho
ON tableGestation2_biometry(id_reportEcho);

-- =====================================================
-- tableGestation2_cranium
-- =====================================================
CREATE TABLE IF NOT EXISTS tableGestation2_cranium (
    id                             INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_reportEcho                  INTEGER NOT NULL,
    calloteCranium                 INTEGER,
    facialeProfile                 INTEGER,
    nasalBones                     INTEGER,
    nasalBones_dimens              TEXT,
    eyeball                        INTEGER,
    eyeball_desciption             TEXT,
    nasolabialTriangle             INTEGER,
    nasolabialTriangle_description TEXT,
    nasalFold                      TEXT,
    FOREIGN KEY (`id_reportEcho`)
        REFERENCES reportEcho (id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_tableGestation2_cranium_reportEcho
ON tableGestation2_cranium(id_reportEcho);

-- =====================================================
-- tableGestation2_SNC
-- =====================================================
CREATE TABLE IF NOT EXISTS tableGestation2_SNC (
    id                            INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    id_reportEcho                 INTEGER NOT NULL,
    hemispheres                   INTEGER,
    fissureSilvius                INTEGER,
    corpCalos                     INTEGER,
    ventricularSystem             INTEGER,
    ventricularSystem_description TEXT,
    cavityPellucidSeptum          INTEGER,
    choroidalPlex                 INTEGER,
    choroidalPlex_description     TEXT,
    cerebellum                    INTEGER,
    cerebellum_description        TEXT,
    vertebralColumn               INTEGER,
    vertebralColumn_description   TEXT,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho (id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_tableGestation2_SNC_reportEcho
ON tableGestation2_SNC(id_reportEcho);

-- =====================================================
-- tableGestation2_heart
-- =====================================================
CREATE TABLE IF NOT EXISTS tableGestation2_heart (
    id                                       INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_reportEcho                            INTEGER NOT NULL,
    position                                 TEXT,
    heartBeat                                INTEGER,
    heartBeat_frequency                      TEXT,
    heartBeat_rhythm                         INTEGER,
    pericordialCollections                   INTEGER,
    planPatruCamere                          INTEGER,
    planPatruCamere_description              TEXT,
    ventricularEjectionPathLeft              INTEGER,
    ventricularEjectionPathLeft_description  TEXT,
    ventricularEjectionPathRight             INTEGER,
    ventricularEjectionPathRight_description TEXT,
    intersectionVesselMagistral              INTEGER,
    intersectionVesselMagistral_description  TEXT,
    planTreiVase                             INTEGER,
    planTreiVase_description                 TEXT,
    archAorta                                INTEGER,
    planBicav                                INTEGER,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho (id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_tableGestation2_heart_reportEcho
ON tableGestation2_heart(id_reportEcho);

-- =====================================================
-- tableGestation2_thorax
-- =====================================================
CREATE TABLE IF NOT EXISTS tableGestation2_thorax (
    id                         INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_reportEcho              INTEGER NOT NULL,
    pulmonaryAreas             INTEGER,
    pulmonaryAreas_description TEXT,
    pleuralCollections         INTEGER,
    diaphragm                  INTEGER,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho (id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_tableGestation2_thorax_reportEcho
ON tableGestation2_thorax(id_reportEcho);

-- =====================================================
-- tableGestation2_abdomen
-- =====================================================
CREATE TABLE IF NOT EXISTS tableGestation2_abdomen (
    id                    INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_reportEcho         INTEGER NOT NULL,
    abdominalWall         INTEGER,
    abdominalCollections  INTEGER,
    stomach               INTEGER,
    stomach_description   TEXT,
    abdominalOrgans       INTEGER,
    cholecist             INTEGER,
    cholecist_description TEXT,
    intestine             INTEGER,
    intestine_description TEXT,
    CONSTRAINT tableGestation2_abdomen_reportEcho_id FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho (id) ON DELETE CASCADE ON UPDATE RESTRICT
);

CREATE INDEX IF NOT EXISTS idx_tableGestation2_abdomen_reportEcho
ON tableGestation2_abdomen(id_reportEcho);

-- =====================================================
-- tableGestation2_urinarySystem
-- =====================================================
CREATE TABLE IF NOT EXISTS tableGestation2_urinarySystem (
    id                   INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    id_reportEcho        INTEGER NOT NULL,
    kidneys              INTEGER,
    kidneys_descriptions TEXT,
    ureter               INTEGER,
    ureter_descriptions  TEXT,
    bladder              INTEGER,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho (id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_tableGestation2_urinarySystem_reportEcho
ON tableGestation2_urinarySystem(id_reportEcho);

-- =====================================================
-- tableGestation2_other
-- =====================================================
CREATE TABLE IF NOT EXISTS tableGestation2_other (
    id                             INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_reportEcho                  INTEGER NOT NULL,
    externalGenitalOrgans          INTEGER,
    externalGenitalOrgans_aspect   INTEGER,
    extremities                    INTEGER,
    extremities_descriptions       TEXT,
    fetusMass                      TEXT,
    placenta                       INTEGER,
    placentaLocalization           TEXT,
    placentaDegreeMaturation       TEXT,
    placentaDepth                  TEXT,
    placentaStructure              INTEGER,
    placentaStructure_descriptions TEXT,
    umbilicalCordon                INTEGER,
    umbilicalCordon_description    TEXT,
    insertionPlacenta              INTEGER,
    amnioticIndex                  TEXT,
    amnioticIndexAspect            INTEGER,
    amnioticBedDepth               TEXT,
    cervix                         TEXT,
    cervix_description             TEXT,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho (id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_tableGestation2_other_reportEcho
ON tableGestation2_other(id_reportEcho);

-- =====================================================
-- tableGestation2_doppler
-- =====================================================
CREATE TABLE IF NOT EXISTS tableGestation2_doppler (
    id             INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_reportEcho  INTEGER NOT NULL,
    ombilic_PI     TEXT,
    ombilic_RI     TEXT,
    ombilic_SD     TEXT,
    ombilic_flux   INTEGER,
    cerebral_PI    TEXT,
    cerebral_RI    TEXT,
    cerebral_SD    TEXT,
    cerebral_flux  INTEGER,
    uterRight_PI   TEXT,
    uterRight_RI   TEXT,
    uterRight_SD   TEXT,
    uterRight_flux INTEGER,
    uterLeft_PI    TEXT,
    uterLeft_RI    TEXT,
    uterLeft_SD    TEXT,
    uterLeft_flux  INTEGER,
    ductVenos      TEXT,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho (id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_tableGestation2_doppler_reportEcho
ON tableGestation2_doppler(id_reportEcho);
