#pragma once

#include <QDateTime>
#include <QHash>
#include <QString>

namespace StatusObject {

    enum Column {
        Unknow       = -1,
        ZeroWrite    = 0,
        DeletionMark = 1
    };

    inline Column determineStatusObject(const int val) {
        switch (val) {
        case 0: return StatusObject::ZeroWrite;
        case 1: return StatusObject::DeletionMark;
        default: return StatusObject::Unknow;
        }
    };

    inline int statusObjectToInt(Column val) {
        switch (val) {
        case StatusObject::Unknow: return -1;
        case StatusObject::ZeroWrite: return 0;
        case StatusObject::DeletionMark: return 1;
        default: return -1;
        }
    };
}

namespace name {

}
//------------------------------------------------
// --- Cataloage

namespace CatalogType {

    enum class Type {
        Doctors,
        Nurses,
        Patients,
        Users,
        Organizations,
        Investigations,
        TypesPrices,
        ConclusionTemplates,
        SystemTemplates,
        Unknow
    };

    enum class FormType {
        List,
        Selection
    };

    enum class Item {
        // doctors, nurses
        Id,
        DeletionMark,
        FullName,
        Telephone,
        Email,

        // users
        LastConnection,

        // organizations
        Id_contracts,

        // patients
        Birthday,
        Address,
        IDNP,
        Comment
    };

    inline QString enumToString(Type catalogType){
        switch (catalogType) {
        case Type::Doctors:              return "doctors";
        case Type::Nurses:               return "nurses";
        case Type::Patients:             return "patients";
        case Type::Users:                return "users";
        case Type::Organizations:        return "organizations";
        case Type::Investigations:       return "investigations";
        case Type::TypesPrices:          return "typesPrices";
        case Type::ConclusionTemplates:  return "conclusionTemplates";
        case Type::SystemTemplates:      return "systemTemplates";
        case Type::Unknow:               return "unknow";
        }

        Q_UNREACHABLE();
        return {};
    }

    inline QString enumToStringRo(Type catalogType){
        switch (catalogType) {
        case Type::Doctors:              return "doctori";
        case Type::Nurses:               return "as.medicale";
        case Type::Patients:             return "pacienti";
        case Type::Users:                return "utilizatori";
        case Type::Organizations:        return "centre medicale";
        case Type::Investigations:       return "investigatii";
        case Type::TypesPrices:          return "tipul preturilor";
        case Type::ConclusionTemplates:  return "sabloane concluziilor";
        case Type::SystemTemplates:      return "sabloane sistemelor";
        case Type::Unknow:               return "unknow";
        }

        Q_UNREACHABLE();
        return {};
    }

    inline Type stringToEnum(const QString &str)
    {
        if (str == "doctors")             return Type::Doctors;
        if (str == "nurses")              return Type::Nurses;
        if (str == "patients")            return Type::Patients;
        if (str == "users")               return Type::Users;
        if (str == "organizations")       return Type::Organizations;
        if (str == "investigations")      return Type::Investigations;
        if (str == "typesPrices")         return Type::TypesPrices;
        if (str == "conclusionTemplates") return Type::ConclusionTemplates;
        if (str == "systemTemplates")     return Type::SystemTemplates;
        if (str == "unknow")              return Type::Unknow;

        Q_UNREACHABLE();
        return Type::Unknow;
    }

}

namespace UsersSections {
enum Column {
    Id,
    DeletionMark,
    Name,
    Password,
    Hash,
    LastConnection,
    Uuid
};
}

namespace DoctorsSections {
enum Column {
    Id,
    DeletionMark,
    FullName,
    Telephone,
    Email,
    Comment,
    Uuid
};
}

namespace NursesSections {
enum Column {
    Id,
    DeletionMark,
    FullName,
    Telephone,
    Email,
    Comment,
    Uuid
};
}

namespace PatientsColumns {
enum Column {
    Id,
    DeletionMark,
    FullName,
    Birthday,
    IDNP,
    MedicalPolicy,
    Address,
    Telephone,
    Email,
    Comment,
    Uuid
};
}

