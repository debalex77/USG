CREATE TABLE IF NOT EXISTS contracts (
    id               INTEGER PRIMARY KEY AUTOINCREMENT,
    deletionMark     INTEGER NOT NULL DEFAULT 0,
    id_organizations INTEGER NOT NULL,
    id_typesPrices   INTEGER,
    name             TEXT NOT NULL,
    dateInit         TEXT,
    notValid         INTEGER,
    comment          TEXT,
    uuid             BLOB NOT NULL,
    FOREIGN KEY (id_organizations)
        REFERENCES organizations(id)
        ON DELETE CASCADE,
    FOREIGN KEY (id_typesPrices)
        REFERENCES typesPrices(id)
        ON DELETE SET NULL
);

CREATE UNIQUE INDEX IF NOT EXISTS
    uq_contracts_uuid ON contracts(uuid);

CREATE INDEX IF NOT EXISTS
    idx_contracts_organizations ON contracts(id_organizations);

CREATE INDEX IF NOT EXISTS
    idx_contracts_typesPrices ON contracts(id_typesPrices);
