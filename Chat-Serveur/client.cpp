#include "client.h"

#define CONSOLE(a) emit console(a)

Client::Client(QTcpSocket *socket) : m_socket(socket), m_taillePaquet(0), m_pseudo(""), m_pingsPending(0), m_ping(0)
{
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(donneesRecues()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(deconnexion()));

    //Création du timer pour le lancement des pings.
    m_pingTimer = new QTimer(this);
    connect(m_pingTimer, SIGNAL(timeout()), this, SLOT(sendPing()));
    m_pingTimer->start(10000);
}

Client::~Client()
{
}

void Client::donneesRecues()
{
    QDataStream stream(m_socket);

    //Récupération de la taille du paquet
    if (m_taillePaquet == 0)
    {
        if (m_socket->bytesAvailable() < sizeof m_taillePaquet)
            return;

        stream >> m_taillePaquet;
    }

    //Récupération du reste du paquet
    if (m_socket->bytesAvailable() < m_taillePaquet)
        return;

    //On lit la socket pour la taille d'un paquet et on stocke.
    Paquet *in = new Paquet(m_socket->read(m_taillePaquet));

    //Remise à zéro de la taille du paquet
    m_taillePaquet = 0;

    //On envoie le paquet reçu
    emit paquetRecu(in);

    //S'il nous reste quelque chose dans la socket, on relance la fonction.
    if (m_socket->bytesAvailable())
        donneesRecues();
}

void Client::deconnexion()
{
    emit deconnecte();
}

void Client::sendPing()
{
    //On stocke le nombre de millisecondes jusqu'à minuit.
    QTime time;
    Paquet out;
    out << SMSG_PING;
    out << quint32(time.msecsTo(QTime::currentTime()));
    out.send(m_socket);

    m_pingsPending++;

    if (m_pingsPending > 3)
    {
        CONSOLE(m_pseudo + " a été kické pour ping timeout.");
        m_socket->abort();
    }
}
