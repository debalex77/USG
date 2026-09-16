CREATE TRIGGER create_full_name_doctor
AFTER INSERT ON doctors
FOR EACH ROW
BEGIN
    INSERT INTO fullNameDoctors
        (id_doctors, name, nameAbbreviated, nameTelephone)
    VALUES (
        NEW.id,
        NEW.name || ' ' || NEW.fName,
        NEW.name || ' ' || substr(NEW.fName, 1, 1) || '.',
        NEW.name || ' ' || NEW.fName || ', tel.: ' || NEW.telephone
    );
END;
