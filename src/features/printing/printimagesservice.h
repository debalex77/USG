#ifndef PRINTIMAGESSERVICE_H
#define PRINTIMAGESSERVICE_H

#include <QByteArray>
#include <QSize>
#include <QSqlDatabase>

class QStandardItem;
class QStandardItemModel;

class PrintImagesService
{
public:
    struct Result {
        bool logo              = false;
        bool organizationStamp = false;
        bool doctorStamp       = false;
        bool doctorSignature   = false;
    };

    struct DoctorPrintImages {
        QByteArray signature;
        QByteArray stamp;
    };

    static Result fillModel(QStandardItemModel *model,
                            const QSqlDatabase &db,
                            int organizationId,
                            int doctorId);

    static DoctorPrintImages loadDoctorPrintImages(const QSqlDatabase& db,
                                                   int doctorId);

private:

    PrintImagesService() = delete;
    static QStandardItem* mkImageItem(const QByteArray& bytes,
                                      const QSize& targetSize);
    static QByteArray decodeImage(const QVariant &value,
                                  const QString &sourceDescription);
};

#endif // PRINTIMAGESSERVICE_H
