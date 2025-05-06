#ifndef COMMONSETTINGSMANAGER_H
#define COMMONSETTINGSMANAGER_H

#include <QString>
#include <QVariant>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QMap>
#include <QJsonValue>
#include <QJsonParseError>

class CommonSettingsManager
{
public:
    CommonSettingsManager(const QString &filePath) : m_filePath(filePath)
    {
        load();
    }

    QVariant getValue(const QString &reportId, const QString &key, const QVariant &defaultValue = QVariant())
    {
        return m_data.value(reportId).value(key, defaultValue);
    }

    void setValue(const QString &reportId, const QString &key, const QVariant &value)
    {
        m_data[reportId][key] = value;
    }

    void save()
    {
        QJsonObject root;
        for (auto itReport = m_data.begin(); itReport != m_data.end(); ++itReport) {
            QJsonObject reportObj;
            for (auto itKey = itReport.value().begin(); itKey != itReport.value().end(); ++itKey) {
                reportObj[itKey.key()] = QJsonValue::fromVariant(itKey.value());
            }
            root[itReport.key()] = reportObj;
        }

        QFile file(m_filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
            file.close();
        }
    }

private:
    void load()
    {
        QFile file(m_filePath);
        if (! file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
        file.close();

        if (! err.error && doc.isObject()) {
            QJsonObject root = doc.object();
            const QStringList reportIds = root.keys();
            for (const QString &reportId : reportIds) {
                QJsonObject reportObj = root.value(reportId).toObject();
                const QStringList keys = reportObj.keys();
                for (const QString &key : keys)
                    m_data[reportId][key] = reportObj.value(key).toVariant();
            }
        }
    }

private:
    QString m_filePath;
    QMap<QString, QMap<QString, QVariant>> m_data; // <id_report, <settings, value>>
};

#endif // COMMONSETTINGSMANAGER_H
