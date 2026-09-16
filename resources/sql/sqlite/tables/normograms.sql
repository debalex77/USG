CREATE TABLE IF NOT EXISTS normograms (
    id           INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    name         TEXT NOT NULL,
    crl          TEXT NOT NULL,
    `5_centile`  REAL NOT NULL,
    `50_centile` REAL NOT NULL,
    `95_centile` REAL NOT NULL
);
