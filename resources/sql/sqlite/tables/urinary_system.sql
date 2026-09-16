CREATE TABLE IF NOT EXISTS tableKidney (
    id                    INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_reportEcho         INTEGER NOT NULL,
    contour_right         TEXT CHECK(contour_right IN ('clar', 'sters', 'regulat', 'neregulat')) DEFAULT 'clar',
    contour_left          TEXT CHECK(contour_left IN ('clar', 'sters', 'regulat', 'neregulat')) DEFAULT 'clar',
    dimens_right          TEXT DEFAULT NULL,
    dimens_left           TEXT DEFAULT NULL,
    corticomed_right      TEXT DEFAULT NULL,
    corticomed_left       TEXT DEFAULT NULL,
    pielocaliceal_right   TEXT DEFAULT NULL,
    pielocaliceal_left    TEXT DEFAULT NULL,
    formations            TEXT DEFAULT NULL,
    suprarenal_formations TEXT DEFAULT NULL,
    concluzion            TEXT DEFAULT NULL,
    recommendation        TEXT DEFAULT NULL,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho(id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_tableKidney_reportEcho
ON tableKidney(id_reportEcho);

CREATE TABLE IF NOT EXISTS tableBladder (
    id            INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_reportEcho INTEGER NOT NULL,
    volum         TEXT,
    walls         TEXT,
    formations    TEXT,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho (id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_tableBladder_reportEcho
ON tableBladder(id_reportEcho);
