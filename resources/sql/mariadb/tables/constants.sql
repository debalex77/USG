CREATE TABLE IF NOT EXISTS `constants` (
    `id_users`	       BIGINT UNSIGNED NOT NULL,
    `id_organizations` BIGINT UNSIGNED,
    `id_doctors`       BIGINT UNSIGNED,
    `id_nurses`	       BIGINT UNSIGNED,
    `brandUSG`	       VARCHAR (200),
    `logo`             LONGBLOB,
    KEY `idx_constants_users` (`id_users`),
    KEY `idx_constants_organizations` (`id_organizations`),
    KEY `idx_constants_doctors` (`id_doctors`),
    KEY `idx_constants_nurses` (`id_nurses`),
    CONSTRAINT `fk_constants_doctors`
        FOREIGN KEY (`id_doctors`)
        REFERENCES `doctors` (`id`)
        ON DELETE SET NULL
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_constants_nurses`
        FOREIGN KEY (`id_nurses`)
        REFERENCES `nurses` (`id`)
        ON DELETE SET NULL
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_constants_organizations`
        FOREIGN KEY (`id_organizations`)
        REFERENCES `organizations` (`id`)
        ON DELETE SET NULL
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_constants_users`
        FOREIGN KEY (`id_users`)
        REFERENCES `users` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;
