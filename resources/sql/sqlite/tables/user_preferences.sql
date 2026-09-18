CREATE TABLE IF NOT EXISTS userPreferences (
    id                    INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    id_users              INT NOT NULL,
    versionApp            TEXT,
    showQuestionCloseApp  INT,
    showUserManual        INT,
    showHistoryVersion    INT,
    order_splitFullName   INT,
    updateListDoc         TEXT,
    showDesignerMenuPrint INT,
    checkNewVersionApp    INT,
    databasesArchiving    INT,
    showAsistantHelper    INT,
    showDocumentsInSeparatWindow INT,
    minimizeAppToTray     INT,
    FOREIGN KEY (id_users)
        REFERENCES users (id)
        ON DELETE CASCADE
);

CREATE UNIQUE INDEX IF NOT EXISTS uq_userPreferences_users
ON userPreferences(id_users);
