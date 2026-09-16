CREATE TABLE IF NOT EXISTS `reportEcho` (
    `id`                BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `deletionMark`      TINYINT(1) NOT NULL DEFAULT 0,
    `numberDoc`         VARCHAR (15) NOT NULL,
    `dateDoc`           DATETIME NOT NULL,
    `docYear`           SMALLINT GENERATED ALWAYS AS (YEAR(`dateDoc`)) STORED,
    `patient_id`       BIGINT UNSIGNED NOT NULL,
    `id_orderEcho`      BIGINT UNSIGNED NOT NULL,
    `t_organs_internal` BOOLEAN,
    `t_urinary_system`  BOOLEAN,
    `t_prostate`        BOOLEAN,
    `t_gynecology`      BOOLEAN,
    `t_breast`          BOOLEAN,
    `t_thyroid`         BOOLEAN,
    `t_gestation0`      BOOLEAN,
    `t_gestation1`      BOOLEAN,
    `t_gestation2`      BOOLEAN,
    `t_gestation3`      BOOLEAN,
    `t_lymphNodes`      BOOLEAN,
    `id_users`          BIGINT UNSIGNED NOT NULL,
    `concluzion`        VARCHAR (700),
    `comment`           VARCHAR (255),
    `attachedImages`    INT,
    `uuid`              BINARY(16) NOT NULL,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uq_reportEcho_uuid` (`uuid`),
    UNIQUE KEY `uq_reportEcho_pacients_number` (`patient_id`, `numberDoc`, `docYear`),
    KEY `idx_reportEcho_numberDoc_dateDoc` (`numberDoc`, `dateDoc`),
    KEY `idx_reportEcho_pacients` (`patient_id`),
    KEY `idx_reportEcho_order` (`id_orderEcho`),
    KEY `idx_reportEcho_users` (`id_users`),
    CONSTRAINT `fk_reportEcho_orderEcho`
        FOREIGN KEY (`id_orderEcho`)
        REFERENCES `orderEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_reportEcho_pacients`
        FOREIGN KEY (`patient_id`)
        REFERENCES `patients` (`id`)
        ON DELETE RESTRICT
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_reportEcho_users`
        FOREIGN KEY (`id_users`)
        REFERENCES `users` (`id`)
        ON DELETE RESTRICT
        ON UPDATE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `reportEchoPresentation` (
    `id`                  BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho`       BIGINT UNSIGNED NOT NULL,
    `docPresentation`     VARCHAR (250) NOT NULL,
    `docPresentationDate` VARCHAR (250) NOT NULL,
    PRIMARY KEY (`id`),
    KEY `idx_reportEchoPresentation_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_reportEchoPresentation_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;
