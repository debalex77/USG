CREATE TABLE IF NOT EXISTS applicationSettings (
    user_id                                   INTEGER NOT NULL PRIMARY KEY,
    check_for_updates_on_startup              INTEGER NOT NULL DEFAULT 1,
    show_user_manual_on_startup               INTEGER NOT NULL DEFAULT 0,
    show_assistant_on_startup                 INTEGER NOT NULL DEFAULT 1,
    document_journal_refresh_interval_seconds INTEGER NOT NULL DEFAULT 0,
    synchronization_enabled                   INTEGER NOT NULL DEFAULT 1,
    FOREIGN KEY (user_id)
        REFERENCES users(id)
        ON DELETE CASCADE,
    CHECK (check_for_updates_on_startup IN (0, 1)),
    CHECK (show_user_manual_on_startup IN (0, 1)),
    CHECK (show_assistant_on_startup IN (0, 1)),
    CHECK (synchronization_enabled IN (0, 1)),
    CHECK (document_journal_refresh_interval_seconds >= 0)
);
