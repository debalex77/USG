CREATE TABLE IF NOT EXISTS conclusionTemplates (
    id           INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    deletionMark INT NOT NULL,
    cod          TEXT NOT NULL,
    name         TEXT NOT NULL,
    system       TEXT,
    uuid         BLOB NOT NULL
);

CREATE UNIQUE INDEX IF NOT EXISTS
    uq_conclusionTemplates_uuid ON conclusionTemplates(uuid);

CREATE TABLE IF NOT EXISTS formationsSystemTemplates (
    id           INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    deletionMark INT NOT NULL,
    name         TEXT NOT NULL,
    typeSystem   TEXT CHECK(typeSystem IN ('Unknow', 'Ficat', 'Colecist', 'Pancreas', 'Splina', 'Intestine', 'Recomandari (org.interne)',
                       'Rinichi', 'V.urinara', 'Gl.suprarenale', 'Recomandari (s.urinar)',
                       'Prostata', 'Recomandari (prostata)',
                       'Tiroida', 'Recomandari (tiroida)',
                       'Gl.mamara (stanga)', 'Gl.mamara (dreapta)', 'Recomandari (gl.mamare)',
                       'Ginecologia (uter)', 'Ginecologia (ovar stang)', 'Ginecologia (ovar drept)', 'Recomandari (ginecologia)',
                       'Recomandari (gestatation0)', 'Recomandari (gestatation1)', 'Recomandari (gestatation2)',
                       'Recomandari (gangl.limfatici)')) DEFAULT 'Unknow',
    uuid         BLOB NOT NULL
);

CREATE UNIQUE INDEX IF NOT EXISTS
    uq_formationsSystemTemplates_uuid ON formationsSystemTemplates(uuid);
