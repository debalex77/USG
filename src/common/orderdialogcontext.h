#ifndef ORDERDIALOGCONTEXT_H
#define ORDERDIALOGCONTEXT_H

struct OrderDocumentContextData
{
    int orderId        = 0;
    // Organizația care a trimis pacientul, salvată în document.
    // Nu reprezintă organizația utilizată în antetul formularului tipărit.
    int organizationId = 0;
    int contractId     = 0;
    int priceTypeId    = 0;

    int referringDoctorId = 0;
    // Doctorul documentului; identitatea de tipar este doctorul implicit din
    // UserPreference.
    int executingDoctorId = 0;
    int nurseId = 0;

    int patientId = 0;
    int authorUserId = 0;
    int deletionMark = 0;

    [[nodiscard]] bool isValid() const
    {
        return orderId > 0
               && organizationId > 0
               && patientId > 0
               && authorUserId > 0;
    }
};

class OrderDialogContext final
{
public:
    [[nodiscard("OrderDocumentContextData::data() verifica datele")]]
    const OrderDocumentContextData &data() const;

    void setData(const OrderDocumentContextData &data);
    void clear();

private:
    OrderDocumentContextData m_data;
};

#endif // ORDERDIALOGCONTEXT_H
