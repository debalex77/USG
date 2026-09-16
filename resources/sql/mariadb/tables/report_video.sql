CREATE TABLE IF NOT EXISTS `reportVideo` (
    `id`            BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_orderEcho`  BIGINT UNSIGNED NOT NULL,
    `id_reportEcho` BIGINT UNSIGNED NOT NULL,
    `relative_path` VARCHAR (512) NOT NULL,
    `file_name`     VARCHAR (80) NOT NULL,
    `comment`       VARCHAR (255),
    `uuid`          BINARY(16) NOT NULL,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uq_reportVideo_uuid` (`uuid`),
    CONSTRAINT `fk_reportVideo_orderEcho`
        FOREIGN KEY (`id_orderEcho`)
        REFERENCES `orderEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_reportVideo_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;
