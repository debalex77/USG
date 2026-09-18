SELECT
    c.id_organizations,
    org.IDNP,
    org.name,
    org.address,
    org.telephone,
    fd.nameAbbreviated AS doctor,
    fn.nameAbbreviated AS nurse,
    org.email,
    org.site
FROM
    constants c
LEFT JOIN
    organizations org ON c.id_organizations = org.id
LEFT JOIN
    fullNameDoctors fd ON c.id_doctors = fd.id_doctors
LEFT JOIN
    fullNameNurses fn on c.id_nurses = fn.id_nurses
WHERE
    c.id_users = ?
