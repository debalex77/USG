CREATE TABLE IF NOT EXISTS `settingsUsers` (
    `id`                    BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_users`              BIGINT UNSIGNED NOT NULL,
    `owner`                 VARCHAR (50),
    `nameOption`            VARCHAR (150),
    `versionApp`            VARCHAR (8),
    `showQuestionCloseApp`  BOOLEAN,
    `showUserManual`        BOOLEAN,
    `showHistoryVersion`    BOOLEAN,
    `order_splitFullName`   BOOLEAN,
    `updateListDoc`         VARCHAR (3),
    `showDesignerMenuPrint` BOOLEAN,
    PRIMARY KEY (`id`),
    KEY `idx_settingsUsers_users` (`id_users`),
    CONSTRAINT `fk_settingsUsers_users`
        FOREIGN KEY (`id_users`)
        REFERENCES `users` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;
