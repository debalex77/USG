CREATE VIEW IF NOT EXISTS v_users_combo_active AS
SELECT
    id,
    name
FROM
    users
WHERE
    deletionMark = 0
