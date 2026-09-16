CREATE TABLE IF NOT EXISTS cloudServer (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_organizations INTEGER NOT NULL,
    id_users         INTEGER NOT NULL,
    hostName         TEXT NOT NULL,
    databaseName     TEXT NOT NULL,
    port             TEXT,
    connectionOption TEXT,
    username         TEXT NOT NULL,
    password         TEXT NOT NULL,
    iv               TEXT NOT NULL,
    FOREIGN KEY (id_organizations)
        REFERENCES organizations (id)
        ON DELETE CASCADE,
    FOREIGN KEY (id_users)
        REFERENCES users (id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_cloudServer_organizations
ON cloudServer(id_organizations);

CREATE INDEX IF NOT EXISTS idx_cloudServer_users
ON cloudServer(id_users);
