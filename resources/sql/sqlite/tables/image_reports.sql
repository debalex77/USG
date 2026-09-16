CREATE TABLE IF NOT EXISTS imagesReports (
    id            INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_reportEcho INT NOT NULL,
    id_orderEcho  INT NOT NULL,
    patient_id   INT NOT NULL,
    image_1       BLOB,
    image_2       BLOB,
    image_3       BLOB,
    image_4       BLOB,
    image_5       BLOB,
    comment_1     TEXT,
    comment_2     TEXT,
    comment_3     TEXT,
    comment_4     TEXT,
    comment_5     TEXT,
    id_user       INT,
    uuid          BLOB NOT NULL
);

CREATE UNIQUE INDEX IF NOT EXISTS
    uq_imagesReports_uuid ON imagesReports(uuid);

CREATE INDEX IF NOT EXISTS
    idx_imagesReports_reportEcho ON imagesReports(id_reportEcho);

CREATE INDEX IF NOT EXISTS
    idx_imagesReports_orderEcho ON imagesReports(id_orderEcho);

CREATE INDEX IF NOT EXISTS
    idx_imagesReports_patients ON imagesReports(patient_id);

CREATE INDEX IF NOT EXISTS
    idx_imagesReports_report_patient ON imagesReports(id_reportEcho, patient_id);

CREATE INDEX IF NOT EXISTS
    idx_imagesReports_order_patient ON imagesReports(id_orderEcho, patient_id);

CREATE INDEX IF NOT EXISTS
    idx_imagesReports_order_report ON imagesReports(id_orderEcho, id_reportEcho);

CREATE INDEX IF NOT EXISTS
    idx_imagesReports_order_report_patient ON imagesReports(id_orderEcho, id_reportEcho, patient_id);
