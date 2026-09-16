-- =========================================
-- TRIGGER: AFTER INSERT
-- =========================================
CREATE TRIGGER `create_full_name_nurse`
AFTER INSERT ON `nurses`
FOR EACH ROW
BEGIN
    INSERT INTO fullNameNurses
        (id_nurses, name, nameAbbreviated, nameTelephone)
    VALUES (
        NEW.id,
        CONCAT(NEW.name,' ', NEW.fName),
        CONCAT(NEW.name,' ', SUBSTRING(NEW.fName,1,1),'.'),
        CONCAT(NEW.name,' ', NEW.fName,', tel.: ', NEW.telephone)
    );
END;

-- =========================================
-- TRIGGER: AFTER UPDATE
-- =========================================
CREATE TRIGGER `update_full_name_nurse`
AFTER UPDATE ON `nurses`
FOR EACH ROW
BEGIN
    UPDATE fullNameNurses SET
        name            = CONCAT(NEW.name, ' ', NEW.fName),
        nameAbbreviated = CONCAT(NEW.name, ' ', SUBSTRING(NEW.fName, 1, 1), '.'),
        nameTelephone   = CONCAT(NEW.name, ' ', NEW.fName, ', tel.: ', NEW.telephone)
    WHERE
        id_nurses = NEW.id;
END;
