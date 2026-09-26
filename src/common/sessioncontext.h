#ifndef SESSIONCONTEXT_H
#define SESSIONCONTEXT_H

#include <QObject>

class SessionContext final : public QObject
{
    Q_OBJECT

public:
    static SessionContext &instance();

    [[nodiscard("SessionContext::instance().userID - verifica ID utilizatorului")]]
    int userId() const;

    [[nodiscard]]
    bool hasAuthenticatedUser() const;

    void setCandidateUserId(int userId);
    void setAuthenticatedUserId(int userId);
    void clear();

signals:
    void userIdChanged(int userId);
    void authenticationChanged(bool authenticated);

private:
    explicit SessionContext(QObject *parent = nullptr);

    int m_userId = -1;
    bool m_authenticated = false;
};

#endif // SESSIONCONTEXT_H
