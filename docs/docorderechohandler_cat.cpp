#include <docs/docorderechohandler_p.h>

void DocOrderEchoHandler::Impl::insertNewCatDoctor()
{
    auto catDoctors = new CatGeneral(&o);
    catDoctors->setAttribute(Qt::WA_DeleteOnClose);
    catDoctors->setProperty("typeCatalog", CatGeneral::TypeCatalog::Doctors);
    catDoctors->setProperty("itNew", true);
    catDoctors->show();
}

void DocOrderEchoHandler::Impl::openCatDoctor()
{
    auto ui = o.uiPtr();
    int id_doctor = ui->comboDoctor->itemData(ui->comboDoctor->currentIndex(), Qt::UserRole).toInt();
    if (id_doctor == 0)
        return;

    qInfo(logInfo()) << QObject::tr("Editarea datelor doctorului '%1' cu id='%2'.")
                            .arg(ui->comboDoctor->currentText(), QString::number(o.getIdDoctor()));

    CatGeneral* catDoctors = new CatGeneral(&o);
    catDoctors->setAttribute(Qt::WA_DeleteOnClose);
    catDoctors->setProperty("typeCatalog", CatGeneral::TypeCatalog::Doctors);
    catDoctors->setProperty("itNew", false);
    catDoctors->setProperty("Id", o.getIdDoctor());
    catDoctors->show();
}

void DocOrderEchoHandler::Impl::updateModelDoctor(BaseSqlQueryModel &modelDoctors)
{
    /** golim modelul */
    if (modelDoctors.rowCount() > 0)
        modelDoctors.clear();

    /** setam solicitarea */
    modelDoctors.setQuery(R"(
        SELECT
            doctors.id,
            fullNameDoctors.name
        FROM
            doctors
        INNER JOIN
            fullNameDoctors ON doctors.id = fullNameDoctors.id_doctors
        WHERE
            deletionMark = 0
        ORDER BY
            fullNameDoctors.name
    )");

}

void DocOrderEchoHandler::Impl::updateModelNurse(BaseSqlQueryModel &modelNurses)
{
    /** golim modelul */
    if (modelNurses.rowCount() > 0)
        modelNurses.clear();

    /** setam solicitarea */
    modelNurses.setQuery(R"(
        SELECT
            nurses.id,
            fullNameNurses.name
        FROM
            nurses
        INNER JOIN
            fullNameNurses ON nurses.id = fullNameNurses.id_nurses
        WHERE
            nurses.deletionMark = 0
        ORDER BY
            fullNameNurses.name
    )");
}

void DocOrderEchoHandler::Impl::updateModelPatients(QStandardItemModel &modelPatients)
{
    /** golim modelul */
    modelPatients.clear();

    /** pregatim solicitarea */
    const QString strQuery =
        globals().thisMySQL ?
        QStringLiteral(R"(
            SELECT
                pacients.id,
                CONCAT(pacients.name, ' ', pacients.fName, ', ',
                DATE_FORMAT(pacients.birthday, '%d.%m.%Y'), ', idnp: ',
                IFNULL(pacients.IDNP, '')) AS FullName
            FROM pacients
            WHERE pacients.deletionMark = 0
            ORDER BY FullName ASC;
        )")
                            :
        QStringLiteral(R"(
            SELECT
                pacients.id,
                pacients.name || ' ' || pacients.fName || ', ' ||
                strftime('%d.%m.%Y', pacients.birthday) || ', idnp: ' ||
                IFNULL(pacients.IDNP, '') AS FullName
            FROM pacients
            WHERE pacients.deletionMark = 0
            ORDER BY FullName ASC;
        )");

    /** executam solicitarea */
    QSqlQuery qry;
    if (! qry.exec(strQuery)) {
        qWarning(logWarning()) << "Eroare exec query:"
                               << qry.lastError().text();
        return;
    }

    /** pu performanta cream container */
    QList<QStandardItem*> items;

    /** prelucrarea solicitarii si completarea containerului 'items' */
    if (qry.exec()) {
        while (qry.next()) {
            QStandardItem *item = new QStandardItem;
            item->setData(qry.value(0).toInt(), Qt::UserRole);
            item->setData(qry.value(1).toString(), Qt::DisplayRole);
            items.append(item);
        }
    }

    /** adaugam toate randurile printr-o tranzactie/simultan */
    modelPatients.invisibleRootItem()->appendRows(items);
}
