CREATE TABLE IF NOT EXISTS investigations (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    deletionMark  INTEGER NOT NULL DEFAULT 0,
    cod           TEXT NOT NULL,
    name          TEXT NOT NULL,
    "use"         INTEGER NOT NULL,
    owner         INTEGER,
    uuid          BLOB NOT NULL,
    FOREIGN KEY(owner) REFERENCES investigationsGroup(id)
);

CREATE UNIQUE INDEX IF NOT EXISTS
    uq_investigations_uuid ON investigations(uuid);

CREATE INDEX IF NOT EXISTS
    idx_investigations_cod ON investigations(cod);

CREATE INDEX IF NOT EXISTS
    idx_investigations_name ON investigations(name);

CREATE INDEX IF NOT EXISTS
    idx_investigations_owner ON investigations(owner);
