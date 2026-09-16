CREATE TABLE IF NOT EXISTS tableGestation0 (
    id               INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_reportEcho    INTEGER NOT NULL,
    view_examination INTEGER,
    antecedent       TEXT,
    lmp              TEXT,
    gestation_age    TEXT,
    GS               TEXT,
    GS_age           TEXT,
    CRL              TEXT,
    CRL_age          TEXT,
    BCF              TEXT,
    liquid_amniotic  TEXT,
    miometer         TEXT,
    cervix           TEXT,
    ovary            TEXT,
    concluzion       TEXT,
    recommendation   TEXT,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho (id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_tableGestation0_reportEcho
ON tableGestation0(id_reportEcho);
