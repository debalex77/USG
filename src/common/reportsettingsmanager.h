#ifndef REPORTSETTINGSMANAGER_H
#define REPORTSETTINGSMANAGER_H

#include "common/globals.h"
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

    bool jsonContainsData(const QString &reportId)
    {
        return m_json.contains(reportId);
    }

    QJsonObject getJsonObject(const QString &docName, const QJsonObject &defaultObject = QJsonObject()) const
    {
        const QJsonValue v = m_json.value(docName);

        if (!v.isObject())
            return defaultObject;

        return v.toObject();
    }

    QVariant getValue(const QString &raportId,
                      const QString &key,
                      const QVariant &defaultValue = QVariant()) const
    {
        const QJsonValue root = m_json.value(raportId);
        if (!root.isObject())
            return defaultValue;

        const QStringList parts = key.split('/', Qt::SkipEmptyParts);
        if (parts.isEmpty())
            return defaultValue;

        QJsonValue current = root;

        for (const QString &part : parts) {
            if (!current.isObject())
                return defaultValue;

            const QJsonObject obj = current.toObject();
            if (!obj.contains(part))
                return defaultValue;

            current = obj.value(part);
        }

        if (current.isUndefined() || current.isNull())
            return defaultValue;

        return current.toVariant();
    }

    void setValue(const QString &raportId, const QString &key, const QVariant &value)
    {
        QJsonObject raportObj = m_json.value(raportId).toObject();
        const QStringList parts = key.split('/', Qt::SkipEmptyParts);

        if (parts.isEmpty())
            return;

        std::function<void(QJsonObject&, int)> setNested;
        setNested = [&](QJsonObject &obj, int index)
        {
            const QString &part = parts.at(index);

            if (index == parts.size() - 1) {
                obj.insert(part, QJsonValue::fromVariant(value));
                return;
            }

            QJsonObject childObj = obj.value(part).toObject();
            setNested(childObj, index + 1);
            obj.insert(part, childObj);
        };

        setNested(raportObj, 0);
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
        QDir dir(globals().reportsPath);
        dir.setFilter(QDir::Files | QDir::NoSymLinks);
        QFileInfoList listFiles = dir.entryInfoList();
        for (int n = 0; n < listFiles.size(); n++) {
            QFileInfo fileInfo = listFiles.at(n);
            list << fileInfo.baseName();
        }
        return list;
    }

    void save()
    {
        // 1. Încarcă ce există pe disc
        QJsonObject diskJson;

        QFile fileIn(m_filePath);
        if (fileIn.exists() && fileIn.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(fileIn.readAll(), &err);
            fileIn.close();

            if (err.error == QJsonParseError::NoError && doc.isObject())
                diskJson = doc.object();
        }

        // 2. MERGE: suprascriem doar cheile locale
        for (auto it = m_json.begin(); it != m_json.end(); ++it) {
            diskJson[it.key()] = it.value();
        }

        // 3. Scriem rezultatul final
        QFile fileOut(m_filePath);
        if (fileOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QJsonDocument doc(diskJson);
            fileOut.write(doc.toJson(QJsonDocument::Indented));
            fileOut.close();
        } else {
            qWarning() << "[ReportSettingsManager] - nu sunt salvate setarile în:" << m_filePath;
        }

        // 4. actualizăm cache-ul intern
        m_json = diskJson;
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
                qWarning() << "[ReportSettingsManager] - Eroare parsare JSON:" << err.errorString();

            // ne conducem dupa continut 'showOnLaunch' care este
            // doar in fisierul 'report_settings.json', daca nu contine, atunci
            // fisierul 'table_settings.json'
            if (m_json.contains("showOnLaunch") &&
                m_json.value("listReports").toVariant().toStringList().isEmpty())
                setListReports();
        }
    }

    QString m_filePath;
    QJsonObject m_json;
    bool m_autoSave = true;
};

#endif // REPORTSETTINGSMANAGER_H
