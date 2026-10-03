#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QMessageBox>

#include "settings/settingsrepository.h"

class BaseSqlQueryModel;
class DataBase;
class QCloseEvent;
class QComboBox;

namespace Ui { class SettingsDialog; }

class SettingsDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(DataBase &database, QWidget *parent = nullptr);
    ~SettingsDialog() override;

    void setUserId(int userId);

public slots:
    void reject() override;

signals:
    void settingsApplied();

private:
    void initializeModels();
    void initializeConnections();
    void loadUser(int userId);
    bool saveSettings();
    bool confirmDiscardChanges();
    QMessageBox::StandardButton askSaveChanges(const QString &text);
    void markModified();
    void setComboValue(QComboBox *combo, int value);
    int comboValue(const QComboBox *combo) const;
    void updateLogoPreview();
    void synchronizeOrganizationCombos(QComboBox *source, QComboBox *destination);
    SettingsRepository::PersistedSettings collectSettings() const;

private:
    Ui::SettingsDialog *ui = nullptr;
    DataBase           &m_database;
    SettingsRepository m_repository;
    SettingsRepository::PersistedSettings m_loaded;
    int m_currentUserId = -1;
    bool m_loading   = false;
    bool m_accepting = false;

    BaseSqlQueryModel *m_usersModel = nullptr;
    BaseSqlQueryModel *m_organizationsModel = nullptr;
    BaseSqlQueryModel *m_defaultOrganizationsModel = nullptr;
    BaseSqlQueryModel *m_doctorsModel = nullptr;
    BaseSqlQueryModel *m_nursesModel = nullptr;

protected:
    void closeEvent(QCloseEvent *event) override;

};

#endif // SETTINGSDIALOG_H
