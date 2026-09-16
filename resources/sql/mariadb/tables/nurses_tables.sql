CREATE TABLE IF NOT EXISTS `nurses` (
    `id`           BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `deletionMark` TINYINT(1) NOT NULL DEFAULT 0,
    `name`         VARCHAR (80) NOT NULL,
    `fName`        VARCHAR (50),
    `mName`        VARCHAR (50),
    `telephone`    VARCHAR (100),
    `email`        VARCHAR (100),
    `comment`      VARCHAR (255),
    `uuid`         BINARY(16) NOT NULL,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uq_nurses_uuid` (`uuid`),
    KEY `idx_nurses_name_fname` (`name`, `fName`)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `fullNameNurses` (
    `id`               BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_nurses`        BIGINT UNSIGNED NOT NULL,
    `name`             varchar(200) NOT NULL,
    `nameAbbreviated`  varchar(250) DEFAULT NULL,
    `nameTelephone`    varchar(250) DEFAULT NULL,
    PRIMARY KEY (`id`),
    KEY `idx_fullNameNurses_nurses` (`id_nurses`),
    CONSTRAINT `fk_fullNameNurses_nurses`
        FOREIGN KEY (`id_nurses`)
        REFERENCES `nurses` (`id`)
        ON DELETE CASCADE
) ENGINE=InnoDB;
