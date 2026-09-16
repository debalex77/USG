CREATE TABLE IF NOT EXISTS `investigationsGroup` (
    `id`           BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `deletionMark` TINYINT(1) NOT NULL DEFAULT 0,
    `cod`          VARCHAR (10),
    `name`         VARCHAR (50),
    `nameForPrint` VARCHAR (250),
    `uuid`         BINARY(16) NOT NULL,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uq_investigationsGroup_uuid` (`uuid`)
) ENGINE=InnoDB;
