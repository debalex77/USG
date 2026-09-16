CREATE TABLE IF NOT EXISTS constants (
    id_users         INTEGER NOT NULL,
    id_organizations INTEGER,
    id_doctors       INTEGER,
    id_nurses        INTEGER,
    brandUSG         TEXT,
    logo             BLOB,
    PRIMARY KEY (id_users),
    FOREIGN KEY (id_users)
        REFERENCES users(id)
        ON DELETE CASCADE,
    FOREIGN KEY (id_organizations)
        REFERENCES organizations(id)
        ON DELETE SET NULL,
    FOREIGN KEY (id_doctors)
        REFERENCES doctors(id)
        ON DELETE SET NULL,
    FOREIGN KEY (id_nurses)
        REFERENCES nurses(id)
        ON DELETE SET NULL
);
