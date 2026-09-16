-- =========================================
-- orderEcho
-- =========================================
CREATE TABLE IF NOT EXISTS orderEcho (
    id                 INTEGER PRIMARY KEY AUTOINCREMENT,
    deletionMark       INTEGER NOT NULL DEFAULT 0,
    numberDoc          TEXT,
    dateDoc            TEXT,
    docYear            INTEGER GENERATED ALWAYS AS (
                           CAST(strftime('%Y', dateDoc) AS INTEGER)
                       ) VIRTUAL,
    id_organizations   INTEGER,
    id_contracts       INTEGER,
    id_typesPrices     INTEGER,
    id_doctors         INTEGER,
    id_doctors_execute INTEGER,
    id_nurses          INTEGER,
    patient_id        INTEGER NOT NULL,
    id_users           INTEGER NOT NULL,
    sum                REAL DEFAULT 0.0,
    comment            TEXT,
    cardPayment        INTEGER,
    attachedImages     INTEGER,
    uuid               BLOB NOT NULL,
    FOREIGN KEY (id_organizations)
        REFERENCES organizations(id)
        ON DELETE SET NULL,
    FOREIGN KEY (id_contracts)
        REFERENCES contracts(id)
        ON DELETE SET NULL,
    FOREIGN KEY (id_typesPrices)
        REFERENCES typesPrices(id)
        ON DELETE SET NULL,
    FOREIGN KEY (id_doctors)
        REFERENCES doctors(id)
        ON DELETE SET NULL,
    FOREIGN KEY (id_doctors_execute)
        REFERENCES doctors(id)
        ON DELETE SET NULL,
    FOREIGN KEY (id_nurses)
        REFERENCES nurses(id)
        ON DELETE SET NULL,
    FOREIGN KEY (patient_id)
        REFERENCES patients(id)
        ON DELETE RESTRICT,
    FOREIGN KEY (id_users)
        REFERENCES users(id)
        ON DELETE RESTRICT
);

CREATE UNIQUE INDEX IF NOT EXISTS
    uq_orderEcho_uuid ON orderEcho(uuid);

CREATE UNIQUE INDEX IF NOT EXISTS
    uq_orderEcho_org_number
    ON orderEcho(id_organizations, numberDoc, docYear);

CREATE INDEX IF NOT EXISTS
    idx_orderEcho_active_date
    ON orderEcho(deletionMark, dateDoc);

CREATE INDEX IF NOT EXISTS
    idx_orderEcho_organizations_contracts
    ON orderEcho(id_organizations, id_contracts);

CREATE INDEX IF NOT EXISTS
    idx_orderEcho_doctors
    ON orderEcho(id_doctors);

CREATE INDEX IF NOT EXISTS
    idx_orderEcho_doctors_execute
    ON orderEcho(id_doctors_execute);

CREATE INDEX IF NOT EXISTS
    idx_orderEcho_nurses
    ON orderEcho(id_nurses);

CREATE INDEX IF NOT EXISTS
    idx_orderEcho_pacients
    ON orderEcho(patient_id);

CREATE INDEX IF NOT EXISTS
    idx_orderEcho_users
    ON orderEcho(id_users);

-- =========================================
-- orderEchoTable
-- =========================================
CREATE TABLE IF NOT EXISTS orderEchoTable (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    deletionMark INTEGER NOT NULL DEFAULT 0,
    id_orderEcho INTEGER NOT NULL,
    cod           TEXT NOT NULL,
    name          TEXT NOT NULL,
    price         REAL DEFAULT 0.0,
    FOREIGN KEY (id_orderEcho)
        REFERENCES orderEcho(id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS
    idx_orderEchoTable_orderEcho
    ON orderEchoTable(id_orderEcho);

CREATE INDEX IF NOT EXISTS
    idx_orderEchoTable_cod_name
    ON orderEchoTable(cod, name);

-- =========================================
-- orderEchoPresentation
-- =========================================
CREATE TABLE IF NOT EXISTS orderEchoPresentation (
    id                  INTEGER PRIMARY KEY AUTOINCREMENT,
    id_orderEcho        INTEGER NOT NULL,
    docPresentation     TEXT NOT NULL,
    docPresentationDate TEXT NOT NULL,
    FOREIGN KEY (id_orderEcho)
        REFERENCES orderEcho(id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS
    idx_orderEchoPresentation_orderEcho
    ON orderEchoPresentation(id_orderEcho);
