CREATE TABLE IF NOT EXISTS tableGynecology (
    id                          INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_reportEcho               INTEGER NOT NULL,
    transvaginal                INTEGER,
    dateMenstruation            TEXT DEFAULT NULL,
    antecedent                  TEXT DEFAULT NULL,
    uterus_dimens               TEXT DEFAULT NULL,
    uterus_pozition             TEXT DEFAULT NULL,
    uterus_ecostructure         TEXT DEFAULT NULL,
    uterus_formations           TEXT DEFAULT NULL,
    ecou_dimens                 TEXT DEFAULT NULL,
    ecou_ecostructure           TEXT DEFAULT NULL,
    cervix_dimens               TEXT DEFAULT NULL,
    cervix_ecostructure         TEXT DEFAULT NULL,
    douglas                     TEXT DEFAULT NULL,
    plex_venos                  TEXT DEFAULT NULL,
    ovary_right_dimens          TEXT DEFAULT NULL,
    ovary_left_dimens           TEXT DEFAULT NULL,
    ovary_right_volum           TEXT DEFAULT NULL,
    ovary_left_volum            TEXT DEFAULT NULL,
    ovary_right_follicle        TEXT DEFAULT NULL,
    ovary_left_follicle         TEXT DEFAULT NULL,
    ovary_right_formations      TEXT DEFAULT NULL,
    ovary_left_formations       TEXT DEFAULT NULL,
    concluzion                  TEXT DEFAULT NULL,
    recommendation              TEXT DEFAULT NULL,
    junctional_zone             TEXT CHECK(junctional_zone IN ('contur clar', 'contur sters')) DEFAULT 'contur clar',
    junctional_zone_description TEXT DEFAULT NULL,
    cervical_canal              TEXT CHECK(cervical_canal IN ('nedilatat', 'dilatat')) DEFAULT 'nedilatat',
    cervical_canal_formations   TEXT DEFAULT NULL,
    fallopian_tubes             TEXT CHECK(fallopian_tubes IN ('nonvizibile', 'vizibile')) DEFAULT 'nonvizibile',
    fallopian_tubes_formations  TEXT DEFAULT NULL,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho (id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_tableGynecology_reportEcho
ON tableGynecology(id_reportEcho);
