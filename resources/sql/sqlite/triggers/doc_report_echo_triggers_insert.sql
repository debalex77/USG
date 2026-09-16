CREATE TRIGGER create_reportEcho_presentation
AFTER INSERT ON reportEcho
FOR EACH ROW
BEGIN
    INSERT INTO reportEchoPresentation
        (id_reportEcho, docPresentation, docPresentationDate)
    VALUES (
        NEW.id,
        'Raport ecografic nr.' || NEW.numberDoc || ' din ' ||
        strftime('%d.%m.%Y %H:%M:%S', NEW.dateDoc),
        'Raport ecografic nr.' || NEW.numberDoc || ' din ' ||
        strftime('%d.%m.%Y', NEW.dateDoc)
    );
END;
