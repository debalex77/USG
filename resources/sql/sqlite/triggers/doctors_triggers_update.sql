CREATE TRIGGER update_full_name_doctor
AFTER UPDATE ON doctors
FOR EACH ROW
BEGIN
    UPDATE fullNameDoctors SET
        name            = NEW.name || ' ' || NEW.fName,
        nameAbbreviated = NEW.name || ' ' || substr(NEW.fName, 1, 1) || '.',
        nameTelephone   = NEW.name || ' ' || NEW.fName || ', tel.: ' || NEW.telephone
    WHERE
        id_doctors = NEW.id;
END;
