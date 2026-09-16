CREATE TABLE IF NOT EXISTS `orderEcho` (
    `id`                 BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `deletionMark`       TINYINT(1) NOT NULL DEFAULT 0,
    `numberDoc`          VARCHAR (15),
    `dateDoc`            DATETIME,
    `docYear`            SMALLINT GENERATED ALWAYS AS (YEAR(`dateDoc`)) STORED,
    `id_organizations`   BIGINT UNSIGNED NOT NULL,
    `id_contracts`       BIGINT UNSIGNED NOT NULL,
    `id_typesPrices`     BIGINT UNSIGNED NOT NULL,
    `id_doctors`         BIGINT UNSIGNED,
    `id_doctors_execute` BIGINT UNSIGNED,
    `id_nurses`          BIGINT UNSIGNED,
    `patient_id`        BIGINT UNSIGNED NOT NULL,
    `id_users`           BIGINT UNSIGNED NOT NULL,
    `sum`                DECIMAL (15,2) DEFAULT '0.00',
    `comment`            VARCHAR (255),
    `cardPayment`        INT,
    `attachedImages`     INT,
    `uuid`               BINARY(16) NOT NULL,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uq_orderEcho_uuid` (`uuid`),
    UNIQUE KEY `uq_orderEcho_org_number` (`id_organizations`, `numberDoc`, `docYear`),
    KEY `idx_orderEcho_active_date` (`deletionMark`, `dateDoc`),
    KEY `idx_orderEcho_organizations_contracts` (`id_organizations`, `id_contracts`),
    KEY `idx_orderEcho_doctors` (`id_doctors`),
    KEY `idx_orderEcho_doctors_execute` (`id_doctors_execute`),
    KEY `idx_orderEcho_nurses` (`id_nurses`),
    KEY `idx_orderEcho_pacients` (`patient_id`),
    KEY `idx_orderEcho_users` (`id_users`),
    CONSTRAINT `fk_orderEcho_organizations`
        FOREIGN KEY (`id_organizations`)
        REFERENCES `organizations` (`id`)
        ON DELETE RESTRICT
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_orderEcho_contracts`
        FOREIGN KEY (`id_contracts`)
        REFERENCES `contracts` (`id`)
        ON DELETE RESTRICT
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_orderEcho_typesPrices`
        FOREIGN KEY (`id_typesPrices`)
        REFERENCES `typesPrices` (`id`)
        ON DELETE RESTRICT
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_orderEcho_doctors`
        FOREIGN KEY (`id_doctors`)
        REFERENCES `doctors` (`id`)
        ON DELETE SET NULL
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_orderEcho_doctors_execute`
        FOREIGN KEY (`id_doctors_execute`)
        REFERENCES `doctors` (`id`)
        ON DELETE SET NULL
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_orderEcho_nurses`
        FOREIGN KEY (`id_nurses`)
        REFERENCES `nurses` (`id`)
        ON DELETE SET NULL
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_orderEcho_pacients`
        FOREIGN KEY (`patient_id`)
        REFERENCES `patients` (`id`)
        ON DELETE RESTRICT
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_orderEcho_users`
        FOREIGN KEY (`id_users`)
        REFERENCES `users` (`id`)
        ON DELETE RESTRICT
        ON UPDATE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `orderEchoTable` (
    `id`           BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `deletionMark` TINYINT(1) NOT NULL DEFAULT 0,
    `id_orderEcho` BIGINT UNSIGNED NOT NULL,
    `cod`          VARCHAR (10) NOT NULL,
    `name`         VARCHAR (500) NOT NULL,
    `price`        DECIMAL (15,3) DEFAULT '0.00',
    PRIMARY KEY (`id`),
    KEY `idx_orderEchoTable_orderEcho` (`id_orderEcho`),
    KEY `idx_orderEchoTable_cod_name` (`cod`, `name`),
    CONSTRAINT `fk_orderEchoTable_orderEcho`
        FOREIGN KEY (`id_orderEcho`)
        REFERENCES `orderEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `orderEchoPresentation` (
    `id`                  BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_orderEcho`        BIGINT UNSIGNED NOT Null,
    `docPresentation`     VARCHAR (250) NOT Null,
    `docPresentationDate` VARCHAR (250) NOT Null,
    PRIMARY KEY (`id`),
    KEY `idx_orderEchoPresentation_orderEcho` (`id_orderEcho`),
    CONSTRAINT `fk_orderEchoPresentation_orderEcho`
        FOREIGN KEY (`id_orderEcho`)
        REFERENCES `orderEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;
