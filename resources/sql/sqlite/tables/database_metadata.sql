CREATE TABLE IF NOT EXISTS databaseMetadata (
    singleton_id   INTEGER NOT NULL PRIMARY KEY CHECK (singleton_id = 1),
    schema_version TEXT NOT NULL
);
