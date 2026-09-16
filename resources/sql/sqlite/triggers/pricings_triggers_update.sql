CREATE TRIGGER update_Pricings_presentation
AFTER UPDATE ON pricings
FOR EACH ROW
BEGIN
    UPDATE pricingsPresentation SET
        docPresentation =
            'Formarea prețurilor nr.' || NEW.numberDoc || ' din ' ||
            strftime('%d.%m.%Y %H:%M:%S', NEW.dateDoc),
        docPresentationDate =
            'Formarea prețurilor nr.' || NEW.numberDoc || ' din ' ||
            strftime('%d.%m.%Y', NEW.dateDoc)
    WHERE
        id_pricings = NEW.id;
END;
