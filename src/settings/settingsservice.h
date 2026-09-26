#ifndef SETTINGSSERVICE_H
#define SETTINGSSERVICE_H

#include <QObject>

#include "settings/settingstypes.h"

class SettingsService final : public QObject
{
    Q_OBJECT

public:
    static SettingsService &instance();

    [[nodiscard("SettingsService::instance().application() - verifica datele setarilor pentru aplicatie")]]
    const Settings::ApplicationPreferences &application() const;

    [[nodiscard("SettingsService::instance().organization() - verifica datele setarilor pentru organizatie")]]
    const Settings::OrganizationSettings &organization() const;

    [[nodiscard("SettingsService::instance().user() - verifica datele setarilor pentru user-lui")]]
    const Settings::UserPreferencesData &user() const;

    [[nodiscard("SettingsService::instance().synchronization() - verifica datele setarilor pentru syncronizare")]]
    const Settings::SynchronizationSettings &synchronization() const;

    [[nodiscard("SettingsService::instance().snapshot() - verifica datele snapshot-lui")]]
    Settings::Snapshot snapshot() const;

    void setApplication(const Settings::ApplicationPreferences &preferences);
    void setOrganization(const Settings::OrganizationSettings &settings);
    void setUser(const Settings::UserPreferencesData &preferences);
    void setSynchronization(const Settings::SynchronizationSettings &settings);
    void setSnapshot(const Settings::Snapshot &snapshot);

signals:
    void applicationChanged(const Settings::ApplicationPreferences &preferences);
    void organizationChanged(const Settings::OrganizationSettings &settings);
    void userChanged(const Settings::UserPreferencesData &preferences);
    void synchronizationChanged(const Settings::SynchronizationSettings &settings);
    void printMenuModeChanged(Settings::PrintMenuMode mode);

private:
    explicit SettingsService(QObject *parent = nullptr);

    Settings::Snapshot m_snapshot;
};

#endif // SETTINGSSERVICE_H
