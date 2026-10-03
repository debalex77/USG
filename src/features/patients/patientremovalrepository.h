#ifndef PATIENTREMOVALREPOSITORY_H
#define PATIENTREMOVALREPOSITORY_H

#include <QDateTime>
#include <QList>
#include <QString>

class DataBase;

// Eliminarea definitivă a pacientului din baza de date. Pacientul care
// figurează în comenzi, rapoarte sau programări nu se elimină.
class PatientRemovalRepository
{
public:
    struct Reference
    {
        enum class Kind { Order = 1, Report = 2, Appointment = 3 };

        Kind kind = Kind::Order;
        QString numberDoc;
        QDateTime dateDoc;
    };

    enum class RemoveResult { Removed, Referenced, Error };

    explicit PatientRemovalRepository(DataBase &db);

    // Documentele în care figurează pacientul; false la eroare SQL.
    bool findReferences(qint64 patientId, QList<Reference> *references,
                        QString *error = nullptr) const;

    // Ștergerea și verificarea referințelor se fac într-o singură interogare,
    // ca un document salvat între timp să nu rămână fără pacient.
    RemoveResult removePatient(qint64 patientId, QString *error = nullptr) const;

private:
    void removeOrphanImages(qint64 patientId) const;

    DataBase &m_db;
};

#endif // PATIENTREMOVALREPOSITORY_H
