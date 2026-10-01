#ifndef LAYOUTSETTINGSMANAGER_H
#define LAYOUTSETTINGSMANAGER_H

#include "common/applicationpathscontext.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSaveFile>
#include <QSet>
#include <QStandardPaths>
#include <QVariant>
#include <QVariantMap>

#include <functional>
#include <utility>

class LayoutSettingsManager
{
public:

    LayoutSettingsManager(const QString &filePath)
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
        m_dirtyPaths.insert(raportId + QLatin1Char('/') + parts.join(QLatin1Char('/')));

        if (m_autoSave)
            save();
    }

    void setShowOnLaunchRaport(const QString &raportId)
    {
        m_json["showOnLaunch"] = raportId;
        m_dirtyPaths.insert(QStringLiteral("showOnLaunch"));
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
        m_dirtyPaths.insert(QStringLiteral("listReports"));
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
        QDir dir(ApplicationPathsContext::instance().data().reportsDirectory);
        dir.setFilter(QDir::Files | QDir::NoSymLinks);
        dir.setNameFilters({QStringLiteral("*.lrxml")});
        const QFileInfoList listFiles = dir.entryInfoList();
        for (const QFileInfo &fileInfo : listFiles) {
            // numele raportului = numele fișierului fără „.lrxml”
            const QString name = fileInfo.completeBaseName();
            if (!name.isEmpty())
                list << name;
        }
        return list;
    }

    void save()
    {
        if (m_dirtyPaths.isEmpty())
            return;

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

        // 2. MERGE: suprascriem numai valorile modificate de această
        // instanță, până la cheia imbricată exactă. Mai multe ferestre
        // pot avea simultan cache-uri ale aceluiași fișier, inclusiv mai
        // multe selectoare CatalogTableEditor pentru tipuri diferite de
        // șabloane. Rescrierea obiectului părinte ar restaura dimensiuni vechi
        // peste valorile salvate mai recent de altă fereastră.
        for (const QString &dirtyPath : std::as_const(m_dirtyPaths)) {
            const QStringList parts = dirtyPath.split(QLatin1Char('/'), Qt::SkipEmptyParts);
            if (parts.isEmpty())
                continue;

            QJsonValue localValue = m_json.value(parts.constFirst());
            for (qsizetype index = 1; index < parts.size(); ++index) {
                if (!localValue.isObject()) {
                    localValue = QJsonValue();
                    break;
                }
                localValue = localValue.toObject().value(parts.at(index));
            }

            std::function<void(QJsonObject &, qsizetype)> mergeValue;
            mergeValue = [&](QJsonObject &object, qsizetype index) {
                const QString &part = parts.at(index);
                if (index == parts.size() - 1) {
                    object.insert(part, localValue);
                    return;
                }
                QJsonObject child = object.value(part).toObject();
                mergeValue(child, index + 1);
                object.insert(part, child);
            };
            mergeValue(diskJson, 0);
        }

        // 3. Scriem rezultatul final
        QSaveFile fileOut(m_filePath);
        if (fileOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QJsonDocument doc(diskJson);
            const QByteArray json = doc.toJson(QJsonDocument::Indented);
            if (fileOut.write(json) == json.size() && fileOut.commit()) {
                m_json = diskJson;
                m_dirtyPaths.clear();
            } else {
                qWarning() << "[LayoutSettingsManager] - scrierea atomică a eșuat pentru:"
                           << m_filePath << fileOut.errorString();
            }
        } else {
            qWarning() << "[LayoutSettingsManager] - nu sunt salvate setarile în:" << m_filePath;
        }
    }

    void reload()
    {
        m_json = QJsonObject();
        m_dirtyPaths.clear();
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
                qWarning() << "[LayoutSettingsManager] - Eroare parsare JSON:" << err.errorString();

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
    QSet<QString> m_dirtyPaths;
    bool m_autoSave = true;
};

#endif // LAYOUTSETTINGSMANAGER_H
