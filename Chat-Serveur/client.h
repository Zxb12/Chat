#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <QTcpServer>
#include <QDataStream>

#include "../shared/paquet.h"

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
    bool setPseudo(QString pseudo) { m_pseudo = pseudo; return true; };

signals:
    void deconnecte();
    void console(QString);
    void paquetRecu(QDataStream*);

public slots:
    void donneesRecues();
    void deconnexion();

private:
    QTcpSocket *m_socket;
    quint16 m_taillePaquet;
    QString m_pseudo;
    //AuthStatus m_authStatus;

};

#endif // CLIENT_H
