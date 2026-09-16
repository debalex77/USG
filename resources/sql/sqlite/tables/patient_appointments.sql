CREATE TABLE IF NOT EXISTS patientAppointments (
    id                 INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    dateDoc            TEXT NOT NULL,
    time_registration  INTEGER,
    execute            INT,
    dataPatient        TEXT NOT NULL,
    name_organizations TEXT,
    id_organizations   INT,
    name_doctors       TEXT,
    id_doctors         INT,
    investigations     TEXT,
    patient_id         INTEGER,
    investigation_id  INTEGER,
    comment            TEXT,
    FOREIGN KEY (id_organizations)
        REFERENCES organizations (id)
        ON DELETE SET NULL,
    FOREIGN KEY (id_doctors)
        REFERENCES doctors (id)
        ON DELETE SET NULL
);

CREATE INDEX IF NOT EXISTS idx_patientAppointments_organizations
ON patientAppointments(id_organizations);

CREATE INDEX IF NOT EXISTS idx_patientAppointments_patient_text
ON patientAppointments(dataPatient);

CREATE INDEX IF NOT EXISTS idx_patientAppointments_date
ON patientAppointments(dateDoc);

CREATE INDEX IF NOT EXISTS idx_patientAppointments_patient_id
ON patientAppointments(patient_id);

CREATE INDEX IF NOT EXISTS idx_patientAppointments_investigation_id
ON patientAppointments(investigation_id);

CREATE TABLE IF NOT EXISTS patientAppointmentInvestigations (
    appointment_id  INTEGER NOT NULL,
    investigation_id INTEGER NOT NULL,
    position         INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY (appointment_id, investigation_id),
    FOREIGN KEY (appointment_id) REFERENCES patientAppointments(id) ON DELETE CASCADE,
    FOREIGN KEY (investigation_id) REFERENCES investigations(id) ON DELETE RESTRICT
);

CREATE INDEX IF NOT EXISTS idx_patientAppointmentInvestigations_investigation
ON patientAppointmentInvestigations(investigation_id);
