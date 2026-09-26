CREATE TABLE IF NOT EXISTS `databaseMetadata` (
    `singleton_id`   TINYINT UNSIGNED NOT NULL,
    `schema_version` VARCHAR(32) NOT NULL,
    PRIMARY KEY (`singleton_id`),
    CONSTRAINT `chk_databaseMetadata_singleton`
        CHECK (`singleton_id` = 1)
) ENGINE=InnoDB;
