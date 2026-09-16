CREATE TABLE IF NOT EXISTS organizations (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    deletionMark  INTEGER NOT NULL DEFAULT 0,
    IDNP          TEXT NOT NULL,
    TVA           TEXT,
    name          TEXT NOT NULL,
    address       TEXT,
    telephone     TEXT,
    email         TEXT,
    comment       TEXT,
    id_contracts  INTEGER,
    stamp         BLOB,
    uuid          BLOB NOT NULL
);

CREATE UNIQUE INDEX IF NOT EXISTS
    uq_organizations_uuid ON organizations(uuid);

CREATE INDEX IF NOT EXISTS
    idx_organizations_idnp ON organizations(IDNP);

CREATE INDEX IF NOT EXISTS
    idx_organizations_name ON organizations(name);
