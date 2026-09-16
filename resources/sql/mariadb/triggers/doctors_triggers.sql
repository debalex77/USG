-- =========================================
-- TRIGGER: AFTER INSERT
-- =========================================
CREATE TRIGGER `create_full_name_doctor`
AFTER INSERT ON `doctors`
FOR EACH ROW
BEGIN
    INSERT INTO fullNameDoctors
        (id_doctors, name, nameAbbreviated, nameTelephone)
    VALUES (
        NEW.id,
        CONCAT(NEW.name, ' ', NEW.fName),
        CONCAT(NEW.name, ' ', SUBSTRING(NEW.fName, 1, 1), '.'),
        CONCAT(NEW.name, ' ', NEW.fName, ', tel.: ', NEW.telephone)
    );
END;

-- =========================================
-- TRIGGER: AFTER UPDATE
-- =========================================
CREATE TRIGGER `update_full_name_doctor`
AFTER UPDATE ON `doctors`
FOR EACH ROW
BEGIN
    UPDATE fullNameDoctors SET
        name            = CONCAT(NEW.name, ' ', NEW.fName),
        nameAbbreviated = CONCAT(NEW.name, ' ', SUBSTRING(NEW.fName, 1, 1), '.'),
        nameTelephone   = CONCAT(NEW.name, ' ', NEW.fName, ', tel.: ', NEW.telephone)
    WHERE
        id_doctors = NEW.id;
END;
