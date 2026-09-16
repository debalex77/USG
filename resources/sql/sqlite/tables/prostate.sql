CREATE TABLE IF NOT EXISTS tableProstate (
    id             INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_reportEcho  INTEGER NOT NULL,
    dimens         TEXT,
    volume         TEXT,
    ecostructure   TEXT,
    contour        TEXT,
    ecogency       TEXT,
    formations     TEXT,
    transrectal    INTEGER NOT NULL,
    concluzion     TEXT,
    recommendation TEXT,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho (id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_tableProstate_reportEcho
ON tableProstate(id_reportEcho);
