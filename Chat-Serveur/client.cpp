#include "client.h"

#define CONSOLE(a) emit console(a)

Client::Client(QTcpSocket *socket) : m_socket(socket), m_taillePaquet(0), m_pseudo("")
{
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(donneesRecues()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(deconnexion()));
}

Client::~Client()
{

}

void Client::donneesRecues()
{
    QDataStream in(m_socket);

    //R�cup�ration de la taille du paquet
    if (m_taillePaquet == 0)
    {
        if (m_socket->bytesAvailable() < sizeof m_taillePaquet)
            return;

        in >> m_taillePaquet;
    }

    //R�cup�ration du reste du paquet
    if (m_socket->bytesAvailable() < m_taillePaquet)
        return;

    //Remise � z�ro de la taille du paquet
    m_taillePaquet = 0;

    //On envoie le paquet re�u
    emit paquetRecu(&in);
}

void Client::deconnexion()
{
    emit deconnecte();
}
