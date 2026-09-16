CREATE TABLE IF NOT EXISTS users (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,
    deletionMark   INTEGER NOT NULL DEFAULT 0,
    name           TEXT NOT NULL,
    password       TEXT,
    hash           TEXT,
    lastConnection TEXT,
    uuid           BLOB NOT NULL
);

CREATE UNIQUE INDEX IF NOT EXISTS
    uq_users_uuid ON users(uuid);

CREATE INDEX IF NOT EXISTS
    idx_users_name ON users(name);
