CREATE TABLE IF NOT EXISTS `organizations` (
    `id`           BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `deletionMark` TINYINT(1) NOT NULL DEFAULT 0,
    `IDNP`         VARCHAR (15) NOT NULL,
    `TVA`          VARCHAR (10),
    `name`         VARCHAR (100) NOT NULL,
    `address`      VARCHAR (255),
    `telephone`    VARCHAR (100),
    `email`        VARCHAR (100),
    `comment`      VARCHAR (255),
    `id_contracts` BIGINT UNSIGNED,
    `stamp`        LONGBLOB,
    `uuid`         BINARY(16) NOT NULL,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uq_organizations_uuid` (`uuid`),
    KEY `idx_organizations_idnp` (`IDNP`),
    KEY `idx_organizations_name` (`name`)
) ENGINE=InnoDB;
