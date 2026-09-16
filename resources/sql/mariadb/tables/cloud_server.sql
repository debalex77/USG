CREATE TABLE IF NOT EXISTS `cloudServer` (
    `id`               BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_organizations` BIGINT UNSIGNED NOT NULL,
    `id_users`         BIGINT UNSIGNED NOT NULL,
    `hostName`         VARCHAR(30) NOT NULL,
    `databaseName`     VARCHAR(30) NOT NULL,
    `port`             VARCHAR(5),
    `connectionOption` VARCHAR(255),
    `username`         VARCHAR(50) NOT NULL,
    `password`         VARCHAR(255) NOT NULL,
    `iv`               VARCHAR(24) NOT NULL,
    PRIMARY KEY (`id`),
    KEY `idx_contsOnline_organizations` (`id_organizations`),
    KEY `idx_contsOnline_users` (`id_users`),
    CONSTRAINT `fk_cloudServer_organizations`
        FOREIGN KEY (`id_organizations`)
        REFERENCES `organizations` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_cloudServer_users`
        FOREIGN KEY (`id_users`)
        REFERENCES `users` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;
