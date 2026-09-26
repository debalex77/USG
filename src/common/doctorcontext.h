#ifndef DOCTORCONTEXT_H
#define DOCTORCONTEXT_H

#include <QByteArray>
#include <QObject>
#include <QString>

struct DoctorContextData
{
    QString fullName;
    QString abbreviatedName;

    QByteArray stampData;
    QByteArray signatureData;

    friend bool operator==(const DoctorContextData &,
                           const DoctorContextData &) = default;
};

class DoctorContext final : public QObject
{
    Q_OBJECT

public:
    static DoctorContext &instance();

    [[nodiscard("DoctorContext::instance().data() - verifica corectitudinea datelor")]]
    const DoctorContextData &data() const;

    void setData(const DoctorContextData &data);
    void clear();

signals:
    void changed(const DoctorContextData &data);

private:
    explicit DoctorContext(QObject *parent = nullptr);

    DoctorContextData m_data;
};

Q_DECLARE_METATYPE(DoctorContextData)

#endif // DOCTORCONTEXT_H
