CREATE TABLE IF NOT EXISTS `cryptoSplitKey` (
    `id_organizations` BIGINT UNSIGNED NOT NULL,
    `key_part1`        VARCHAR(128) NOT NULL,
    PRIMARY KEY (`id_organizations`),
    CONSTRAINT `fk_cryptoSplitKey_organizations`
        FOREIGN KEY (`id_organizations`)
        REFERENCES `organizations` (`id`)
        ON DELETE CASCADE
        ON UPDATE CASCADE
) ENGINE=InnoDB;
