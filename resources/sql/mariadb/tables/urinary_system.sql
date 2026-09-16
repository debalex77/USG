CREATE TABLE IF NOT EXISTS `tableKidney` (
    `id`                    BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho`         BIGINT UNSIGNED NOT NULL,
    `contour_right`         ENUM('clar', 'sters', 'regulat', 'neregulat') DEFAULT 'clar',
    `contour_left`          ENUM('clar', 'sters', 'regulat', 'neregulat') DEFAULT 'clar',
    `dimens_right`          VARCHAR (15),
    `dimens_left`           VARCHAR (15),
    `corticomed_right`      VARCHAR (5),
    `corticomed_left`       VARCHAR (5),
    `pielocaliceal_right`   VARCHAR (30),
    `pielocaliceal_left`    VARCHAR (30),
    `formations`            VARCHAR (500),
    `suprarenal_formations` VARCHAR(500) DEFAULT NULL,
    `concluzion`            VARCHAR (500),
    `recommendation`        VARCHAR (255),
    PRIMARY KEY (`id`),
    KEY `idx_tableKidney_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableKidney_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `tableBladder` (
    `id`            BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho` BIGINT UNSIGNED NOT NULL,
    `volum`         VARCHAR (5),
    `walls`         VARCHAR (5),
    `formations`    VARCHAR (300),
    PRIMARY KEY (`id`),
    KEY `idx_tableBladder_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableBladder_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;
