CREATE TABLE IF NOT EXISTS `userPreferences` (
    `id`                    BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_users`              BIGINT UNSIGNED NOT NULL,
    `versionApp`            VARCHAR (8),
    `showQuestionCloseApp`  BOOLEAN,
    `showUserManual`        BOOLEAN,
    `showHistoryVersion`    BOOLEAN,
    `order_splitFullName`   BOOLEAN,
    `updateListDoc`         VARCHAR (3),
    `showDesignerMenuPrint` BOOLEAN,
    `checkNewVersionApp`    BOOLEAN,
    `databasesArchiving`    BOOLEAN,
    `showAsistantHelper`    BOOLEAN,
    `showDocumentsInSeparatWindow` BOOLEAN,
    `minimizeAppToTray`     BOOLEAN,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uq_userPreferences_users` (`id_users`),
    CONSTRAINT `fk_userPreferences_users`
        FOREIGN KEY (`id_users`)
        REFERENCES `users` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;
