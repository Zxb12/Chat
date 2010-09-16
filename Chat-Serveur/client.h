#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <QTcpServer>
#include <QDataStream>
#include <QTime>
#include <QTimer>

#include "../shared/paquet.h"

class PingThread;

//enum AuthStatus
//{
//    Disconnected,
//    Connected
//};


class Client : public QObject
{
    Q_OBJECT
public:
    Client(QTcpSocket *socket);
    ~Client();

    QTcpSocket* getSocket() { return m_socket; }

    QString getPseudo() { return m_pseudo; }
    bool setPseudo(QString pseudo) { m_pseudo = pseudo; return true; }
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
    QTcpSocket *m_socket;
    quint16 m_taillePaquet;
    QString m_pseudo;

    quint8 m_pingsPending;
    QTimer *m_pingTimer;
    quint16 m_ping;

    //AuthStatus m_authStatus;

};

#endif // CLIENT_H
