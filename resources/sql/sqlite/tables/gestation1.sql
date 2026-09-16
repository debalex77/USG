CREATE TABLE IF NOT EXISTS tableGestation1 (
    multiplePregnancy INTEGER NOT NULL DEFAULT 0,
    id                INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_reportEcho     INTEGER NOT NULL,
    view_examination  INTEGER,
    antecedent        TEXT,
    gestation_age     TEXT,
    lmp               TEXT,
    CRL               TEXT,
    CRL_age           TEXT,
    BPD               TEXT,
    BPD_age           TEXT,
    NT                TEXT,
    NT_percent        TEXT,
    BN                TEXT,
    BN_percent        TEXT,
    BCF               TEXT,
    FL                TEXT,
    FL_age            TEXT,
    callote_cranium   TEXT,
    plex_choroid      TEXT,
    vertebral_column  TEXT,
    stomach           TEXT,
    bladder           TEXT,
    diaphragm         TEXT,
    abdominal_wall    TEXT,
    location_placenta TEXT,
    sac_vitelin       TEXT,
    amniotic_liquid   TEXT,
    miometer          TEXT,
    cervix            TEXT,
    ovary             TEXT,
    concluzion        TEXT,
    recommendation    TEXT,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho (id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_tableGestation1_reportEcho
ON tableGestation1(id_reportEcho);
