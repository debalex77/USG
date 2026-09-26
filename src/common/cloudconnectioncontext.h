#ifndef CLOUDCONNECTIONCONTEXT_H
#define CLOUDCONNECTIONCONTEXT_H

#include <QObject>
#include <QReadWriteLock>
#include <QString>

struct CloudConnectionData
{
    QString hostName;
    QString databaseName;
    int port = 3306;
    QString connectionOptions;
    QString userName;
    QString password;
    bool configured = false;
    bool enabled = false;

    [[nodiscard]]
    bool isUsable() const
    {
        return configured && enabled
               && !hostName.trimmed().isEmpty()
               && !databaseName.trimmed().isEmpty()
               && port > 0
               && !userName.trimmed().isEmpty()
               && !password.isEmpty();
    }

    friend bool operator==(const CloudConnectionData &,
                           const CloudConnectionData &) = default;
};

class CloudConnectionContext final : public QObject
{
    Q_OBJECT

public:
    static CloudConnectionContext &instance();

    [[nodiscard("CloudConnectionContext::instance().data() - verifica corectitudinea datelor")]]
    CloudConnectionData data() const;

    void setData(const CloudConnectionData &data);
    void clear();

signals:
    void changed(const CloudConnectionData &data);

private:
    explicit CloudConnectionContext(QObject *parent = nullptr);

    mutable QReadWriteLock m_lock;
    CloudConnectionData m_data;
};

Q_DECLARE_METATYPE(CloudConnectionData)

#endif // CLOUDCONNECTIONCONTEXT_H
