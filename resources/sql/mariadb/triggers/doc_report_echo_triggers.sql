CREATE TRIGGER `create_reportEcho_presentation`
AFTER INSERT ON `reportEcho`
FOR EACH ROW
BEGIN
    INSERT INTO reportEchoPresentation
        (id_reportEcho , docPresentation, docPresentationDate)
    VALUES (
        NEW.id,
        CONCAT('Raport ecografic nr.', NEW.numberDoc ,' din ', DATE_FORMAT(NEW.dateDoc, '%d.%m.%Y %H:%i:%s')),
        CONCAT('Raport ecografic nr.', NEW.numberDoc ,' din ', DATE_FORMAT(NEW.dateDoc, '%d.%m.%Y'))
    );
END;

CREATE TRIGGER `update_reportEcho_presentation`
AFTER UPDATE ON `reportEcho`
FOR EACH ROW
BEGIN
    UPDATE reportEchoPresentation SET
        docPresentation     = CONCAT('Raport ecografic nr.', NEW.numberDoc , ' din ', DATE_FORMAT(NEW.dateDoc, '%d.%m.%Y %H:%i:%s')),
        docPresentationDate = CONCAT('Raport ecografic nr.', NEW.numberDoc , ' din ', DATE_FORMAT(NEW.dateDoc, '%d.%m.%Y'))
    WHERE
        id_reportEcho = NEW.id;
END;
