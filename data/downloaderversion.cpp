#include "downloaderversion.h"
#include "data/loggingcategories.h"

DownloaderVersion::DownloaderVersion(QObject *parent)
    : QObject{parent}
{
    manager = new QNetworkAccessManager();
    connect(manager, &QNetworkAccessManager::finished, this, &DownloaderVersion::onResult);
}

void DownloaderVersion::getData()
{
    QUrl url(GITHUB_URL);    // URL
    QNetworkRequest request; // trimitem solicitarea
    request.setUrl(url);     // setam URL in solicitarea
    manager->get(request);   // solicitarea propriu-zisa
}

void DownloaderVersion::onResult(QNetworkReply *reply)
{
    // daca eroarea
    if(reply->error()){
        // prezentam eroarea
        qInfo(logInfo()) << "ERROR";
        qInfo(logInfo()) << reply->errorString();
    } else {
        // in caz contrar cream fisierul
        QDir dir;
        QString path_file_version = dir.toNativeSeparators(QDir::tempPath() + "/usg_version.txt");
        QFile *file = new QFile(path_file_version);
        if (file->exists()){
#if defined(Q_OS_LINUX)
            QString str_cmd = "rm " + path_file_version;
            system(str_cmd.toStdString().c_str());
#elif defined (Q_OS_WIN)
            QString str_cmd = "del " + path_file_version;
            system(str_cmd.toStdString().c_str());
#endif
        }

        if(file->open(QFile::WriteOnly)){
            file->write(reply->readAll());  // scrim datele in fisier
            file->close();                  // inchidem fisier
            qInfo(logInfo()) << "Downloading is completed";
            emit onReady(); // emitem signal
        }
    }
}
