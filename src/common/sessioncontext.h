#ifndef SESSIONCONTEXT_H
#define SESSIONCONTEXT_H

#include <QObject>
#include <QUuid>

class SessionContext final : public QObject
{
    Q_OBJECT

public:
    static SessionContext &instance();

    [[nodiscard("SessionContext::instance().userID - verifica ID utilizatorului")]]
    int userId() const;

    // UUID-ul utilizatorului autentificat (users.uuid); nul pentru un
    // utilizator candidat sau când baza nu are încă această coloană.
    [[nodiscard]]
    QUuid userUuid() const;

    [[nodiscard]]
    bool hasAuthenticatedUser() const;

    void setCandidateUserId(int userId);
    void setAuthenticatedUserId(int userId, const QUuid &userUuid = QUuid());
    void clear();

signals:
    void userIdChanged(int userId);
    void authenticationChanged(bool authenticated);

private:
    explicit SessionContext(QObject *parent = nullptr);

    int m_userId = -1;
    QUuid m_userUuid;
    bool m_authenticated = false;
};

#endif // SESSIONCONTEXT_H
