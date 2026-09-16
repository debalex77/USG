-- =========================================
-- TRIGGER: AFTER INSERT
-- =========================================
CREATE TRIGGER `create_Pricings_presentation`
AFTER INSERT ON `pricings`
FOR EACH ROW
BEGIN
    INSERT INTO pricingsPresentation
        (id_pricings, docPresentation,docPresentationDate)
    VALUES (
        NEW.id,
        CONCAT('Formarea prețurilor nr.', NEW.numberDoc , ' din ', DATE_FORMAT(NEW.dateDoc, '%d.%m.%Y %H:%i:%s')),
        CONCAT('Formarea prețurilor nr.', NEW.numberDoc , ' din ', DATE_FORMAT(NEW.dateDoc, '%d.%m.%Y'))
    );
END;

-- =========================================
-- TRIGGER: AFTER UPDATE
-- =========================================
CREATE TRIGGER `update_Pricings_presentation`
AFTER UPDATE ON `pricings`
FOR EACH ROW
BEGIN
    UPDATE pricingsPresentation SET
        docPresentation     = CONCAT('Formarea prețurilor nr.', NEW.numberDoc , ' din ', DATE_FORMAT(NEW.dateDoc, '%d.%m.%Y %H:%i:%s')),
        docPresentationDate = CONCAT('Formarea prețurilor nr.', NEW.numberDoc , ' din ', DATE_FORMAT(NEW.dateDoc, '%d.%m.%Y'))
    WHERE
        id_pricings = NEW.id;
END;
