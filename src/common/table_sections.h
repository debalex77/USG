#pragma once

#include <algorithm>

#include <QDateTime>
#include <QCoreApplication>
#include <QHash>
#include <QSet>
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

    enum class ConsentType {
        NonInvasive         = 0x1,
        Endocavitary        = 0x2,
        ObstetricScreening  = 0x4,
        Invasive            = 0x8
    };

    Q_DECLARE_FLAGS(ConsentTypes, ConsentType)
    Q_DECLARE_OPERATORS_FOR_FLAGS(ConsentTypes)

    // Clasificarea codurilor din investig_2024.xml pentru textul consimțământului.
    // Valorile păstrează sistemele de raport asociate codului, dacă acestea există.
    inline const QHash<QString, ReportSystems> InvasiveConsentCodeMap =
    {
        {"1050.2.", {ReportSystem::Prostate}},
        {"1050.3.", {ReportSystem::UrinarySystem}},
        {"1050.4.", {ReportSystem::UrinarySystem}},
        {"1050.5.", {ReportSystem::UrinarySystem}},
        {"1050.6.", {ReportSystem::Prostate}},
        {"1050.8.", {ReportSystem::UrinarySystem}}
    };

    inline const QHash<QString, ReportSystems> EndocavitaryConsentCodeMap =
    {
        {"1027.5.1.", {ReportSystem::Gestation1}},
        {"1028.5.1.", {ReportSystem::Gestation1}},
        {"1038.", {ReportSystem::Prostate}},
        {"1039.", {ReportSystem::Prostate}},
        {"1050.25.", {ReportSystem::Gynecology}},
        {"1050.26.", {ReportSystem::Gynecology}},
        {"1050.37.", {ReportSystem::Prostate}},
        {"1050.38.", {ReportSystem::Prostate}},
        {"1050.39.", {ReportSystem::Prostate}},
        {"1050.63.", {ReportSystem::OrgansInternal,
                       ReportSystem::UrinarySystem,
                       ReportSystem::Gynecology}},
        {"1050.67.", {ReportSystem::Gestation0}},
        {"1050.69.", {ReportSystem::Gynecology}}
    };

    inline const QHash<QString, ReportSystems> ObstetricScreeningConsentCodeMap =
    {
        {"1027.4.1.", {ReportSystem::Gestation1}},
        {"1027.5.1.", {ReportSystem::Gestation1}},
        {"1028.4.1.", {ReportSystem::Gestation1}},
        {"1028.5.1.", {ReportSystem::Gestation1}},
        {"1029.1.1.", {ReportSystem::Gestation2}},
        {"1029.1.2.", {ReportSystem::Gestation2}},
        {"1029.2.", {ReportSystem::Gestation2}},
        {"1029.3.", {ReportSystem::Gestation2}},
        {"1029.4.", {ReportSystem::Gestation0}},
        {"1030.1.", {ReportSystem::Gestation2}},
        {"1049.1.", {ReportSystem::Gestation2}},
        {"1050.66.", {ReportSystem::Gestation0}},
        {"1050.67.", {ReportSystem::Gestation0}}
    };

    inline const QHash<QString, ReportSystems> NonInvasiveConsentCodeMap =
    {
        {"1041.", {}}, {"1041.1.", {}}, {"1044.", {}}, {"1045.", {}},
        {"1046.", {}}, {"1047.", {}}, {"1048.", {}}, {"1049.", {}},
        {"1049.2.", {}}, {"1050.", {}}, {"1050.10.", {ReportSystem::OrgansInternal}},
        {"1050.11.", {ReportSystem::OrgansInternal}},
        {"1050.12.", {ReportSystem::OrgansInternal}},
        {"1050.14.", {ReportSystem::OrgansInternal}},
        {"1050.15.", {ReportSystem::OrgansInternal}},
        {"1050.17.", {ReportSystem::OrgansInternal}},
        {"1050.18.", {ReportSystem::OrgansInternal}},
        {"1050.19.", {ReportSystem::UrinarySystem}},
        {"1050.20.", {ReportSystem::UrinarySystem}},
        {"1050.21.", {ReportSystem::UrinarySystem}},
        {"1050.22.", {ReportSystem::Gynecology}},
        {"1050.23.", {ReportSystem::Gynecology}},
        {"1050.24.", {ReportSystem::Gynecology}},
        {"1050.27.", {ReportSystem::Gynecology}},
        {"1050.29.", {ReportSystem::UrinarySystem}},
        {"1050.30.", {ReportSystem::UrinarySystem}},
        {"1050.31.", {ReportSystem::Thyroid}},
        {"1050.32.", {ReportSystem::Thyroid}},
        {"1050.33.", {ReportSystem::Thyroid}},
        {"1050.34.", {ReportSystem::Breast}},
        {"1050.35.", {ReportSystem::Breast}},
        {"1050.36.", {ReportSystem::Breast}},
        {"1050.40.", {}}, {"1050.41.", {}}, {"1050.43.", {}},
        {"1050.44.", {}}, {"1050.45.", {}}, {"1050.46.", {}},
        {"1050.47.", {}}, {"1050.49.", {}}, {"1050.50.", {}},
        {"1050.51.", {}}, {"1050.52.", {ReportSystem::LymphNodes}},
        {"1050.53.", {ReportSystem::LymphNodes}},
        {"1050.54.", {ReportSystem::LymphNodes}},
        {"1050.55.", {ReportSystem::Prostate}},
        {"1050.56.", {ReportSystem::UrinarySystem}}, {"1050.59.", {}},
        {"1050.60.", {ReportSystem::OrgansInternal}},
        {"1050.61.", {ReportSystem::OrgansInternal, ReportSystem::UrinarySystem}},
        {"1050.62.", {ReportSystem::OrgansInternal,
                       ReportSystem::UrinarySystem,
                       ReportSystem::Gynecology}},
        {"1050.68.", {ReportSystem::Gynecology}}, {"1050.70.", {}},
        {"1050.71.", {}}
    };

    inline const QSet<QString> TransvaginalConsentCodes = {
        "1027.5.1.", "1028.5.1.", "1050.25.", "1050.26.",
        "1050.63.", "1050.67.", "1050.69."
    };

    inline const QSet<QString> TransrectalConsentCodes = {
        "1038.", "1039.", "1050.37.", "1050.38.", "1050.39."
    };

    inline size_t qHash(ReportSections::ReportSystem key, size_t seed = 0) noexcept
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

    inline ConsentTypes consentTypesByCodes(const QStringList &codes)
    {
        ConsentTypes result;

        for (const QString &code : codes) {
            const QString normalizedCode = code.trimmed();
            bool classified = false;

            if (InvasiveConsentCodeMap.contains(normalizedCode)) {
                result |= ConsentType::Invasive;
                classified = true;
            }
            if (EndocavitaryConsentCodeMap.contains(normalizedCode)) {
                result |= ConsentType::Endocavitary;
                classified = true;
            }
            if (ObstetricScreeningConsentCodeMap.contains(normalizedCode)) {
                result |= ConsentType::ObstetricScreening;
                classified = true;
            }
            if (NonInvasiveConsentCodeMap.contains(normalizedCode) || !classified)
                result |= ConsentType::NonInvasive;
        }

        return result;
    }

    inline QString informedConsentTextByCodes(const QStringList &codes)
    {
        const ConsentTypes consentTypes = consentTypesByCodes(codes);
        const auto trConsent = [](const char *text) {
            return QCoreApplication::translate("OrderDialog", text);
        };

        if (consentTypes.testFlag(ConsentType::Invasive)) {
            return trConsent("Am fost informat(ă), într-un limbaj accesibil, despre scopul, modul de "
                             "efectuare, beneficiile, riscurile și limitele procedurii indicate mai sus, "
                             "precum și despre alternativele disponibile și consecințele refuzului.\n\n"
                             "Am fost informat(ă) despre posibilele riscuri și complicații ale procedurii, "
                             "inclusiv durere sau disconfort, sângerare, infecție, lezarea structurilor "
                             "învecinate și, în cazuri rare, necesitatea unor intervenții medicale "
                             "suplimentare.\n\n"
                             "Am avut posibilitatea de a adresa întrebări și am primit răspunsuri la "
                             "acestea. Îmi exprim liber consimțământul pentru efectuarea procedurii "
                             "indicate mai sus.");
        }

        const bool transvaginal = std::any_of(codes.cbegin(), codes.cend(), [](const QString &code) {
            return TransvaginalConsentCodes.contains(code.trimmed());
        });
        const bool transrectal = std::any_of(codes.cbegin(), codes.cend(), [](const QString &code) {
            return TransrectalConsentCodes.contains(code.trimmed());
        });
        const QString endocavitaryAccess = transvaginal && transrectal
            ? trConsent("în vagin sau în rect")
            : (transrectal ? trConsent("în rect") : trConsent("în vagin"));

        const QString endocavitaryParagraph =
            trConsent("Am fost informat(ă) că examinarea se efectuează prin introducerea unui "
                      "transductor ecografic protejat corespunzător %1 și că aceasta poate provoca "
                      "un disconfort temporar. Examinarea poate fi întreruptă la solicitarea mea.")
                .arg(endocavitaryAccess);

        if (consentTypes.testFlag(ConsentType::ObstetricScreening)) {
            QString consent = trConsent(
                "Am fost informată despre scopul, modul de efectuare, beneficiile și limitele "
                "examinării ecografice a sarcinii.\n"
                "Înțeleg că examinarea are ca scop evaluarea sarcinii și, în funcție de vârsta "
                "gestațională și tipul examinării, evaluarea dezvoltării și anatomiei fetale și "
                "depistarea unor eventuale anomalii. Nu toate malformațiile și anomaliile fetale "
                "pot fi identificate ecografic. Unele pot deveni evidente numai ulterior în "
                "evoluția sarcinii, iar un rezultat ecografic normal nu garantează absența unei "
                "patologii fetale. Ecografia nu poate exclude toate anomaliile cromozomiale sau "
                "sindroamele genetice.\n\n");

            if (consentTypes.testFlag(ConsentType::Endocavitary))
                consent += endocavitaryParagraph + QStringLiteral("\n\n");

            consent += trConsent(
                "Utilizarea Doppler: Am fost informată că examinarea poate include utilizarea "
                "modurilor Doppler pentru evaluarea circulației materne și/sau fetale. Conform "
                "datelor disponibile, nu au fost raportate efecte adverse asupra fătului în urma "
                "utilizării diagnostice a ultrasunetelor. Expunerea este limitată la timpul și "
                "nivelul de energie necesare obținerii informației medicale, conform principiului "
                "ALARA.\n\n"
                "Am avut posibilitatea de a adresa întrebări și am primit răspunsuri la acestea. "
                "Îmi exprim liber consimțământul pentru efectuarea examinării ecografice "
                "indicate mai sus.");
            return consent;
        }

        if (consentTypes.testFlag(ConsentType::Endocavitary)) {
            return trConsent("Am fost informat(ă), într-un limbaj accesibil, despre scopul, modul de "
                             "efectuare și limitele investigației ecografice indicate mai sus.\n\n%1\n\n"
                             "Înțeleg că ecografia nu poate identifica sau exclude toate patologiile, "
                             "iar rezultatul poate fi influențat de particularitățile anatomice și "
                             "condițiile examinării.\n\n"
                             "Am avut posibilitatea de a adresa întrebări și am primit răspunsuri la "
                             "acestea. Îmi exprim liber consimțământul pentru efectuarea examinării "
                             "endocavitare indicate mai sus.")
                .arg(endocavitaryParagraph);
        }

        return trConsent("Am fost informat(ă), într-un limbaj accesibil, despre scopul, modul de efectuare "
                         "și limitele investigației ecografice indicate mai sus.\n\n"
                         "Înțeleg că rezultatul examinării poate fi influențat de particularitățile "
                         "anatomice, pregătirea pacientului și condițiile de examinare și că ecografia "
                         "nu poate identifica sau exclude toate patologiile.\n\n"
                         "Am avut posibilitatea de a adresa întrebări și am primit răspunsuri la acestea. "
                         "Îmi exprim liber consimțământul pentru efectuarea investigației ecografice "
                         "indicate mai sus.");
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
