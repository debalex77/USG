CREATE TABLE IF NOT EXISTS `tableLiver` (
    `id`                BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho`     BIGINT UNSIGNED NOT NULL,
    `left`              VARCHAR (5),
    `right`             VARCHAR (5),
    `contur`            VARCHAR (20),
    `parenchim`         VARCHAR (20),
    `ecogenity`         VARCHAR (30),
    `formations`        VARCHAR (300),
    `ductsIntrahepatic` VARCHAR (50),
    `porta`             VARCHAR (5),
    `lienalis`          VARCHAR (5),
    `concluzion`        VARCHAR (500),
    `recommendation`    VARCHAR (255),
    PRIMARY KEY (`id`),
    KEY `idx_tableLiver_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableLiver_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `tableCholecist` (
    `id`            BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho` BIGINT UNSIGNED NOT NULL,
    `form`          VARCHAR (150),
    `dimens`        VARCHAR (15),
    `walls`         VARCHAR (5),
    `choledoc`      VARCHAR (5),
    `formations`    VARCHAR (300),
    PRIMARY KEY (`id`),
    KEY `idx_tableCholecist_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableCholecist_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `tablePancreas` (
    `id`            BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho` BIGINT UNSIGNED NOT NULL,
    `cefal`         VARCHAR (5),
    `corp`          VARCHAR (5),
    `tail`          VARCHAR (5),
    `texture`       VARCHAR (20),
    `ecogency`      VARCHAR (30),
    `formations`    VARCHAR (300),
    PRIMARY KEY (`id`),
    KEY `idx_tablePancreas_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tablePancreas_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `tableSpleen` (
    `id`            BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho` BIGINT UNSIGNED NOT NULL,
    `dimens`        VARCHAR (15),
    `contur`        VARCHAR (20),
    `parenchim`     VARCHAR (30),
    `formations`    VARCHAR (300),
    PRIMARY KEY (`id`),
    KEY `idx_tableSpleen_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableSpleen_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `tableIntestinalLoop` (
    `id`            BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho` BIGINT UNSIGNED NOT NULL,
    `formations`    VARCHAR (300),
    PRIMARY KEY (`id`),
    KEY `idx_tableIntestinalLoop_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableIntestinalLoop_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;
