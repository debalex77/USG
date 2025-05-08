#ifndef REPORTSETTINGSMANAGER_H
#define REPORTSETTINGSMANAGER_H

#include "data/globals.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QVariant>
#include <QVariantMap>

class ReportSettingsManager
{
public:

    ReportSettingsManager(const QString &filePath)
        : m_filePath(filePath)
    {
        load();
    }

    QVariant getValue(const QString &raportId, const QString &key, const QVariant &defaultValue = QVariant()) const
    {
        if (!m_json.contains(raportId))
            return defaultValue;
        QJsonObject raportObj = m_json.value(raportId).toObject();
        return raportObj.value(key).toVariant();
    }

    void setValue(const QString &raportId, const QString &key, const QVariant &value)
    {
        QJsonObject raportObj = m_json.value(raportId).toObject();
        raportObj.insert(key, QJsonValue::fromVariant(value));
        m_json[raportId] = raportObj;

        if (m_autoSave)
            save();
    }

    void setShowOnLaunchRaport(const QString &raportId)
    {
        m_json["showOnLaunch"] = raportId;
        if (m_autoSave)
            save();
    }

    QString showOnLaunchRaportId() const
    {
        return m_json.value("showOnLaunch").toString();
    }

    void setListReports()
    {
        m_json["listReports"] = QJsonValue::fromVariant(getListReportsFromDirectory());
        if (m_autoSave)
            save();
    }

    QStringList getListReports()
    {
        return m_json.value("listReports").toVariant().toStringList();
    }

    QStringList getListReportsFromDirectory()
    {
        QStringList list;
        QDir dir(globals().pathReports);
        dir.setFilter(QDir::Files | QDir::NoSymLinks);
        QFileInfoList listFiles = dir.entryInfoList();
        for (int n = 0; n < listFiles.size(); n++) {
            QFileInfo fileInfo = listFiles.at(n);
            list << fileInfo.baseName();
        }
        return list;
    }

    void save() const
    {
        QFile file(m_filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QJsonDocument doc(m_json);
            file.write(doc.toJson(QJsonDocument::Indented));
            file.close();
        } else {
            qWarning() << "[ReportSettingsManager] - nu sunt salvate setarile în:" << m_filePath;
        }
    }

    void reload()
    {
        m_json = QJsonObject();
        load();
    }

    void setAutoSave(bool enabled)
    {
        m_autoSave = enabled;
    }

private:

    void load()
    {
        QFile file(m_filePath);
        if (!file.exists())
            return;

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
            file.close();

            if (err.error == QJsonParseError::NoError && doc.isObject())
                m_json = doc.object();
            else
                qWarning() << "Eroare parsare JSON:" << err.errorString();

            if (m_json.value("listReports").toVariant().toStringList().isEmpty())
                setListReports();
        }
    }

    QString m_filePath;
    QJsonObject m_json;
    bool m_autoSave = false;
};

#endif // REPORTSETTINGSMANAGER_H
