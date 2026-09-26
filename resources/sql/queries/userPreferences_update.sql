UPDATE userPreferences SET
    versionApp            = ?,
    showQuestionCloseApp  = ?,
    showUserManual        = ?,
    showHistoryVersion    = ?,
    updateListDoc         = ?,
    showDesignerMenuPrint = ?,
    checkNewVersionApp    = ?,
    databasesArchiving    = ?,
    showAsistantHelper    = ?
WHERE
    id_users = ?
