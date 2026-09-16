UPDATE userPreferences SET
    versionApp            = ?,
    showQuestionCloseApp  = ?,
    showUserManual        = ?,
    showHistoryVersion    = ?,
    order_splitFullName   = ?,
    updateListDoc         = ?,
    showDesignerMenuPrint = ?,
    checkNewVersionApp    = ?,
    databasesArchiving    = ?,
    showAsistantHelper    = ?
WHERE
    id_users = ?
