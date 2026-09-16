CREATE TABLE IF NOT EXISTS cryptoSplitKey (
    id_organizations INTEGER PRIMARY KEY,
    key_part1 TEXT NOT NULL,
    CONSTRAINT fk_cryptoSplitKey_organizations
    FOREIGN KEY(id_organizations)
    REFERENCES organizations (id)
    ON DELETE CASCADE
    ON UPDATE CASCADE
);
