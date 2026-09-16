#ifndef ORDERINVESTIGATIONMODEL_H
#define ORDERINVESTIGATIONMODEL_H

#include "baseabstractmodel.h"

class OrderInvestigationModel final : public BaseAbstractModel
{
    Q_OBJECT
public:
    enum class Kind {
        Available,
        Selected
    };

    explicit OrderInvestigationModel(Kind kind, QObject *parent = nullptr);

    Kind kind() const;

private:
    Kind m_kind;
};

#endif // ORDERINVESTIGATIONMODEL_H
