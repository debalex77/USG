CREATE TABLE IF NOT EXISTS doc_sequences (
    name TEXT NOT NULL,
    year INTEGER NOT NULL,
    value INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY (name, year)
);
