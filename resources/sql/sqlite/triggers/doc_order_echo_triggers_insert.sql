CREATE TRIGGER create_orderDoc_presentation
AFTER INSERT ON orderEcho
FOR EACH ROW
BEGIN
    INSERT INTO orderEchoPresentation
        (id_orderEcho, docPresentation, docPresentationDate)
    VALUES (
        NEW.id,
        'Comanda ecografica nr.' || NEW.numberDoc || ' din ' ||
        strftime('%d.%m.%Y %H:%M:%S', NEW.dateDoc),
        'Comanda ecografica nr.' || NEW.numberDoc || ' din ' ||
        strftime('%d.%m.%Y', NEW.dateDoc)
    );
END;
