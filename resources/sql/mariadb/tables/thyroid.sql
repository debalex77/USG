CREATE TABLE IF NOT EXISTS `tableThyroid` (
    `id`                   BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho`        BIGINT UNSIGNED NOT NULL,
    `thyroid_right_dimens` VARCHAR (20),
    `thyroid_right_volum`  VARCHAR (5),
    `thyroid_left_dimens`  VARCHAR (20),
    `thyroid_left_volum`   VARCHAR (5),
    `thyroid_istm`         VARCHAR (4),
    `thyroid_ecostructure` VARCHAR (20),
    `thyroid_formations`   VARCHAR (500),
    `thyroid_ganglions`    VARCHAR (300),
    `concluzion`           VARCHAR (500),
    `recommendation`       VARCHAR (255),
    PRIMARY KEY (`id`),
    KEY `idx_tableThyroid_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableThyroid_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;
