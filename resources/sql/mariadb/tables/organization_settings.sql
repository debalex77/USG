CREATE TABLE IF NOT EXISTS `organizationSettings` (
    `organization_id`        BIGINT UNSIGNED NOT NULL,
    `default_doctor_id`      BIGINT UNSIGNED,
    `default_nurse_id`       BIGINT UNSIGNED,
    `ultrasound_device_name` VARCHAR(200),
    `logo`                   LONGBLOB,
    PRIMARY KEY (`organization_id`),
    KEY `idx_organizationSettings_doctor` (`default_doctor_id`),
    KEY `idx_organizationSettings_nurse` (`default_nurse_id`),
    CONSTRAINT `fk_organizationSettings_organization`
        FOREIGN KEY (`organization_id`)
        REFERENCES `organizations` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_organizationSettings_doctor`
        FOREIGN KEY (`default_doctor_id`)
        REFERENCES `doctors` (`id`)
        ON DELETE SET NULL
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_organizationSettings_nurse`
        FOREIGN KEY (`default_nurse_id`)
        REFERENCES `nurses` (`id`)
        ON DELETE SET NULL
        ON UPDATE RESTRICT
) ENGINE=InnoDB;