namespace PatientSearchColumns {
enum Column {
    Id,
    DeletionMark,
    Name,
    FirstName,
    FullName,
    Birthday,
    MedicalPolicy,
    Address,
    Telephone,
    Email,
    IDNP,
    Comment,
    Uuid
};
}

namespace OrganizationsSections {
enum Column {
    Id,
    DeletionMark,
    Name,
    IDNP,
    Address,
    Telephone,
    Email,
    Comment,
    Id_contracts,
    Stamp,
    Uuid
};
}

namespace InvestigationsSections {
enum Column {
    Id,
    DeletionMark,
    Cod,
    Name,
    Use,
    Owner,
    Uuid
};
}

namespace TypePricesSections {
enum Column {
    Id,
    DeletionMark,
    Name,
    Discount,
    Noncomercial,
    Uuid
};
}

namespace ConclusionTemplatesSections {
enum Column {
    Id,
    DeletionMark,
    Code,
    Name,
    System,
    Uuid
};
}

namespace SystemFormationsTemplatesSections {
enum Column {
    Id,
    DeletionMark,
    Name,
    System,
    Uuid
};
}

namespace ContractsSections {
enum Column {
    Id,
    DeletionMark,
    Id_organization,
    Id_typePrice,
    Name,
    DateInit,
    NotValid,
    Comment,
    Uuid
};
}

namespace ContractsViewSections {
enum Column {
    Id_Contract,
    NameContract,
    Contract_dateInit,
    Name_TypePrice,
    Id_TypesPrices,
    Organization
};
}

namespace ReportForHistoryPatientSections {
enum Column {
    Id,
    FullNameDoc,
    Concluzion
};
}

//------------------------------------------------
// --- Documente

namespace TagInvestigations {

    enum Column {
        OrgansInternal,
        UrinarySystem,
        Prostate,
        Gynecology,
        Breast,
        Thyroide,
        Gestation0,
        Gestation1,
        Gestation2,
        LymphNodes
    };

    inline const QHash<QString, QVector<Column>> CodeMap =
    {
        {"1021",      {OrgansInternal, UrinarySystem}},
        {"1022",      {OrgansInternal, UrinarySystem}},
        {"1023",      {OrgansInternal}},
        {"1024",      {UrinarySystem}},
        {"1025",      {Prostate}},
        {"1026",      {Gynecology}},
        {"1027",      {Gestation0}},
        {"1027.1",    {Gestation0}},
        {"1028",      {Gestation0}},
        {"1028.1",    {Gestation0}},
        {"1028.4.1",  {Gestation1}},
        {"1028.4.2",  {Gestation1}},
        {"1029.1.1",  {Gestation2}},
        {"1029.1.2",  {Gestation2}},
        {"1029.2",    {Gestation2}},
        {"1029.21",   {Gestation2}},
        {"1029.3",    {Gestation2}},
        {"1029.31",   {Gestation2}},
        {"1033",      {Gynecology}},
        {"1036",      {Thyroide}},
        {"1036.1",    {Thyroide}},
        {"1037",      {Breast}},
        {"1037.1",    {Breast}},
        {"1038",      {Prostate}},
        {"1050.19",   {UrinarySystem}},
        {"1050.20",   {UrinarySystem}},
        {"1050.22",   {Gynecology}},
        {"1050.23",   {Gynecology}},
        {"1050.25",   {Gynecology}},
        {"1050.26",   {Gynecology}},
        {"1050.31",   {Thyroide}},
        {"1050.32",   {Thyroide}},
        {"1050.34",   {Breast}},
        {"1050.35",   {Breast}},
        {"1050.52.",  {LymphNodes}},
        {"1050.53.",  {LymphNodes}},
        {"1050.54.",  {LymphNodes}},
        {"1050.55",   {Prostate}},
        {"1050.61",   {OrgansInternal, UrinarySystem}},
        {"1050.62",   {OrgansInternal, UrinarySystem, Gynecology}},
        {"1050.62.1", {OrgansInternal, UrinarySystem, Prostate}},
        {"1050.62.2", {OrgansInternal, UrinarySystem, Prostate, Thyroide}},
        {"1050.63",   {OrgansInternal, UrinarySystem, Gynecology}},
        {"1050.63.1", {OrgansInternal, UrinarySystem, Gynecology, Breast, Thyroide}},
        {"1050.66",   {Gestation0}},
        {"1050.67",   {Gestation0}},
        {"1050.69",   {Gynecology}}
    };

}

