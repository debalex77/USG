CREATE TABLE IF NOT EXISTS tableThyroid (
    id                   INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_reportEcho        INTEGER NOT NULL,
    thyroid_right_dimens TEXT,
    thyroid_right_volum  TEXT,
    thyroid_left_dimens  TEXT,
    thyroid_left_volum   TEXT,
    thyroid_istm         TEXT,
    thyroid_ecostructure TEXT,
    thyroid_formations   TEXT,
    thyroid_ganglions    TEXT,
    concluzion           TEXT,
    recommendation       TEXT,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho (id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_tableThyroid_reportEcho
ON tableThyroid(id_reportEcho);
