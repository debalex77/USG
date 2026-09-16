CREATE TABLE IF NOT EXISTS onlineAccount (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_organizations INTEGER NOT NULL,
    id_users         INTEGER NOT NULL,
    email            TEXT NOT NULL UNIQUE,
    smtp_server      TEXT NOT NULL,
    port             TEXT NOT NULL,
    username         TEXT NOT NULL,
    password         TEXT NOT NULL,
    iv               TEXT NOT NULL,
    tag              TEXT NOT NULL,
    FOREIGN KEY (id_organizations)
        REFERENCES organizations (id)
        ON DELETE CASCADE,
    FOREIGN KEY (id_users)
        REFERENCES users (id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_onlineAccount_organizations
ON onlineAccount(id_organizations);

CREATE INDEX IF NOT EXISTS idx_onlineAccount_users
ON onlineAccount(id_users);
