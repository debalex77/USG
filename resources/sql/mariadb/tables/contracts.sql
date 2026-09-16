CREATE TABLE IF NOT EXISTS `contracts` (
    `id`               BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `deletionMark`     TINYINT(1) NOT NULL DEFAULT 0,
    `id_organizations` BIGINT UNSIGNED NOT NULL,
    `id_typesPrices`   BIGINT UNSIGNED,
    `name`             VARCHAR (50) NOT NULL,
    `dateInit`         DATE,
    `notValid`         BOOLEAN,
    `comment`          VARCHAR (255),
    `uuid`             BINARY(16) NOT NULL,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uq_contracts_uuid` (`uuid`),
    KEY `idx_contracts_organizations` (`id_organizations`),
    KEY `idx_contracts_typesPrices` (`id_typesPrices`),
    CONSTRAINT `fk_contracts_organizations`
        FOREIGN KEY (`id_organizations`)
        REFERENCES `organizations` (`id`)
        ON DELETE CASCADE,
    CONSTRAINT `fk_contracts_typesPrices`
        FOREIGN KEY (`id_typesPrices`)
        REFERENCES `typesPrices` (`id`)
        ON DELETE SET NULL
) ENGINE=InnoDB;
-- =========================================
-- Adaugam indexul in organizatie
-- =========================================
ALTER TABLE `organizations`
    ADD INDEX IF NOT EXISTS `idx_organizations_contracts` (`id_contracts` ASC) VISIBLE;
ALTER TABLE `organizations`
    ADD CONSTRAINT `fk_organizations_contracts`
    FOREIGN KEY IF NOT EXISTS (`id_contracts`)
    REFERENCES `contracts` (`id`)
    ON DELETE SET NULL;
