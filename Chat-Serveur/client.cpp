#include "client.h"

Client::Client(QTcpSocket *socket, FenPrincipale *parent) : m_parent(parent), m_socket(socket), m_taillePaquet(0), m_pseudo(""), m_account(""), m_loginLevel(0),
m_idCompte(0), m_logoutMessage(""), m_sessionState(NOT_CHECKED), m_channel(0), m_pingsPending(0), m_ping(0)

{
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(donneesRecues()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(deconnexion()));

    //Cr�ation du timer pour le lancement des pings.
    m_pingTimer = new QTimer(this);
    connect(m_pingTimer, SIGNAL(timeout()), this, SLOT(sendPing()));
    m_pingTimer->start(m_parent->getPingInterval());
    sendPing(); //On lance un ping

    //G�n�ration du hash de l'IP.
    //4 caract�res hexa.
    m_hashIP = QCryptographicHash::hash(m_socket->peerAddress().toString().toUtf8(),
                                        QCryptographicHash::Md5).toHex().left(4);
}

Client::~Client()
{
}

void Client::donneesRecues()
{
    QDataStream stream(m_socket);

    //R�cup�ration de la taille du paquet
    if (m_taillePaquet == 0)
    {
        if (m_socket->bytesAvailable() < sizeof m_taillePaquet)
            return;

        stream >> m_taillePaquet;
    }

    //R�cup�ration du reste du paquet
    if (m_socket->bytesAvailable() < m_taillePaquet)
        return;

    //On lit la socket pour la taille d'un paquet et on stocke.
    Paquet *in = new Paquet(m_socket->read(m_taillePaquet));

    //Remise � z�ro de la taille du paquet
    m_taillePaquet = 0;

    //On envoie le paquet re�u
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
    //On stocke le nombre de millisecondes jusqu'� minuit.
    QTime time;
    Paquet out;
    out << SMSG_PING;
    out << quint32(time.msecsTo(QTime::currentTime()));
    out >> m_socket;

    m_pingsPending++;

    if (m_pingsPending > m_parent->getMaxPingsPending())
    {
        console(m_pseudo + " a �t� kick� pour ping timeout.");

        //On avertit les connect�s.
        if (!m_pseudo.isEmpty())
        {
            Paquet out;
            out << SMSG_USER_KICKED;
            out << QString("le serveur");    //Par qui on a �t� kick�
            out << m_pseudo;                 //Qui a �t� kick�
            out << QString("ping timeout");  //Raison
            m_parent->envoyerAuChannel(out, m_channel);
        }

        //D�connexion.
        m_socket->abort();
    }
}
