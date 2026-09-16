CREATE TABLE IF NOT EXISTS  `normograms` (
    `id`         BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `name`       VARCHAR (30) NOT NULL,
    `crl`        VARCHAR (5) NOT NULL,
    `5_centile`  DOUBLE NOT NULL,
    `50_centile` DOUBLE NOT NULL,
    `95_centile` DOUBLE NOT NULL,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB;
