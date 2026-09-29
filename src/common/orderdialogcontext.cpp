#include "orderdialogcontext.h"

const OrderDocumentContextData &OrderDialogContext::data() const
{
    return m_data;
}

void OrderDialogContext::setData(const OrderDocumentContextData &data)
{
    m_data = data;
}

void OrderDialogContext::clear()
{
    m_data = {};
}
