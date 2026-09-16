CREATE TABLE IF NOT EXISTS nurses (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    deletionMark INTEGER NOT NULL DEFAULT 0,
    name         TEXT NOT NULL,
    fName        TEXT,
    mName        TEXT,
    telephone    TEXT,
    email        TEXT,
    comment      TEXT,
    uuid         BLOB NOT NULL
);

CREATE UNIQUE INDEX IF NOT EXISTS
    uq_nurses_uuid ON nurses(uuid);

CREATE INDEX IF NOT EXISTS
    idx_nurses_name_fname ON nurses(name, fName);

CREATE TABLE IF NOT EXISTS fullNameNurses (
    id               INTEGER PRIMARY KEY AUTOINCREMENT,
    id_nurses        INTEGER NOT NULL,
    name             TEXT NOT NULL,
    nameAbbreviated  TEXT DEFAULT NULL,
    nameTelephone    TEXT DEFAULT NULL,
    FOREIGN KEY (id_nurses)
        REFERENCES nurses(id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS
    idx_fullNameNurses_nurses ON fullNameNurses(id_nurses);
