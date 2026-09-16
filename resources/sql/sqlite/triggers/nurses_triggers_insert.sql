CREATE TRIGGER `create_full_name_nurse`
AFTER INSERT ON `nurses`
FOR EACH ROW
BEGIN
    INSERT INTO fullNameNurses
        (id_nurses, name, nameAbbreviated, nameTelephone)
    VALUES (
        NEW.id,
        NEW.name || ' ' || NEW.fName,
        NEW.name || ' ' || substr(NEW.fName, 1, 1) || '.',
        NEW.name || ' ' || NEW.fName || ', tel.: ' || NEW.telephone
    );
END;
