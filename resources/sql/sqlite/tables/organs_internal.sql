-- ==================================================
-- tableLiver
-- ==================================================
CREATE TABLE IF NOT EXISTS tableLiver (
    id                INTEGER PRIMARY KEY AUTOINCREMENT,
    id_reportEcho     INTEGER NOT NULL,
    [left]            TEXT,
    [right]           TEXT,
    contur            TEXT,
    parenchim         TEXT,
    ecogenity         TEXT,
    formations        TEXT,
    ductsIntrahepatic TEXT,
    porta             TEXT,
    lienalis          TEXT,
    concluzion        TEXT,
    recommendation    TEXT,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho(id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_tableLiver_reportEcho
ON tableLiver(id_reportEcho);


-- ==================================================
-- tableCholecist
-- ==================================================
CREATE TABLE IF NOT EXISTS tableCholecist (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    id_reportEcho INTEGER NOT NULL,
    form          TEXT,
    dimens        TEXT,
    walls         TEXT,
    choledoc      TEXT,
    formations    TEXT,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho(id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS
    idx_tableCholecist_reportEcho
    ON tableCholecist(id_reportEcho);

-- ==================================================
-- tablePancreas
-- ==================================================
CREATE TABLE IF NOT EXISTS tablePancreas (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    id_reportEcho INTEGER NOT NULL,
    cefal         TEXT,
    corp          TEXT,
    tail          TEXT,
    texture       TEXT,
    ecogency      TEXT,
    formations    TEXT,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho(id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS
    idx_tablePancreas_reportEcho
    ON tablePancreas(id_reportEcho);

-- ==================================================
-- tableSpleen
-- ==================================================
CREATE TABLE IF NOT EXISTS tableSpleen (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    id_reportEcho INTEGER NOT NULL,
    dimens        TEXT,
    contur        TEXT,
    parenchim     TEXT,
    formations    TEXT,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho(id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS
    idx_tableSpleen_reportEcho
    ON tableSpleen(id_reportEcho);

-- ==================================================
-- tableIntestinalLoop
-- ==================================================
CREATE TABLE IF NOT EXISTS tableIntestinalLoop (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    id_reportEcho INTEGER NOT NULL,
    formations    TEXT,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho(id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS
    idx_tableIntestinalLoop_reportEcho
    ON tableIntestinalLoop(id_reportEcho);
