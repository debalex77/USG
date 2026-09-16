CREATE TABLE IF NOT EXISTS doctors (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    deletionMark INTEGER NOT NULL DEFAULT 0,
    name         TEXT NOT NULL,
    fName        TEXT,
    mName        TEXT,
    telephone    TEXT,
    email        TEXT,
    comment      TEXT,
    signature    BLOB,
    stamp        BLOB,
    uuid         BLOB NOT NULL
);

CREATE UNIQUE INDEX IF NOT EXISTS
    uq_doctors_uuid ON doctors(uuid);

CREATE INDEX IF NOT EXISTS
    idx_doctors_name_fname ON doctors(name, fName);

CREATE TABLE IF NOT EXISTS fullNameDoctors (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    id_doctors      INTEGER NOT NULL,
    name            TEXT NOT NULL,
    nameAbbreviated TEXT,
    nameTelephone   TEXT,
    FOREIGN KEY (id_doctors)
        REFERENCES doctors(id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS
    idx_fullNameDoctors_doctors ON fullNameDoctors(id_doctors);
