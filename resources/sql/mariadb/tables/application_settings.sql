CREATE TABLE IF NOT EXISTS `applicationSettings` (
    `user_id`                                   BIGINT UNSIGNED NOT NULL,
    `check_for_updates_on_startup`              BOOLEAN NOT NULL DEFAULT TRUE,
    `show_user_manual_on_startup`               BOOLEAN NOT NULL DEFAULT FALSE,
    `show_assistant_on_startup`                 BOOLEAN NOT NULL DEFAULT TRUE,
    `document_journal_refresh_interval_seconds` INT UNSIGNED NOT NULL DEFAULT 0,
    `synchronization_enabled`                   BOOLEAN NOT NULL DEFAULT TRUE,
    PRIMARY KEY (`user_id`),
    CONSTRAINT `fk_applicationSettings_user`
        FOREIGN KEY (`user_id`)
        REFERENCES `users` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;