namespace PaymentMethod {
enum Column {
    Cash,
    Card,
    Transfer
};
}

namespace PrintType {
enum Column {
    Designer,
    Preview,
    ExportToPDF
};
}

namespace DocStatus {

    enum Column {
        Unknow       = -1,
        Write        = 0,
        DeletionMark = 1,
        Post         = 2
    };

    inline Column determineStatusDoc(const int val) {
        switch (val) {
        case -1: return DocStatus::Unknow;
        case 0: return DocStatus::Write;
        case 1: return DocStatus::DeletionMark;
        case 2: return DocStatus::Post;
        default: return DocStatus::Unknow;
        }
    }
}

namespace OrderSections {
enum column {
    Id,
    DeletionMark,
    NumberDoc,
    DateDoc,
    Id_Organizations,
    Id_Contracts,
    Id_TypesPrices,
    Id_Doctors,
    Id_Doctors_exec,
    Id_Nurses,
    Id_Patients,
    Id_Users,
    Sum,
    Comment,
    CardPayment,
    AttachedImg,
    Uuid
};
}

namespace OrderTableSections {
enum Column {
    Id,
    DeletionMark,
    Id_OrderEcho,
    Cod,
    Name,
    Price
};
}

namespace OrderToolBoxIdx {
enum Idx {
    Box_organization,
    Box_patient,
    Box_commnet
};
}

namespace OrderJournal {
enum Column {
    Id,
    DeletionMark,
    AttachedImages,
    CardPayment,
    NumberDoc,
    DateDoc,
    Id_Organization,
    Id_Contract,
    PatientId,
    Id_Doctor,
    Id_User,
    OrganizationName,
    ContractName,
    PatientFullName,
    PatientIdnp,
    DoctorName,
    UserName,
    PatientSearch,
    Sum,
    Comment,
    Uuid
};
struct Item
{
    int id           = -1;
    int deletionMark = 1;

    int attachedImages = 0;
    int cardPayment    = 0;

    QString   numberDoc;
    QDateTime dateDoc;
    QString   dateDocText;  // pu afișare rapidă in model
                            // nu se efectuiaza conversia in string
    int idOrganizations = -1;
    int idContracts     = -1;
    int patientId      = -1;
    int idDoctors       = -1;
    int idUsers         = -1;

    QString organizationName;
    QString contractName;
    QString patientName;
    QString patientIdnp;
    QString doctorName;
    QString userName;
    QString patientSearch;

    double  sum = 0.0;
    QString sumText; // pu afișare rapidă in model
                     // nu se efectuiaza conversia in string
    QString comment;
    QByteArray uuid;
};
}

namespace ReportJournal {
enum Column {
    Id, DeletionMark, AttachedImages, NumberDoc, DateDoc,
    OrderId, PatientId, UserId, PatientFullName, PatientIdnp,
    OrderDescription, UserName, Conclusion, Comment, PatientSearch, Uuid
};

struct Item {
    qint64 id = -1;
    int deletionMark = 1;
    int attachedImages = 0;
    QString numberDoc;
    QDateTime dateDoc;
    QString dateDocText;
    qint64 orderId = -1;
    qint64 patientId = -1;
    qint64 userId = -1;
    QString patientName;
    QString patientIdnp;
    QString orderDescription;
    QString userName;
    QString conclusion;
    QString comment;
    QString patientSearch;
    QByteArray uuid;
};
}

namespace PricingSections {
enum Column {
    Id,
    DeletionMark,
    NumberDoc,
    DateDoc,
    Id_TypesPrices,
    Id_Organizations,
    Id_Contracts,
    Id_Users,
    Comment,
    Uuid
};
}

namespace PricingsTableSection {
enum Column {
    Id,
    DeletionMark,
    Id_Pricings,
    Cod,
    Name,
    Price
};
}

