#ifndef DOWNLOADERVERSION_H
#define DOWNLOADERVERSION_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDir>
#include <QFile>

#define GITHUB_URL "https://raw.githubusercontent.com/debalex77/USG/master/version.txt"

class DownloaderVersion : public QObject
{
    Q_OBJECT
public:
    explicit DownloaderVersion(QObject *parent = nullptr);

signals:
    void onReady();

public slots:
    void getData();                       // initierea solicitarii de a obtine date
    void onResult(QNetworkReply *reply);  // Slot pentru procesarea răspunsului despre datele primite

private:
    QNetworkAccessManager *manager; // manager de retea
};

#endif // DOWNLOADERVERSION_H
