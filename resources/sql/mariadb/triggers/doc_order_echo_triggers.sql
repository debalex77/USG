-- =========================================
-- TRIGGER: AFTER INSERT
-- =========================================
CREATE TRIGGER `create_orderDoc_presentation`
AFTER INSERT ON `orderEcho`
FOR EACH ROW
BEGIN
    INSERT INTO orderEchoPresentation
        (id_orderEcho, docPresentation, docPresentationDate)
    VALUES (
        NEW.id,
        CONCAT('Comanda ecografica nr.', NEW.numberDoc ,' din ', DATE_FORMAT(NEW.dateDoc, '%d.%m.%Y %H:%i:%s')),
        CONCAT('Comanda ecografica nr.', NEW.numberDoc ,' din ', DATE_FORMAT(NEW.dateDoc, '%d.%m.%Y'))
    );
END;

-- =========================================
-- TRIGGER: AFTER UPDATE
-- =========================================
CREATE TRIGGER `update_orderDoc_presentation`
AFTER UPDATE ON `orderEcho`
FOR EACH ROW
BEGIN
    UPDATE orderEchoPresentation SET
        docPresentation     = CONCAT('Comanda ecografica nr.', NEW.numberDoc , ' din ', DATE_FORMAT(NEW.dateDoc, '%d.%m.%Y %H:%i:%s')),
        docPresentationDate = CONCAT('Comanda ecografica nr.', NEW.numberDoc , ' din ', DATE_FORMAT(NEW.dateDoc, '%d.%m.%Y'))
    WHERE
        id_orderEcho = NEW.id;
END;
