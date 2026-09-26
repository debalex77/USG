CREATE TABLE IF NOT EXISTS `userSettings` (
    `user_id`                            BIGINT UNSIGNED NOT NULL,
    `default_organization_id`            BIGINT UNSIGNED,
    `minimize_to_tray`                   BOOLEAN NOT NULL DEFAULT FALSE,
    `confirm_on_exit`                    BOOLEAN NOT NULL DEFAULT TRUE,
    `archive_sqlite_on_exit`             BOOLEAN NOT NULL DEFAULT FALSE,
    `open_documents_in_separate_windows` BOOLEAN NOT NULL DEFAULT FALSE,
    `print_menu_mode`                    TINYINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`user_id`),
    KEY `idx_userSettings_default_organization` (`default_organization_id`),
    CONSTRAINT `fk_userSettings_user`
        FOREIGN KEY (`user_id`)
        REFERENCES `users` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_userSettings_default_organization`
        FOREIGN KEY (`default_organization_id`)
        REFERENCES `organizations` (`id`)
        ON DELETE SET NULL
        ON UPDATE RESTRICT,
    CONSTRAINT `chk_userSettings_print_menu_mode`
        CHECK (`print_menu_mode` IN (0, 1))
) ENGINE=InnoDB;
