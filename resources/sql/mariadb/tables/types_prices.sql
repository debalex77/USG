CREATE TABLE IF NOT EXISTS `typesPrices` (
    `id`           BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `deletionMark` TINYINT(1) NOT NULL DEFAULT 0,
    `name`	   VARCHAR (50) NOT NULL,
    `discount`	   DECIMAL (15,3) DEFAULT NULL,
    `noncomercial` BOOLEAN,
    `uuid`         BINARY(16) NOT NULL,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uq_typesPrices_uuid` (`uuid`)
) ENGINE=InnoDB;
