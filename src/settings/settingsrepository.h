#ifndef SETTINGSREPOSITORY_H
#define SETTINGSREPOSITORY_H

#include <QStringList>
#include <QVariantMap>

#include "settings/settingstypes.h"

class DataBase;

class SettingsRepository final
{
public:
    struct PersistedSettings {
        Settings::Snapshot values;
    };

    struct LoadResult {
        PersistedSettings data;
        QString error;
        bool found = false;

        [[nodiscard]]
        bool isValid() const
        {
            return error.isEmpty();
        }
    };

    explicit SettingsRepository(DataBase &database);

    [[nodiscard("SettingsRepository.loadForUser(userID) - verifica ID userului")]]
    LoadResult loadForUser(int userId) const;

    bool saveForUser(const PersistedSettings &settings, QStringList &errors) const;

private:
    bool hasNewSchema() const;
    bool recordExists(const QString &tableName, const QString &keyColumn,
                      int keyValue, bool &exists, QStringList &errors) const;
    bool upsert(const QString &tableName, const QString &keyColumn,
                int keyValue, const QVariantMap &values,
                QStringList &errors) const;

    DataBase &m_database;
};

#endif // SETTINGSREPOSITORY_H
