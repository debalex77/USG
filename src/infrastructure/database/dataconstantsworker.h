#ifndef DATACONSTANTSWORKER_H
#define DATACONSTANTSWORKER_H

#include <infrastructure/database/databaseprovider.h>
#include "common/cloudconnectioncontext.h"
#include "common/doctorcontext.h"
#include "common/organizationcontext.h"
#include <QObject>
#include <infrastructure/security/cryptomanager.h>

class DataConstantsWorker : public QObject
{
    Q_OBJECT
public:

    struct GeneralData
    {
        int id_user = 0;
        int id_organization = 0;
        int id_doctor = 0;
        bool thisMySQL = false;
    };

    explicit DataConstantsWorker(const GeneralData &data, QObject *parent = nullptr);

public slots:
    void process();

signals:
    void finished(bool success,
                  const CloudConnectionData &cloudConnection,
                  int organizationId,
                  int defaultDoctorId,
                  int defaultNurseId,
                  const QString &ultrasoundDeviceName,
                  const QByteArray &logoData,
                  const OrganizationContextData &organizationData,
                  const DoctorContextData &doctorData);

private:
    GeneralData m_data;
    CryptoManager *crypto_manager;
};

#endif // DATACONSTANTSWORKER_H
