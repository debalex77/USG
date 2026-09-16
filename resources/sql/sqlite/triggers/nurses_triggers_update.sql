CREATE TRIGGER `update_full_name_nurse`
AFTER UPDATE ON `nurses`
FOR EACH ROW
BEGIN
    UPDATE fullNameNurses SET
        name            = NEW.name || ' ' || NEW.fName,
        nameAbbreviated = NEW.name || ' ' || substr(NEW.fName, 1, 1) || '.',
        nameTelephone   = NEW.name || ' ' || NEW.fName || ', tel.: ' || NEW.telephone
    WHERE
        id_nurses = NEW.id;
END;
