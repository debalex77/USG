CREATE TABLE IF NOT EXISTS settingsUsers (
    id                    INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_users              INT NOT NULL,
    owner                 TEXT,
    nameOption            TEXT,
    versionApp            TEXT,
    showQuestionCloseApp  INT,
    showUserManual        INT,
    showHistoryVersion    INT,
    order_splitFullName   INT,
    updateListDoc         TEXT,
    showDesignerMenuPrint INT,
    FOREIGN KEY (id_users)
        REFERENCES users (id)
        ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_settingsUsers_users
ON settingsUsers(id_users);
