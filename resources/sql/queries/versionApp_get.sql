SELECT
    versionApp
FROM
    userPreferences
WHERE
    id_users = ? AND
    versionApp IS NOT NULL
