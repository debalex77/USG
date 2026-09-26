#ifndef ORGANIZATIONCONTEXT_H
#define ORGANIZATIONCONTEXT_H

#include <QByteArray>
#include <QObject>
#include <QString>

struct OrganizationContextData
{
    QString name;
    QString email;
    QString site;
    QString phone;
    QString address;
    QByteArray stampData;

    friend bool operator==(const OrganizationContextData &,
                           const OrganizationContextData &) = default;
};

class OrganizationContext final : public QObject
{
    Q_OBJECT

public:
    static OrganizationContext &instance();

    [[nodiscard("OrganizationContext::instance().data() - verifica corectitudinea datelor")]]
    const OrganizationContextData &data() const;

    void setData(const OrganizationContextData &data);
    void clear();

signals:
    void changed(const OrganizationContextData &data);

private:
    explicit OrganizationContext(QObject *parent = nullptr);

    OrganizationContextData m_data;
};

Q_DECLARE_METATYPE(OrganizationContextData)

#endif // ORGANIZATIONCONTEXT_H
