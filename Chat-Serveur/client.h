#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <QTcpServer>
#include <QDataStream>
#include <QTime>
#include <QTimer>
#include <QCryptographicHash>

#include "fenprincipale.h"
#include "opcode.h"
#include "../shared/paquet.h"

class FenPrincipale;

class Client : public QObject
{
    Q_OBJECT
public:
    Client(QTcpSocket *socket, FenPrincipale *parent = 0);
    ~Client();

    QTcpSocket* getSocket() { return m_socket; }

    QString getPseudo() { return m_pseudo; }
    void setPseudo(QString pseudo) { m_pseudo = pseudo; }
    QString getAccount() { return m_account; }
    void setAccount(QString account) { m_account = account; }
    quint8 getLoginLevel() { return m_loginLevel; }
    void setLoginLevel(quint8 loginLevel) { m_loginLevel = loginLevel; }
    quint32 getIdCompte() { return m_idCompte; }
    void setIdCompte(quint32 id) { m_idCompte = id; }
    QByteArray getHashIP() { return m_hashIP; }
    QString getLogoutMessage() { return m_logoutMessage; }
    void setLogoutMessage(QString msg) { m_logoutMessage = msg; }
    int getSessionState() { return m_sessionState; }
    void setSessionState(int state) { m_sessionState = state; }

    quint8 getPingsPending() { return m_pingsPending; }
    void setPingsPending(quint8 pings) { m_pingsPending = pings; }
    quint16 getPing() { return m_ping; }
    void setPing(quint16 ping) { m_ping = ping; }


signals:
    void deconnecte();
    void console(QString);
    void paquetRecu(Paquet*);

public slots:
    void donneesRecues();
    void deconnexion();
    void sendPing();

private:
    FenPrincipale *m_parent;
    QTcpSocket *m_socket;
    quint16 m_taillePaquet;

    QString m_pseudo;
    QString m_account;
    quint8 m_loginLevel;
    quint32 m_idCompte;
    QByteArray m_hashIP;
    QString m_logoutMessage;
    int m_sessionState;

    quint8 m_pingsPending;
    QTimer *m_pingTimer;
    quint16 m_ping;
};

#endif // CLIENT_H
