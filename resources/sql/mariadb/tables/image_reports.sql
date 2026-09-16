CREATE TABLE IF NOT EXISTS `imagesReports` (
    `id`            BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho` BIGINT UNSIGNED NOT NULL,
    `id_orderEcho`  BIGINT UNSIGNED NOT NULL,
    `patient_id`   BIGINT UNSIGNED NOT NULL,
    `image_1`       LONGBLOB,
    `image_2`       LONGBLOB,
    `image_3`       LONGBLOB,
    `image_4`       LONGBLOB,
    `image_5`       LONGBLOB,
    `comment_1`     VARCHAR (150),
    `comment_2`     VARCHAR (150),
    `comment_3`     VARCHAR (150),
    `comment_4`     VARCHAR (150),
    `comment_5`     VARCHAR (150),
    `id_user`       BIGINT UNSIGNED NOT NULL,
    `uuid`          BINARY(16) NOT NULL,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uq_imagesReports_uuid` (`uuid`),
    KEY `idx_imagesReports_reportEcho` (`id_reportEcho`),
    KEY `idx_imagesReports_orderEcho` (`id_orderEcho`),
    KEY `idx_imagesReports_patients` (`patient_id`),
    KEY `idx_imagesReports_user` (`id_user`),
    CONSTRAINT `fk_imagesReports_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_imagesReports_orderEcho`
        FOREIGN KEY (`id_orderEcho`)
        REFERENCES `orderEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_imagesReports_patients`
        FOREIGN KEY (`patient_id`)
        REFERENCES `patients` (`id`)
        ON DELETE RESTRICT
        ON UPDATE RESTRICT,
    CONSTRAINT `fk_imagesReports_users`
        FOREIGN KEY (`id_user`)
        REFERENCES `users` (`id`)
        ON DELETE RESTRICT
        ON UPDATE RESTRICT
) ENGINE=InnoDB;
