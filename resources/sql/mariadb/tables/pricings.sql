CREATE TABLE IF NOT EXISTS `pricings` (
    `id`               BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `deletionMark`     TINYINT(1) NOT NULL DEFAULT 0,
    `numberDoc`        VARCHAR(15) NOT NULL,
    `dateDoc`          DATETIME NOT NULL,
    `id_typesPrices`   BIGINT UNSIGNED NOT NULL,
    `id_organizations` BIGINT UNSIGNED NOT NULL,
    `id_contracts`     BIGINT UNSIGNED NOT NULL,
    `id_users`         BIGINT UNSIGNED NOT NULL,
    `comment`          VARCHAR(255) DEFAULT NULL,
    `uuid`             BINARY(16) NOT NULL,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uq_pricings_uuid` (`uuid`),
    UNIQUE KEY `uq_pricings_org_number` (`id_organizations`, `numberDoc`),
    KEY `idx_pricings_active_date` (`deletionMark`, `dateDoc`),
    KEY `idx_pricings_typesPrices` (`id_typesPrices`),
    KEY `idx_pricings_organizations` (`id_organizations`),
    KEY `idx_pricings_contracts` (`id_contracts`),
    KEY `idx_pricings_users` (`id_users`),
    CONSTRAINT `fk_pricings_typesPrices`
        FOREIGN KEY (`id_typesPrices`)
        REFERENCES `typesPrices` (`id`)
        ON DELETE RESTRICT
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_pricings_organizations`
        FOREIGN KEY (`id_organizations`)
        REFERENCES `organizations` (`id`)
        ON DELETE RESTRICT
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_pricings_contracts`
        FOREIGN KEY (`id_contracts`)
        REFERENCES `contracts` (`id`)
        ON DELETE RESTRICT
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_pricings_users`
        FOREIGN KEY (`id_users`)
        REFERENCES `users` (`id`)
        ON DELETE RESTRICT
        ON UPDATE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `pricingsTable` (
    `id`           BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `deletionMark` TINYINT(1) NOT NULL DEFAULT 0,
    `id_pricings`  BIGINT UNSIGNED NOT NULL,
    `cod`	   VARCHAR (10) NOT NULL,
    `name`	   VARCHAR (500) NOT NULL,
    `price`	   DECIMAL (15,3) DEFAULT '0.00',
    PRIMARY KEY (`id`),
    KEY `idx_pricingsTable_pricings` (`id_pricings`),
    KEY `idx_pricingsTable_cod_name` (`cod`, `name`),
    KEY `idx_pricingsTable_cod` (`cod`),
    KEY `idx_pricingsTable_name` (`name`),
    CONSTRAINT `fk_pricingsTable_pricings`
        FOREIGN KEY (`id_pricings`)
        REFERENCES `pricings` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `pricingsPresentation` (
    `id`                  BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_pricings`         BIGINT UNSIGNED NOT NULL,
    `docPresentation`     VARCHAR (250) NOT NULL,
    `docPresentationDate` VARCHAR (250) NOT NULL,
    PRIMARY KEY (`id`),
    KEY `idx_pricingsPresentation_pricings` (`id_pricings`),
    CONSTRAINT `fk_pricingsPresentation_pricings`
        FOREIGN KEY (`id_pricings`)
        REFERENCES `pricings` (`id`)
        ON DELETE CASCADE
) ENGINE=InnoDB;
