-- =========================================
-- reportEcho
-- =========================================
CREATE TABLE IF NOT EXISTS reportEcho (
    id                INTEGER PRIMARY KEY AUTOINCREMENT,
    deletionMark      INTEGER NOT NULL DEFAULT 0,
    numberDoc         TEXT NOT NULL,
    dateDoc           TEXT NOT NULL,
    docYear           INTEGER GENERATED ALWAYS AS (
                          CAST(strftime('%Y', dateDoc) AS INTEGER)
                      ) VIRTUAL,
    patient_id       INTEGER NOT NULL,
    id_orderEcho      INTEGER NOT NULL,
    t_organs_internal INTEGER,
    t_urinary_system  INTEGER,
    t_prostate        INTEGER,
    t_gynecology      INTEGER,
    t_breast          INTEGER,
    t_thyroid         INTEGER,
    t_gestation0      INTEGER,
    t_gestation1      INTEGER,
    t_gestation2      INTEGER,
    t_gestation3      INTEGER,
    t_lymphNodes      INTEGER,
    id_users          INTEGER NOT NULL,
    concluzion        TEXT,
    comment           TEXT,
    attachedImages    INTEGER,
    uuid              BLOB NOT NULL,
    FOREIGN KEY (id_orderEcho)
        REFERENCES orderEcho(id)
        ON DELETE CASCADE,
    FOREIGN KEY (patient_id)
        REFERENCES patients(id)
        ON DELETE RESTRICT,
    FOREIGN KEY (id_users)
        REFERENCES users(id)
        ON DELETE RESTRICT
);

CREATE UNIQUE INDEX IF NOT EXISTS
    uq_reportEcho_uuid ON reportEcho(uuid);

CREATE UNIQUE INDEX IF NOT EXISTS
    uq_reportEcho_pacients_number
    ON reportEcho(patient_id, numberDoc, docYear);

CREATE INDEX IF NOT EXISTS
    idx_reportEcho_numberDoc_dateDoc
    ON reportEcho(numberDoc, dateDoc);

CREATE INDEX IF NOT EXISTS
    idx_reportEcho_pacients
    ON reportEcho(patient_id);

CREATE INDEX IF NOT EXISTS
    idx_reportEcho_order
    ON reportEcho(id_orderEcho);

CREATE INDEX IF NOT EXISTS
    idx_reportEcho_users
    ON reportEcho(id_users);

-- =========================================
-- reportEchoPresentation
-- =========================================
CREATE TABLE IF NOT EXISTS reportEchoPresentation (
    id                  INTEGER PRIMARY KEY AUTOINCREMENT,
    id_reportEcho        INTEGER NOT NULL,
    docPresentation     TEXT NOT NULL,
    docPresentationDate TEXT NOT NULL,

    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho(id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS
    idx_reportEchoPresentation_reportEcho
    ON reportEchoPresentation(id_reportEcho);
