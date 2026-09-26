CREATE TABLE IF NOT EXISTS organizationSettings (
    organization_id        INTEGER NOT NULL PRIMARY KEY,
    default_doctor_id      INTEGER,
    default_nurse_id       INTEGER,
    ultrasound_device_name TEXT,
    logo                    BLOB,
    FOREIGN KEY (organization_id)
        REFERENCES organizations(id)
        ON DELETE CASCADE,
    FOREIGN KEY (default_doctor_id)
        REFERENCES doctors(id)
        ON DELETE SET NULL,
    FOREIGN KEY (default_nurse_id)
        REFERENCES nurses(id)
        ON DELETE SET NULL
);
