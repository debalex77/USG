CREATE TABLE IF NOT EXISTS `users` (
    `id`             BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `deletionMark`   TINYINT(1) NOT NULL DEFAULT 0,
    `name`           VARCHAR(50) NOT NULL,
    `password`       VARCHAR(50),
    `hash`           VARCHAR(255),
    `lastConnection` DATETIME,
    `uuid`           BINARY(16) NOT NULL,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uq_users_uuid` (`uuid`),
    UNIQUE KEY `uq_users_name` (`name`),
    KEY `idx_users_name` (`name`)
) ENGINE=InnoDB;
