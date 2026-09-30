#include "printimagesservice.h"

#include "core/loggingcategories.h"
#include <QImage>
#include <QPixmap>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardItem>
#include <QStandardItemModel>

namespace {

    constexpr QSize kLogoSize  {300, 50};
    constexpr QSize kStampSize {200, 200};

} // namespace

PrintImagesService::Result PrintImagesService::fillModel(QStandardItemModel* model,
                                                         const QSqlDatabase& db,
                                                         int organizationId,
                                                         int doctorId,
                                                         ModelLayout layout)
{
    Result result;

    if (!model)
        return result;

    const DoctorPrintImages doctor = loadDoctorPrintImages(db, doctorId);

    QByteArray organizationLogo;
    QByteArray organizationStamp;
    if (db.isValid() && db.isOpen() && organizationId > 0) {
        QSqlQuery query(db);
        query.prepare(QStringLiteral(R"(
            SELECT
                settings.logo,
                organizations.stamp
            FROM
                organizations
            LEFT JOIN
                organizationSettings settings
                    ON settings.organization_id = organizations.id
            WHERE
                organizations.id = ?
        )"));
        query.addBindValue(organizationId);
        if (!query.exec()) {
            qWarning(logWarning())
                << "PrintImagesService: imaginile organizației nu pot fi citite:"
                << query.lastError().text();
        } else if (query.next()) {
            organizationLogo = decodeImage(
                query.value(0),
                QStringLiteral("logo organizație ID=%1").arg(organizationId));
            organizationStamp = decodeImage(
                query.value(1),
                QStringLiteral("ștampilă organizație ID=%1").arg(organizationId));
        } else {
            qWarning(logWarning())
                << "PrintImagesService: organizația nu a fost găsită; id="
                << organizationId;
        }
    }

    // 1. Logo organizație
    QStandardItem *itLogo = mkImageItem(organizationLogo, kLogoSize);
    result.logo = itLogo->data(Qt::DisplayRole).isValid();

    // 2. Ștampila organizației
    QStandardItem *itOrg = mkImageItem(organizationStamp, kStampSize);
    result.organizationStamp = itOrg->data(Qt::DisplayRole).isValid();

    // 3. Ștampila doctorului
    QStandardItem *itDoc = mkImageItem(doctor.stamp, kStampSize);
    result.doctorStamp = itDoc->data(Qt::DisplayRole).isValid();

    // 4. Semnătura doctorului
    QStandardItem *itSig = mkImageItem(doctor.signature, kStampSize);

    result.doctorSignature = itSig->data(Qt::DisplayRole).isValid();

    model->clear();
    if (layout == ModelLayout::StatisticalReports) {
        // Șabloanele rapoartelor statistice folosesc trei coloane LimeReport:
        // 1 - logo, 2 - ștampila doctorului, 3 - semnătura doctorului.
        // Ștampila organizației nu face parte din aceste rapoarte.
        model->setColumnCount(3);
        model->appendRow({itLogo, itDoc, itSig});
        delete itOrg;
        result.organizationStamp = false;
    } else {
        // Modelele documentelor Order/Report folosesc patru coloane:
        // 1 - logo, 2 - ștampila organizației, 3 - ștampila doctorului,
        // 4 - semnătura doctorului.
        model->setColumnCount(4);
        model->appendRow({itLogo, itOrg, itDoc, itSig});
    }

    return result;
}

PrintImagesService::DoctorPrintImages PrintImagesService::loadDoctorPrintImages(const QSqlDatabase& db,
                                                                                int doctorId)
{
    DoctorPrintImages images;

    if (doctorId <= 0) {
        return images;
    }

    if (!db.isValid() || !db.isOpen()) {
        qWarning(logWarning()) << "PrintImagesService: baza de date nu este deschisă "
                                  "pentru imaginile doctorului.";
        return images;
    }

    QSqlQuery q(db);
    q.prepare(QStringLiteral(R"(
        SELECT
            signature,
            stamp
        FROM
            doctors
        WHERE
            id = ?
    )"));
    q.addBindValue(doctorId);
    if (!q.exec()) {
        qWarning(logWarning()) << "PrintImagesService: imaginile doctorului nu pot fi citite:"
                               << q.lastError().text();
        return images;
    }

    if (!q.next()) {
        qWarning(logWarning()) << "PrintImagesService: doctorul nu a fost găsit; id="
                               << doctorId;
        return images;
    }

    images.signature = decodeImage(
        q.value(0),
        QStringLiteral("semnătură doctor ID=%1").arg(doctorId));
    images.stamp = decodeImage(
        q.value(1),
        QStringLiteral("ștampilă doctor ID=%1").arg(doctorId));

    return images;
}

QByteArray PrintImagesService::decodeImage(const QVariant &value,
                                           const QString &sourceDescription)
{
    const QByteArray stored = value.toByteArray();
    if (stored.isEmpty())
        return {};

    // Compatibilitate atât cu BLOB direct, cât și cu date Base64 istorice.
    if (!QImage::fromData(stored).isNull())
        return stored;

    const QByteArray decoded = QByteArray::fromBase64(stored);
    if (!decoded.isEmpty() && !QImage::fromData(decoded).isNull())
        return decoded;

    qWarning(logWarning())
        << "PrintImagesService: imaginea nu poate fi decodificată:"
        << sourceDescription;
    return {};
}

QStandardItem *PrintImagesService::mkImageItem(const QByteArray& bytes,
                                const QSize& targetSize)
{
    auto* item = new QStandardItem;

    if (bytes.isEmpty())
        return item;

    QPixmap pixmap;

    if (!pixmap.loadFromData(bytes))
        return item;

    pixmap = pixmap.scaled(
        targetSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation);

    if (pixmap.isNull())
        return item;

    // Pentru Qt Views
    item->setData(pixmap, Qt::DecorationRole);

    // Pentru LimeReport
    item->setData(pixmap.toImage(), Qt::DisplayRole);

    return item;
}
