CREATE TABLE IF NOT EXISTS typesPrices (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    deletionMark  INTEGER NOT NULL DEFAULT 0,
    name          TEXT NOT NULL,
    discount      REAL,
    noncomercial  INTEGER,
    uuid          BLOB NOT NULL
);

CREATE UNIQUE INDEX IF NOT EXISTS
    uq_typesPrices_uuid ON typesPrices(uuid);
