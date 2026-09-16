CREATE TABLE IF NOT EXISTS `tableProstate` (
    `id`             BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho`  BIGINT UNSIGNED NOT NULL,
    `dimens`         VARCHAR (25),
    `volume`         VARCHAR (5),
    `ecostructure`   VARCHAR (30),
    `contour`        VARCHAR (20),
    `ecogency`       VARCHAR (30),
    `formations`     VARCHAR (300),
    `transrectal`    BOOLEAN,
    `concluzion`     VARCHAR (500),
    `recommendation` VARCHAR (255),
    PRIMARY KEY (`id`),
    KEY `idx_tableProstate_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableProstate_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;
