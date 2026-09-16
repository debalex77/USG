CREATE TABLE IF NOT EXISTS tableSofTissuesLymphNodes (
    id             INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_reportEcho  INTEGER NOT NULL,
    section_type   TEXT NOT NULL CHECK (section_type IN ('soft_tissues','lymph_nodes','tissues_nodes')) DEFAULT 'lymph_nodes',
    examinedArea        TEXT,
    clinicalIndications TEXT,
    skin_structure         TEXT,
    subcutaneous_tissue    TEXT,
    lesion_location        TEXT,
    lesion_size            TEXT,
    lesion_echogenicity    TEXT,
    lesion_contour         TEXT,
    lesion_vascularization TEXT,
    ln_number              INTEGER,
    ln_size_nodes          TEXT,
    ln_shape               TEXT,
    ln_echogenic_hilum     TEXT,
    ln_cortex              TEXT,
    ln_structure           TEXT,
    ln_contour             TEXT,
    ln_vascularization     TEXT,
    ln_associated_changes  TEXT,
    other_changes          TEXT,
    concluzion             TEXT,
    recommendation         TEXT,
    FOREIGN KEY (id_reportEcho)
        REFERENCES reportEcho (id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_tableSofTissuesLymphNodes_reportEcho
ON tableSofTissuesLymphNodes(id_reportEcho);