namespace PricingsJournal {

enum Column {
    Id,
    DeletionMark,
    NumberDoc,
    DateDoc,
    Id_Organizations,
    Id_Contracts,
    Id_TypesPrices,
    Id_Users,
    TypePriceName,
    OrganizationName,
    ContractName,
    UserName,
    Comment,
    Uuid
};

struct Item {
    int id            = -1;
    int deletionMark  = -1;

    QString numberDoc;
    QDateTime dateDoc;
    QString dateDocText;

    int idOrganizations = -1;
    int idContracts     = -1;
    int idTypesPrices   = -1;
    int idUsers         = -1;

    QString typePriceName;
    QString organizationName;
    QString contractName;
    QString userName;
    QString comment;
    QByteArray uuid;
};

}

namespace ReportSections {

    enum class ReportSystem {
        OrgansInternal = 0x1,
        UrinarySystem  = 0x2,
        Prostate       = 0x4,
        Gynecology     = 0x8,
        Breast         = 0x10,
        Thyroid        = 0x20,
        Gestation0     = 0x40,
        Gestation1     = 0x80,
        Gestation2     = 0x100,
        LymphNodes     = 0x200,
        Images         = 0x400,
        Video          = 0x800
    };

    Q_DECLARE_FLAGS(ReportSystems, ReportSystem)
    Q_DECLARE_OPERATORS_FOR_FLAGS(ReportSystems)

    enum class ViewExamination {
        Unknown   = 0,
        Good      = 1,
        Medium    = 2,
        Difficult = 3
    };

    inline QString viewExaminationToString(int val)
    {
        switch (val) {
        case 1: return "good";
        case 2: return "medium";
        case 3: return "difficult";
        default: return "";
        }
    }

    inline const QHash<QString, ReportSystems> CodeMap =
    {
        {"1021.",      {ReportSystem::OrgansInternal, ReportSystem::UrinarySystem}},
        {"1022.",      {ReportSystem::OrgansInternal, ReportSystem::UrinarySystem}},
        {"1023.",      {ReportSystem::OrgansInternal}},
        {"1024.",      {ReportSystem::UrinarySystem}},
        {"1025.",      {ReportSystem::Prostate}},
        {"1026.",      {ReportSystem::Gynecology}},
        {"1027.",      {ReportSystem::Gestation0}},
        {"1027.1.",    {ReportSystem::Gestation0}},
        {"1027.4.1.",  {ReportSystem::Gestation1}},
        {"1027.5.1.",  {ReportSystem::Gestation1}},
        {"1028.",      {ReportSystem::Gestation0}},
        {"1028.1.",    {ReportSystem::Gestation0}},
        {"1028.4.1.",  {ReportSystem::Gestation1}},
        {"1028.5.1.",  {ReportSystem::Gestation1}},
        {"1028.4.2.",  {ReportSystem::Gestation1}},
        {"1029.1.1.",  {ReportSystem::Gestation2}},
        {"1029.1.2.",  {ReportSystem::Gestation2}},
        {"1029.2.",    {ReportSystem::Gestation2}},
        {"1029.21.",   {ReportSystem::Gestation2}},
        {"1029.3.",    {ReportSystem::Gestation2}},
        {"1029.31.",   {ReportSystem::Gestation2}},
        {"1033.",      {ReportSystem::Gynecology}},
        {"1036.",      {ReportSystem::Thyroid}},
        {"1036.1.",    {ReportSystem::Thyroid}},
        {"1037.",      {ReportSystem::Breast}},
        {"1037.1.",    {ReportSystem::Breast}},
        {"1038.",      {ReportSystem::Prostate}},
        {"1050.19.",   {ReportSystem::UrinarySystem}},
        {"1050.20.",   {ReportSystem::UrinarySystem}},
        {"1050.22.",   {ReportSystem::Gynecology}},
        {"1050.23.",   {ReportSystem::Gynecology}},
        {"1050.25.",   {ReportSystem::Gynecology}},
        {"1050.26.",   {ReportSystem::Gynecology}},
        {"1050.31.",   {ReportSystem::Thyroid}},
        {"1050.32.",   {ReportSystem::Thyroid}},
        {"1050.34.",   {ReportSystem::Breast}},
        {"1050.35.",   {ReportSystem::Breast}},
        {"1050.52.",   {ReportSystem::LymphNodes}},
        {"1050.53.",   {ReportSystem::LymphNodes}},
        {"1050.54.",   {ReportSystem::LymphNodes}},
        {"1050.55.",   {ReportSystem::Prostate}},
        {"1050.60.",   {ReportSystem::OrgansInternal}},
        {"1050.61.",   {ReportSystem::OrgansInternal, ReportSystem::UrinarySystem}},
        {"1050.62.",   {ReportSystem::OrgansInternal, ReportSystem::UrinarySystem, ReportSystem::Gynecology}},
        {"1050.62.1.", {ReportSystem::OrgansInternal, ReportSystem::UrinarySystem, ReportSystem::Prostate}},
        {"1050.62.2.", {ReportSystem::OrgansInternal, ReportSystem::UrinarySystem, ReportSystem::Prostate, ReportSystem::Thyroid}},
        {"1050.63.",   {ReportSystem::OrgansInternal, ReportSystem::UrinarySystem, ReportSystem::Gynecology}},
        {"1050.63.1.", {ReportSystem::OrgansInternal, ReportSystem::UrinarySystem, ReportSystem::Gynecology, ReportSystem::Breast, ReportSystem::Thyroid}},
        {"1050.66.",   {ReportSystem::Gestation0}},
        {"1050.67.",   {ReportSystem::Gestation0}},
        {"1050.69.",   {ReportSystem::Gynecology}}
    };

