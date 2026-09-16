CREATE TABLE IF NOT EXISTS `investigations` (
    `id`           BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `deletionMark` TINYINT(1) NOT NULL DEFAULT 0,
    `cod`          VARCHAR(10) NOT NULL,
    `name`         VARCHAR(500) NOT NULL,
    `use`          BOOLEAN NOT NULL,
    `owner`        BIGINT UNSIGNED,
    `uuid`         BINARY(16) NOT NULL,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uq_investigations_uuid` (`uuid`),
    KEY `idx_investigations_cod` (`cod`),
    KEY `idx_investigations_name` (`name`),
    KEY `idx_investigations_investigationsGroup` (`owner`),
    CONSTRAINT `fk_investigations_investigationsGroup`
        FOREIGN KEY (`owner`)
        REFERENCES `investigationsGroup` (`id`)
        ON DELETE SET NULL
        ON UPDATE RESTRICT
) ENGINE=InnoDB;
