CREATE TABLE IF NOT EXISTS `patientAppointments` (
    `id`                 BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `dateDoc`            DATE NOT NULL,
    `time_registration`  INT,
    `execute`            BOOLEAN,
    `dataPatient`        VARCHAR (200) NOT NULL,
    `name_organizations` VARCHAR (150),
    `id_organizations`   BIGINT UNSIGNED,
    `name_doctors`       VARCHAR (150),
    `id_doctors`         BIGINT UNSIGNED,
    `investigations`     TEXT,
    `patient_id`         BIGINT UNSIGNED,
    `investigation_id`  BIGINT UNSIGNED,
    `comment`            VARCHAR (255),
    PRIMARY KEY (`id`),
    KEY `idx_patientAppointments_organizations` (`id_organizations`),
    KEY `idx_patientAppointments_doctors` (`id_doctors`),
    KEY `idx_patientAppointments_date` (`dateDoc`),
    KEY `idx_patientAppointments_patient_id` (`patient_id`),
    KEY `idx_patientAppointments_investigation_id` (`investigation_id`),
    CONSTRAINT `fk_patientAppointments_organizations`
        FOREIGN KEY (`id_organizations`)
        REFERENCES `organizations` (`id`)
        ON DELETE SET NULL
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_patientAppointments_doctors`
        FOREIGN KEY (`id_doctors`)
        REFERENCES `doctors` (`id`)
        ON DELETE SET NULL
        ON UPDATE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `patientAppointmentInvestigations` (
    `appointment_id`   BIGINT UNSIGNED NOT NULL,
    `investigation_id` BIGINT UNSIGNED NOT NULL,
    `position`         INT NOT NULL DEFAULT 0,
    PRIMARY KEY (`appointment_id`, `investigation_id`),
    KEY `idx_patientAppointmentInvestigations_investigation` (`investigation_id`),
    CONSTRAINT `fk_patientAppointmentInvestigations_appointment`
        FOREIGN KEY (`appointment_id`) REFERENCES `patientAppointments` (`id`) ON DELETE CASCADE,
    CONSTRAINT `fk_patientAppointmentInvestigations_investigation`
        FOREIGN KEY (`investigation_id`) REFERENCES `investigations` (`id`) ON DELETE RESTRICT
) ENGINE=InnoDB;
