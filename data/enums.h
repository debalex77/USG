#ifndef ENUMS_H
#define ENUMS_H

#include "qobjectdefs.h"
#include "qtmetamacros.h"

class Enums
{
    Q_GADGET
public:

    enum IDX
    {
        IDX_UNKNOW   = -1,
        IDX_WRITE    = 0,
        IDX_DELETION = 1,
        IDX_POST     = 2
    };

    enum PRICING_COLUMN
    {
        PRICING_COLUMN_ID,
        PRICING_COLUMN_DEL_MARK,
        PRICING_COLUMN_ID_PRICINGS,
        PRICING_COLUMN_COD,
        PRICING_COLUMN_NAME,
        PRICING_COLUMN_PRICE
    };
    Q_ENUM(PRICING_COLUMN);

    enum ORDER_PAYMENT
    {
        PAYMENT_CASH,
        PAYMENT_CARD,
        PAYMENT_TRANSFER
    };
    Q_ENUM(ORDER_PAYMENT);

    enum TYPE_PRINT
    {
        OPEN_DESIGNER,
        OPEN_PREVIEW
    };
};

#endif // ENUMS_H
