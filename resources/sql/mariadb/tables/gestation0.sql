CREATE TABLE IF NOT EXISTS `tableGestation0` (
    `id`               BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho`    BIGINT UNSIGNED NOT NULL,
    `view_examination` INT,
    `antecedent`       VARCHAR (150),
    `lmp`              VARCHAR(10),
    `gestation_age`    VARCHAR (20),
    `GS`               VARCHAR (5),
    `GS_age`           VARCHAR (20),
    `CRL`              VARCHAR (5),
    `CRL_age`          VARCHAR (20),
    `BCF`              VARCHAR (30),
    `liquid_amniotic`  VARCHAR (40),
    `miometer`         VARCHAR (200),
    `cervix`           VARCHAR (200),
    `ovary`            VARCHAR (200),
    `concluzion`       VARCHAR (500),
    `recommendation`   VARCHAR (255),
    PRIMARY KEY (`id`),
    KEY `idx_tableGestation0_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableGestation0_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;
