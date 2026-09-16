CREATE TABLE IF NOT EXISTS reportVideo (
    id            INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_orderEcho  INTEGER NOT NULL,
    id_reportEcho INTEGER NOT NULL,
    relative_path TEXT NOT NULL,
    file_name     TEXT NOT NULL,
    comment       TEXT,
    uuid          BLOB NOT NULL,
    FOREIGN KEY (id_orderEcho)
        REFERENCES orderEcho(id)
        ON DELETE CASCADE,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho(id)
        ON DELETE CASCADE
);

CREATE UNIQUE INDEX IF NOT EXISTS
    uq_reportVideo_uuid ON reportVideo(uuid);

CREATE INDEX IF NOT EXISTS
    idx_orderEcho_reportVideo
    ON reportVideo(id_orderEcho);

CREATE INDEX IF NOT EXISTS
    idx_reportEcho_reportVideo
    ON reportVideo(id_reportEcho);