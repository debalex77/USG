-- =========================================
-- pricings
-- =========================================
CREATE TABLE IF NOT EXISTS pricings (
    id               INTEGER PRIMARY KEY AUTOINCREMENT,
    deletionMark     INTEGER NOT NULL DEFAULT 0,
    numberDoc        TEXT NOT NULL,
    dateDoc          TEXT NOT NULL,
    id_typesPrices   INTEGER,
    id_organizations INTEGER,
    id_contracts     INTEGER,
    id_users         INTEGER,
    comment          TEXT,
    uuid             BLOB NOT NULL,
    FOREIGN KEY (id_typesPrices)
        REFERENCES typesPrices(id)
        ON DELETE SET NULL,
    FOREIGN KEY (id_organizations)
        REFERENCES organizations(id)
        ON DELETE SET NULL,
    FOREIGN KEY (id_contracts)
        REFERENCES contracts(id)
        ON DELETE SET NULL,
    FOREIGN KEY (id_users)
        REFERENCES users(id)
        ON DELETE SET NULL
);

CREATE UNIQUE INDEX IF NOT EXISTS
    uq_pricings_uuid ON pricings(uuid);

CREATE UNIQUE INDEX IF NOT EXISTS
    uq_pricings_org_number
    ON pricings(id_organizations, numberDoc);

CREATE INDEX IF NOT EXISTS
    idx_pricings_active_date
    ON pricings(deletionMark, dateDoc);

CREATE INDEX IF NOT EXISTS
    idx_pricings_typesPrices
    ON pricings(id_typesPrices);

CREATE INDEX IF NOT EXISTS
    idx_pricings_organizations
    ON pricings(id_organizations);

CREATE INDEX IF NOT EXISTS
    idx_pricings_contracts
    ON pricings(id_contracts);

CREATE INDEX IF NOT EXISTS
    idx_pricings_users
    ON pricings(id_users);

-- =========================================
-- pricingsTable
-- =========================================
CREATE TABLE IF NOT EXISTS pricingsTable (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    deletionMark INTEGER NOT NULL DEFAULT 0,
    id_pricings  INTEGER NOT NULL,
    cod           TEXT NOT NULL,
    name          TEXT NOT NULL,
    price         REAL DEFAULT 0.0,
    FOREIGN KEY (id_pricings)
        REFERENCES pricings(id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS
    idx_pricingsTable_pricings
    ON pricingsTable(id_pricings);

CREATE INDEX IF NOT EXISTS
    idx_pricingsTable_cod_name
    ON pricingsTable(cod, name);

CREATE INDEX IF NOT EXISTS
    idx_pricingsTable_cod
    ON pricingsTable(cod);

CREATE INDEX IF NOT EXISTS
    idx_pricingsTable_name
    ON pricingsTable(name);

-- =========================================
-- pricingsPresentation
-- =========================================
CREATE TABLE IF NOT EXISTS pricingsPresentation (
    id                  INTEGER PRIMARY KEY AUTOINCREMENT,
    id_pricings         INTEGER NOT NULL,
    docPresentation     TEXT NOT NULL,
    docPresentationDate TEXT NOT NULL,
    FOREIGN KEY (id_pricings)
        REFERENCES pricings(id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS
    idx_pricingsPresentation_pricings
    ON pricingsPresentation(id_pricings);