    inline uint qHash(ReportSections::ReportSystem key, uint seed = 0)
    {
        return ::qHash(static_cast<int>(key), seed);
    }

    inline ReportSystems systemsByCode(const QString &code)
    {
        if (auto it = CodeMap.constFind(code); it != CodeMap.constEnd())
            return it.value();

        return {};
    }

    inline ReportSystems systemsByCodes(const QStringList &codes)
    {
        ReportSystems result;

        for (const QString &code : codes)
            result |= systemsByCode(code);

        return result;
    }

    inline QString systemDisplayName(ReportSystem system)
    {
        switch (system) {
        case ReportSystem::OrgansInternal:
            return QObject::tr("organe interne");
        case ReportSystem::UrinarySystem:
            return QObject::tr("sistemul urinar");
        case ReportSystem::Prostate:
            return QObject::tr("prostata");
        case ReportSystem::Gynecology:
            return QObject::tr("ginecologia");
        case ReportSystem::Breast:
            return QObject::tr("gl.mamare");
        case ReportSystem::Thyroid:
            return QObject::tr("tiroida");
        case ReportSystem::Gestation0:
            return QObject::tr("sarcina până la 11 săptămâni");
        case ReportSystem::Gestation1:
            return QObject::tr("sarcina 11-14 săptămâni");
        case ReportSystem::Gestation2:
            return QObject::tr("sarcina 15-40 săptămâni");
        case ReportSystem::LymphNodes:
            return QObject::tr("țes.moi și gangl.limfatici");
        default:
            break;
        }

        return QString();
    }

    inline QList<ReportSystem> allSystems()
    {
        return {
            ReportSystem::OrgansInternal,
            ReportSystem::UrinarySystem,
            ReportSystem::Prostate,
            ReportSystem::Gynecology,
            ReportSystem::Breast,
            ReportSystem::Thyroid,
            ReportSystem::Gestation0,
            ReportSystem::Gestation1,
            ReportSystem::Gestation2,
            ReportSystem::LymphNodes,
            ReportSystem::Images,
            ReportSystem::Video
        };
    }

    inline QStringList displayNamesFromSystems(ReportSystems systems)
    {
        QStringList result;

        for (ReportSystem system : allSystems()) {
            if (systems.testFlag(system))
                result << systemDisplayName(system);
        }

        return result;
    }
}

namespace Doppler {
    enum bloodFlow {
        Unknow,
        Normal,
        Anormal
    };
}
