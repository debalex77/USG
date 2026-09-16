CREATE TRIGGER create_Pricings_presentation
AFTER INSERT ON pricings
FOR EACH ROW
BEGIN
    INSERT INTO pricingsPresentation
        (id_pricings, docPresentation, docPresentationDate)
    VALUES (
        NEW.id,
        'Formarea prețurilor nr.' || NEW.numberDoc || ' din ' ||
        strftime('%d.%m.%Y %H:%M:%S', NEW.dateDoc),
        'Formarea prețurilor nr.' || NEW.numberDoc || ' din ' ||
        strftime('%d.%m.%Y', NEW.dateDoc)
    );
END;
