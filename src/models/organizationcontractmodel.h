#ifndef ORGANIZATIONCONTRACTMODEL_H
#define ORGANIZATIONCONTRACTMODEL_H

#include "baseabstractmodel.h"

class OrganizationContractModel final : public BaseAbstractModel
{
    Q_OBJECT

public:
    explicit OrganizationContractModel(QObject *parent = nullptr);

    void setMainContractId(int id);

private:
    int m_mainContractId = 0;
};

#endif // ORGANIZATIONCONTRACTMODEL_H
