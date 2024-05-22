#include "downloader.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QDir>

Downloader::Downloader(QObject* parent) :
    BaseClass(parent)
{
    connect(&m_manager, &QNetworkAccessManager::finished, this, &Downloader::onReply);
}

bool Downloader::get(const QString& targetFolder, const QUrl& url)
{
    if (targetFolder.isEmpty() || url.isEmpty())
        return false;

    m_file = new QFile(targetFolder + QDir::separator() + url.fileName());
    if (!m_file->open(QIODevice::WriteOnly)) {
        delete m_file;
        m_file = nullptr;
        return false;
    }

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true); // permitem redirectionarea
    m_currentReply = m_manager.get(request);                               // lansam descarcarea

    // conectarea pu citirea datelor si progresului
    connect(m_currentReply, &QNetworkReply::readyRead, this, &Downloader::onReadyRead);
    connect(m_currentReply, &QNetworkReply::downloadProgress, this, &Downloader::updateDownloadProgress);
    return true;
}

void Downloader::onReadyRead()
{
    if (m_file)
        m_file->write(m_currentReply->readAll());
}

void Downloader::cancelDownload()
{
    if (m_currentReply)
        m_currentReply->abort();
}

void Downloader::onReply(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        m_file->flush();
        m_file->close();
    } else {
        qInfo(logWarning()) << "ERROR";
        qInfo(logWarning()) << reply->errorString();
        m_file->remove();
    }

    delete m_file;
    m_file = nullptr;
    reply->deleteLater();

    qInfo(logInfo()) << "Downloading new version App is completed.";
    emit finishedDownload();
}
