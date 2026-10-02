#ifndef REPORTIMAGESEXPORTER_H
#define REPORTIMAGESEXPORTER_H

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QStringList>

#include <functional>

// Salvează imaginile atașate unui raport ecografic (imagesReports) ca fișiere
// în directorul de export pentru e-mail. Folosit de DocEmailExporterWorker și
// ReportsEmailExporterWorker; conexiunea este cea a bazei imaginilor
// (db_image la SQLite, conexiunea principală la MariaDB).
class ReportImagesExporter final
{
    Q_DECLARE_TR_FUNCTIONS(ReportImagesExporter)

public:
    struct Request {
        qint64 orderId = 0;
        qint64 reportId = 0;
        QString reportNumber; // folosit în numele fișierelor
        QString directory;
    };

    struct Result {
        bool success = true; // false la eroare SQL sau la o imagine nesalvată
        QStringList files;
        QStringList errors;
    };

    using Progress = std::function<void(const QString &text)>;

    [[nodiscard]]
    static Result exportImages(QSqlDatabase &connection,
                               const Request &request,
                               const Progress &progress = {});

    // Înlocuiește caracterele nepermise în numele fișierelor.
    [[nodiscard]]
    static QString fileSafeDocumentNumber(QString number);
};

#endif // REPORTIMAGESEXPORTER_H
