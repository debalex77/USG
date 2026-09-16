CREATE TABLE IF NOT EXISTS `onlineAccount` (
    `id`               BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_organizations` BIGINT UNSIGNED NOT NULL,
    `id_users`         BIGINT UNSIGNED NOT NULL,
    `email`            VARCHAR(255) NOT NULL,
    `smtp_server`      VARCHAR(255) NOT NULL,
    `port`             VARCHAR(5)  NOT NULL,
    `username`         VARCHAR(50) NOT NULL,
    `password`         VARCHAR(255) NOT NULL,
    `iv`               VARCHAR(24) NOT NULL,
    `tag`              VARCHAR(24) NOT NULL,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uq_onlineAccount_email` (`email`),
    KEY `idx_onlineAccount_organizations` (`id_organizations`),
    KEY `idx_onlineAccount_users` (`id_users`),
    CONSTRAINT `fk_contsOnline_organizations`
        FOREIGN KEY (`id_organizations`)
        REFERENCES `organizations` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_onlineAccount_users`
        FOREIGN KEY (`id_users`)
        REFERENCES `users` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;
