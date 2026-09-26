CREATE TABLE IF NOT EXISTS userSettings (
    user_id                            INTEGER NOT NULL PRIMARY KEY,
    default_organization_id            INTEGER,
    minimize_to_tray                   INTEGER NOT NULL DEFAULT 0,
    confirm_on_exit                    INTEGER NOT NULL DEFAULT 1,
    archive_sqlite_on_exit             INTEGER NOT NULL DEFAULT 0,
    open_documents_in_separate_windows INTEGER NOT NULL DEFAULT 0,
    print_menu_mode                    INTEGER NOT NULL DEFAULT 0,
    FOREIGN KEY (user_id)
        REFERENCES users(id)
        ON DELETE CASCADE,
    FOREIGN KEY (default_organization_id)
        REFERENCES organizations(id)
        ON DELETE SET NULL,
    CHECK (minimize_to_tray IN (0, 1)),
    CHECK (confirm_on_exit IN (0, 1)),
    CHECK (archive_sqlite_on_exit IN (0, 1)),
    CHECK (open_documents_in_separate_windows IN (0, 1)),
    CHECK (print_menu_mode IN (0, 1))
);
