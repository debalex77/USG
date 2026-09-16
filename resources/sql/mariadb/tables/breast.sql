CREATE TABLE IF NOT EXISTS `tableBreast` (
    `id`                       BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho`            BIGINT UNSIGNED NOT NULL,
    `breast_right_ecostrcture` VARCHAR (255),
    `breast_right_duct`        VARCHAR (20),
    `breast_right_ligament`    VARCHAR (20),
    `breast_right_formations`  VARCHAR (500),
    `breast_right_ganglions`   VARCHAR (300),
    `breast_left_ecostrcture`  VARCHAR (255),
    `breast_left_duct`         VARCHAR (20),
    `breast_left_ligament`     VARCHAR (20),
    `breast_left_formations`   VARCHAR (500),
    `breast_left_ganglions`    VARCHAR (300),
    `concluzion`               VARCHAR (500),
    `recommendation`           VARCHAR (255),
    PRIMARY KEY (`id`),
    KEY `idx_tableBreast_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableBreast_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;
