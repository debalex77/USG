CREATE TABLE IF NOT EXISTS `conclusionTemplates` (
    `id`           BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `deletionMark` TINYINT(1) NOT NULL DEFAULT 0,
    `cod`          VARCHAR (10) NOT NULL,
    `name`         VARCHAR (500) NOT NULL,
    `system`       VARCHAR (150),
    `uuid`         BINARY(16) NOT NULL,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uq_conclusionTemplates_uuid` (`uuid`)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `formationsSystemTemplates` (
    `id`           BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `deletionMark` TINYINT(1) NOT NULL DEFAULT 0,
    `name`         VARCHAR (500) NOT NULL,
    `typeSystem`   ENUM('Unknow', 'Ficat', 'Colecist', 'Pancreas', 'Splina', 'Intestine', 'Recomandari (org.interne)',
                        'Rinichi', 'V.urinara', 'Gl.suprarenale', 'Recomandari (s.urinar)',
                        'Prostata', 'Recomandari (prostata)',
                        'Tiroida', 'Recomandari (tiroida)',
                        'Gl.mamara (stanga)', 'Gl.mamara (dreapta)', 'Recomandari (gl.mamare)',
                        'Ginecologia (uter)', 'Ginecologia (ovar stang)', 'Ginecologia (ovar drept)', 'Recomandari (ginecologia)',
                        'Recomandari (gestatation0)', 'Recomandari (gestatation1)', 'Recomandari (gestatation2)',
                        'Recomandari (gangl.limfatici)') DEFAULT 'Unknow',
    `uuid`         BINARY(16) NOT NULL,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uq_formationsSystemTemplates_uuid` (`uuid`)
) ENGINE=InnoDB;
