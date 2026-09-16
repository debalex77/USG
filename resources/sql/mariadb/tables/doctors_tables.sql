CREATE TABLE IF NOT EXISTS `doctors` (
    `id`           BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `deletionMark` TINYINT(1) NOT NULL DEFAULT 0,
    `name`         VARCHAR(80) NOT NULL,
    `fName`        VARCHAR(50),
    `mName`        VARCHAR(50),
    `telephone`    VARCHAR(100),
    `email`        VARCHAR(100),
    `comment`      VARCHAR(255),
    `signature`    LONGBLOB,
    `stamp`        LONGBLOB,
    `uuid`         BINARY(16) NOT NULL,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uq_doctors_uuid` (`uuid`),
    key `idx_doctors_name_fname` (`name`, `fName`)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `fullNameDoctors` (
    `id`              BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_doctors`      BIGINT UNSIGNED NOT NULL,
    `name`            VARCHAR(200) NOT NULL,
    `nameAbbreviated` VARCHAR(250),
    `nameTelephone`   VARCHAR(250),
    PRIMARY KEY (`id`),
    KEY `idx_fullNameDoctors_doctors` (`id_doctors`),
    CONSTRAINT `fk_fullNameDoctors_doctors`
        FOREIGN KEY (`id_doctors`)
        REFERENCES `doctors` (`id`)
        ON DELETE CASCADE
) ENGINE=InnoDB;
