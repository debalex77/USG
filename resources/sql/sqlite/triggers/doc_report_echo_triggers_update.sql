CREATE TRIGGER update_reportEcho_presentation
AFTER UPDATE ON reportEcho
FOR EACH ROW
BEGIN
    UPDATE reportEchoPresentation SET
        docPresentation =
            'Raport ecografic nr.' || NEW.numberDoc || ' din ' ||
            strftime('%d.%m.%Y %H:%M:%S', NEW.dateDoc),
        docPresentationDate =
            'Raport ecografic nr.' || NEW.numberDoc || ' din ' ||
            strftime('%d.%m.%Y', NEW.dateDoc)
    WHERE
        id_reportEcho = NEW.id;
END;
