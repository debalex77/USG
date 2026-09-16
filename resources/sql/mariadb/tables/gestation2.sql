CREATE TABLE IF NOT EXISTS `tableGestation2` (
    `id`               BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho`    BIGINT UNSIGNED NOT NULL,
    `gestation_age`    VARCHAR(20) DEFAULT NULL,
    `trimestru`        INT DEFAULT NULL,
    `dateMenstruation` VARCHAR(10) DEFAULT NULL,
    `view_examination` INT DEFAULT NULL,
    `single_multiple_pregnancy` INT DEFAULT NULL,
    `single_multiple_pregnancy_description` VARCHAR(250) DEFAULT NULL,
    `antecedent`        VARCHAR(150) DEFAULT NULL,
    `comment`           VARCHAR(250) DEFAULT NULL,
    `concluzion`        VARCHAR(500) DEFAULT NULL,
    `recommendation`    VARCHAR(250) DEFAULT NULL,
    `fetalPrezentation` INT DEFAULT (0),
    `multiplePregnancy` INT DEFAULT (0),
    PRIMARY KEY (`id`),
    KEY `idx_tableGestation2_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableGestation2_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `tableGestation2_biometry` (
    `id`               BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho`    BIGINT UNSIGNED NOT NULL,
    `BPD`              VARCHAR(5) DEFAULT NULL,
    `BPD_age`          VARCHAR(20) DEFAULT NULL,
    `HC`               VARCHAR(5) DEFAULT NULL,
    `HC_age`           VARCHAR(20) DEFAULT NULL,
    `AC`               VARCHAR(5) DEFAULT NULL,
    `AC_age`           VARCHAR(20) DEFAULT NULL,
    `FL`               VARCHAR(5) DEFAULT NULL,
    `FL_age`           VARCHAR(20) DEFAULT NULL,
    `FetusCorresponds` VARCHAR(20) DEFAULT NULL,
    PRIMARY KEY (`id`),
    KEY `idx_tableGestation2_biometry_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableGestation2_biometry_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `tableGestation2_cranium` (
    `id`                 BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho`      BIGINT UNSIGNED NOT NULL,
    `calloteCranium`     INT DEFAULT NULL,
    `facialeProfile`     INT DEFAULT NULL,
    `nasalBones`         INT DEFAULT NULL,
    `nasalBones_dimens`  VARCHAR(5) DEFAULT NULL,
    `eyeball`            INT DEFAULT NULL,
    `eyeball_desciption` VARCHAR(100) DEFAULT NULL,
    `nasolabialTriangle` INT DEFAULT NULL,
    `nasolabialTriangle_description` VARCHAR(100) DEFAULT NULL,
    `nasalFold`          VARCHAR(5) DEFAULT NULL,
    PRIMARY KEY (`id`),
    KEY `idx_tableGestation2_cranium_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableGestation2_cranium_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `tableGestation2_SNC` (
    `id`                            BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho`                 BIGINT UNSIGNED NOT NULL,
    `hemispheres`                   INT DEFAULT NULL,
    `fissureSilvius`                INT DEFAULT NULL,
    `corpCalos`                     INT DEFAULT NULL,
    `ventricularSystem`             INT DEFAULT NULL,
    `ventricularSystem_description` VARCHAR(70) DEFAULT NULL,
    `cavityPellucidSeptum`          INT DEFAULT NULL,
    `choroidalPlex`                 INT DEFAULT NULL,
    `choroidalPlex_description`     VARCHAR(70) DEFAULT NULL,
    `cerebellum`                    INT DEFAULT NULL,
    `cerebellum_description`        VARCHAR(70) DEFAULT NULL,
    `vertebralColumn`               INT DEFAULT NULL,
    `vertebralColumn_description`   VARCHAR(100) DEFAULT NULL,
    PRIMARY KEY (`id`),
    KEY `idx_tableGestation2_SNC_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableGestation2_SNC_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `tableGestation2_heart` (
    `id`                                       BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho`                            BIGINT UNSIGNED NOT NULL,
    `position`                                 VARCHAR(50) DEFAULT NULL,
    `heartBeat`                                INT DEFAULT NULL,
    `heartBeat_frequency`                      VARCHAR(5) DEFAULT NULL,
    `heartBeat_rhythm`                         INT DEFAULT NULL,
    `pericordialCollections`                   INT DEFAULT NULL,
    `planPatruCamere`                          INT DEFAULT NULL,
    `planPatruCamere_description`              VARCHAR(70) DEFAULT NULL,
    `ventricularEjectionPathLeft`              INT DEFAULT NULL,
    `ventricularEjectionPathLeft_description`  VARCHAR(70) DEFAULT NULL,
    `ventricularEjectionPathRight`             INT DEFAULT NULL,
    `ventricularEjectionPathRight_description` VARCHAR(70) DEFAULT NULL,
    `intersectionVesselMagistral`              INT DEFAULT NULL,
    `intersectionVesselMagistral_description`  VARCHAR(70) DEFAULT NULL,
    `planTreiVase`                             INT DEFAULT NULL,
    `planTreiVase_description`                 VARCHAR(70) DEFAULT NULL,
    `archAorta`                                INT DEFAULT NULL,
    `planBicav`                                INT DEFAULT NULL,
    PRIMARY KEY (`id`),
    KEY `idx_tableGestation2_heart_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableGestation2_heart_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `tableGestation2_thorax` (
    `id`                         BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho`              BIGINT UNSIGNED NOT NULL,
    `pulmonaryAreas`             INT DEFAULT NULL,
    `pulmonaryAreas_description` VARCHAR(70) DEFAULT NULL,
    `pleuralCollections`         INT DEFAULT NULL,
    `diaphragm`                  INT DEFAULT NULL,
    PRIMARY KEY (`id`),
    KEY `idx_tableGestation2_thorax_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableGestation2_thorax_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `tableGestation2_abdomen` (
    `id`                    BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho`         BIGINT UNSIGNED NOT NULL,
    `abdominalWall`         INT DEFAULT NULL,
    `abdominalCollections`  INT DEFAULT NULL,
    `stomach`               INT DEFAULT NULL,
    `stomach_description`   VARCHAR(50) DEFAULT NULL,
    `abdominalOrgans`       INT DEFAULT NULL,
    `cholecist`             INT DEFAULT NULL,
    `cholecist_description` VARCHAR(50) DEFAULT NULL,
    `intestine`             INT DEFAULT NULL,
    `intestine_description` VARCHAR(70) DEFAULT NULL,
    PRIMARY KEY (`id`),
    KEY `idx_tableGestation2_abdomen_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableGestation2_abdomen_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `tableGestation2_urinarySystem` (
    `id`                   BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho`        BIGINT UNSIGNED NOT NULL,
    `kidneys`              INT DEFAULT NULL,
    `kidneys_descriptions` VARCHAR(70) DEFAULT NULL,
    `ureter`               INT DEFAULT NULL,
    `ureter_descriptions`  VARCHAR(70) DEFAULT NULL,
    `bladder`              INT DEFAULT NULL,
    PRIMARY KEY (`id`),
    KEY `idx_tableGestation2_urinarySystem_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableGestation2_urinarySystem_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `tableGestation2_other` (
    `id`                             BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho`                  BIGINT UNSIGNED NOT NULL,
    `externalGenitalOrgans`          INT DEFAULT NULL,
    `externalGenitalOrgans_aspect`   INT DEFAULT NULL,
    `extremities`                    INT DEFAULT NULL,
    `extremities_descriptions`       VARCHAR(150) DEFAULT NULL,
    `fetusMass`                      VARCHAR(5) DEFAULT NULL,
    `placenta`                       INT DEFAULT NULL,
    `placentaLocalization`           VARCHAR(50) DEFAULT NULL,
    `placentaDegreeMaturation`       VARCHAR(5) DEFAULT NULL,
    `placentaDepth`                  VARCHAR(5) DEFAULT NULL,
    `placentaStructure`              INT DEFAULT NULL,
    `placentaStructure_descriptions` VARCHAR(150) DEFAULT NULL,
    `umbilicalCordon`                INT DEFAULT NULL,
    `umbilicalCordon_description`    VARCHAR(70) DEFAULT NULL,
    `insertionPlacenta`              INT DEFAULT NULL,
    `amnioticIndex`                  VARCHAR(5) DEFAULT NULL,
    `amnioticIndexAspect`            INT DEFAULT NULL,
    `amnioticBedDepth`               VARCHAR(5) DEFAULT NULL,
    `cervix`                         VARCHAR(5) DEFAULT NULL,
    `cervix_description`             VARCHAR(150) DEFAULT NULL,
    PRIMARY KEY (`id`),
    KEY `idx_tableGestation2_other_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableGestation2_other_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `tableGestation2_doppler` (
    `id`             BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `id_reportEcho`  BIGINT UNSIGNED NOT NULL,
    `ombilic_PI`     VARCHAR(5) DEFAULT NULL,
    `ombilic_RI`     VARCHAR(5) DEFAULT NULL,
    `ombilic_SD`     VARCHAR(5) DEFAULT NULL,
    `ombilic_flux`   INT DEFAULT NULL,
    `cerebral_PI`    VARCHAR(5) DEFAULT NULL,
    `cerebral_RI`    VARCHAR(5) DEFAULT NULL,
    `cerebral_SD`    VARCHAR(5) DEFAULT NULL,
    `cerebral_flux`  INT DEFAULT NULL,
    `uterRight_PI`   VARCHAR(5) DEFAULT NULL,
    `uterRight_RI`   VARCHAR(5) DEFAULT NULL,
    `uterRight_SD`   VARCHAR(5) DEFAULT NULL,
    `uterRight_flux` INT DEFAULT NULL,
    `uterLeft_PI`    VARCHAR(5) DEFAULT NULL,
    `uterLeft_RI`    VARCHAR(5) DEFAULT NULL,
    `uterLeft_SD`    VARCHAR(5) DEFAULT NULL,
    `uterLeft_flux`  INT DEFAULT NULL,
    `ductVenos`      VARCHAR(50) DEFAULT NULL,
    PRIMARY KEY (`id`),
    KEY `idx_tableGestation2_doppler_reportEcho` (`id_reportEcho`),
    CONSTRAINT `fk_tableGestation2_doppler_reportEcho`
        FOREIGN KEY (`id_reportEcho`)
        REFERENCES `reportEcho` (`id`)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
) ENGINE=InnoDB;
