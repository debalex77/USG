CREATE TABLE IF NOT EXISTS tableBreast (
    id                       INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_reportEcho            INTEGER NOT NULL,
    breast_right_ecostrcture TEXT,
    breast_right_duct        TEXT,
    breast_right_ligament    TEXT,
    breast_right_formations  TEXT,
    breast_right_ganglions   TEXT,
    breast_left_ecostrcture  TEXT,
    breast_left_duct         TEXT,
    breast_left_ligament     TEXT,
    breast_left_formations   TEXT,
    breast_left_ganglions    TEXT,
    concluzion               TEXT,
    recommendation           TEXT,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho (id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_tableBreast_reportEcho
ON tableBreast(id_reportEcho);
