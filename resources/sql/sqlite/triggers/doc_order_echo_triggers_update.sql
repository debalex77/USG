CREATE TRIGGER update_orderDoc_presentation
AFTER UPDATE ON orderEcho
FOR EACH ROW
BEGIN
    UPDATE orderEchoPresentation SET
        docPresentation =
            'Comanda ecografica nr.' || NEW.numberDoc || ' din ' ||
            strftime('%d.%m.%Y %H:%M:%S', NEW.dateDoc),
        docPresentationDate =
            'Comanda ecografica nr.' || NEW.numberDoc || ' din ' ||
            strftime('%d.%m.%Y', NEW.dateDoc)
    WHERE
        id_orderEcho = NEW.id;
END;
