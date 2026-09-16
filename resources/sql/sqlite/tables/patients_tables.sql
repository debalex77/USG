CREATE TABLE IF NOT EXISTS patients (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,
    deletion_mark  INTEGER NOT NULL DEFAULT 0,
    idnp           TEXT,
    last_name      TEXT NOT NULL,
    first_name     TEXT,
    middle_name    TEXT,
    medical_policy TEXT,
    birthday       TEXT NOT NULL,
    address        TEXT,
    telephone      TEXT,
    email          TEXT,
    comment        TEXT,
    uuid           BLOB NOT NULL
);

CREATE UNIQUE INDEX IF NOT EXISTS
    uq_patients_uuid ON patients(uuid);

CREATE INDEX IF NOT EXISTS
    idx_patients_idnp ON patients(idnp);

CREATE INDEX IF NOT EXISTS
    idx_patients_medical_policy ON patients(medical_policy);

CREATE INDEX IF NOT EXISTS
    idx_patients_name ON patients(last_name, first_name);

CREATE INDEX IF NOT EXISTS
    idx_patients_name_idnp ON patients(last_name, first_name, idnp);

CREATE INDEX IF NOT EXISTS
    idx_patients_telephone ON patients(telephone);
