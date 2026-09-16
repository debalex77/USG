UPDATE userPreferences SET
    versionApp = ?
WHERE
    id_users = ? AND
    versionApp IS NOT NULL
