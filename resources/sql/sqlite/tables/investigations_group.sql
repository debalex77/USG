CREATE TABLE IF NOT EXISTS investigationsGroup (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    deletionMark  INTEGER NOT NULL DEFAULT 0,
    cod           TEXT,
    name          TEXT,
    nameForPrint  TEXT,
    uuid          BLOB NOT NULL
);

CREATE UNIQUE INDEX IF NOT EXISTS
    uq_investigationsGroup_uuid ON investigationsGroup(uuid);
