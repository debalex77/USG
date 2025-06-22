#ifndef DATACONSTANTSWORKER_H
#define DATACONSTANTSWORKER_H

#include "threads/databaseprovider.h"
#include <QObject>
#include <common/cryptomanager.h>

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

    DataConstantsWorker(DatabaseProvider *provider, GeneralData &data, QObject *parent = nullptr);

public slots:
    void process();

signals:
    void finished();

private:
    GeneralData m_data;
    DatabaseProvider *m_db{nullptr};
    CryptoManager *crypto_manager;
};

#endif // DATACONSTANTSWORKER_H
