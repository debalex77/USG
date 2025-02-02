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

    enum ORDER_IMAGE_VIDEO
    {
        NOT_ATTACHED,
        ATTACHED_IMAGE,
        ATTACHED_VIDEO
    };
    Q_ENUM(ORDER_IMAGE_VIDEO);

    enum ORDER_COLUMN
    {
        ORDER_ID,
        ORDER_DEL_MARK,
        ORDER_ATTACHED_IMAGE,
        ORDER_CARD_PAYMENT,
        ORDER_NUMBER_DOC,
        ORDER_DATE_DOC,
        ORDER_ID_ORGANIZATION,
        ORDER_ORGANIZATION,
        ORDER_ID_CONTRACT,
        ORDER_CONTRACT,
        ORDER_ID_PACIENT,
        ORDER_SEARCH_PACIENT,
        ORDER_PACIENT,
        ORDER_IDNP,
        ORDER_DOCTOR,
        ORDER_ID_USER,
        ORDER_USER,
        ORDER_SUM,
        ORDER_COMMENT
    };
    Q_ENUM(ORDER_COLUMN);

    enum TYPE_PRINT
    {
        OPEN_DESIGNER,
        OPEN_PREVIEW
    };

    enum PROXY_MODEL
    {
        PROXY_COLUMN_ID,
        PROXY_COLUMN_DEL_MARK,
        PROXY_ATTACHED_IMAGE,
        PROXY_CARD_PAYMENT,
        PROXY_NUMBER_DOC,
        PROXY_DATE_DOC
    };
    Q_ENUM(PROXY_MODEL);

    enum BLOOD_FLOW
    {
        FLOW_UNKNOW,
        FLOW_NORMAL,
        FLOW_ANORMAL
    };
    Q_ENUM(BLOOD_FLOW);
};

#endif // ENUMS_H
