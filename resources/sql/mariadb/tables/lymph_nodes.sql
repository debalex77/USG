CREATE TABLE IF NOT EXISTS tableSofTissuesLymphNodes (
    `id`                     BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho`          BIGINT UNSIGNED NOT NULL,
    `section_type`           ENUM ('soft_tissues','lymph_nodes','tissues_nodes') DEFAULT 'lymph_nodes',
    `examinedArea`           VARCHAR(50) DEFAULT NULL,
    `clinicalIndications`    VARCHAR(100) DEFAULT NULL,
    `skin_structure`         VARCHAR(50) DEFAULT NULL,
    `subcutaneous_tissue`    VARCHAR(100) DEFAULT NULL,
    `lesion_location`        VARCHAR(100) DEFAULT NULL,
    `lesion_size`VARCHAR(50) DEFAULT NULL,
    `lesion_echogenicity`    VARCHAR(50) DEFAULT NULL,
    `lesion_contour`         VARCHAR(30) DEFAULT NULL,
    `lesion_vascularization` VARCHAR(30) DEFAULT NULL,
    `ln_number`              INT,
    `ln_size_nodes`          VARCHAR(50) DEFAULT NULL,
    `ln_shape`               VARCHAR(50) DEFAULT NULL,
    `ln_echogenic_hilum`     VARCHAR(50) DEFAULT NULL,
    `ln_cortex`              VARCHAR(50) DEFAULT NULL,
    `ln_structure`           VARCHAR(100) DEFAULT NULL,
    `ln_contour`             VARCHAR(30) DEFAULT NULL,
    `ln_vascularization`     VARCHAR(30) DEFAULT NULL,
    `ln_associated_changes`  VARCHAR(150) DEFAULT NULL,
    `other_changes`          VARCHAR(250) DEFAULT NULL,
    `concluzion`             VARCHAR(500) DEFAULT NULL,
    `recommendation`         VARCHAR(250) DEFAULT NULL,
    PRIMARY KEY (`id`),
    KEY `idx_tableSofTissuesLymphNodes_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableSofTissuesLymphNodes_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;
