#include "fenprincipale.h"
#include "ui_fenprincipale.h"

#define CONSOLE(a)  ui->console->append(QTime::currentTime().toString() + " " + a)

FenPrincipale::FenPrincipale(QWidget *parent) : QWidget(parent), ui(new Ui::FenPrincipale)
{
    ui->setupUi(this);

    m_serveur = new QTcpServer(this);

    //D�marrage du serveur
    if (!m_serveur->listen(QHostAddress::Any, PORT_SERVEUR))
    {
        //Erreur de d�marrage
        ui->etatServeur->setText("Le serveur n'a pas pu d�marrer : " + m_serveur->errorString());
    }
    else
    {
        //Serveur d�marr�
        ui->etatServeur->setText("Serveur d�marr� sur le port : <strong>" + QString::number(m_serveur->serverPort()) + "</strong>");
        connect(m_serveur, SIGNAL(newConnection()), this, SLOT(nouvelleConnexion()));
    }
}

FenPrincipale::~FenPrincipale()
{
    delete m_serveur;
    delete ui;
}

void FenPrincipale::console(QString txt)
{
    CONSOLE(txt);
}

void FenPrincipale::nouvelleConnexion()
{
    //Nouvelle connexion : on cr�e l'objet Client, on fait les connexions
    Client *nouveauClient = new Client(m_serveur->nextPendingConnection());
    connect(nouveauClient, SIGNAL(deconnecte()), this, SLOT(decoClient()));
    connect(nouveauClient, SIGNAL(console(QString)), this, SLOT(console(QString)));
    connect(nouveauClient, SIGNAL(paquetRecu(QDataStream*)), this, SLOT(paquetRecu(QDataStream*)));

    //On ajoute le client � la liste
    m_clients << nouveauClient;
    CONSOLE("Nouveau client, IP : " + nouveauClient->getSocket()->peerAddress().toString());
}

void FenPrincipale::decoClient()
{
    //On d�termine quel client se d�connecte
    Client *client = qobject_cast<Client *>(sender());

    //On v�rifie qu'il existe
    if (!client)
        return;

    CONSOLE("Un client s'est d�connect�.");

    //On le supprime de la liste de clients
    m_clients.removeOne(client);

    //Et on le d�salloue
    delete client;
}

void FenPrincipale::kickClient(Client *client)
{
    CONSOLE("Le client " + client->getSocket()->peerAddress().toString() + " a �t� kick�.");
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);

    out << (quint16) sizeof SMSG_KICK ;
    out << SMSG_KICK;

    client->getSocket()->write(paquet);
    client->getSocket()->close();
}

void FenPrincipale::paquetRecu(QDataStream *stream)
{
    //On extrait le paquet de la socket
    Paquet *in = new Paquet(stream->device()->readAll());

    //On d�termine d'o� vient le paquet
    Client *client = qobject_cast<Client *>(sender());

    //On v�rifie que le client existe
    if (!client)
        return;

    //On r�cup�re l'OpCode
    quint16 opCode;
    *in >> opCode;

    if (opCode > NB_OPCODES)
    {
        CONSOLE("ERREUR: Un client a envoy� un opCode non g�r�(" + QString::number(opCode) + "). Kick du client.");

        //On vide le paquet.
        in->clear();
        delete in;

        //On supprime le client.

        kickClient(client);
        return;
    }

    OpCodeHandler handler = OpCodeTable[opCode];

    CONSOLE("Paquet re�u: " + handler.nom);

    //Lancement de la fonction associ�e.
    (this->*handler.f)(in, client);

    //Lib�ration de la m�moire.
    delete in;
}

void FenPrincipale::envoyerATous(Paquet &paquet)
{
    //Envoi du paquet � tous les clients.
    foreach(Client *client, m_clients)
    {
        paquet.send(client->getSocket());
    }
}

void FenPrincipale::handleHello(Paquet* in, Client* client)
{
    Paquet out;
    out << SMSG_HELLO;

    out.send(client->getSocket());
}

void FenPrincipale::handleServerSide(Paquet* in, Client* client)
{
    CONSOLE("Paquet re�u avec un opCode de serveur.");
}

void FenPrincipale::handleAuthSetName(Paquet* in, Client* client)
{
    QString pseudo;
    *in >> pseudo;

    CONSOLE("Un client s'appelle " + pseudo);

    bool utilise = false;
    foreach(Client* i_client, m_clients)
    {
        if (i_client->getPseudo() == pseudo)
        {
            utilise = true;
            break;
        }
    }

    if (pseudo.size() < TAILLE_PSEUDO_MIN)
    {
        CONSOLE("ERREUR: Nommage impossible, pseudo trop court.");
        Paquet out;
        out << SMSG_AUTH_NAME_TOO_SHORT;
        out.send(client->getSocket());

        return;
    }

    if (utilise)
    {
        CONSOLE("ERREUR: Nommage impossible, nom d�j� utilis�.");

        Paquet out;
        out << SMSG_AUTH_NAME_ALREADY_IN_USE;
        out.send(client->getSocket());

        return;
    }
    else
    {
        CONSOLE("Client nomm�: " + pseudo);
        client->setPseudo(pseudo);

        Paquet out;
        out << SMSG_AUTH_OK;
        out << pseudo;

        out.send(client->getSocket());
        return;
    }
}

void FenPrincipale::handleChatMessage(Paquet *in, Client *client)
{
    QString message;
    *in >> message;

    //On v�rifie si le message n'est pas vide.
    if (message.isEmpty())
    {
        Paquet out;
        out << SMSG_INVALID_MESSAGE;
        out.send(client->getSocket());
        return;
    }

    //On v�rifie que le client a un pseudo.
    if (client->getPseudo().isEmpty())
    {
        Paquet out;
        out << SMSG_NAME_NOT_SET;
        out.send(client->getSocket());
        return;
    }

    //Si on est arriv�s l�, on peut envoyer le message � tous.
    //Pr�paration du paquet
    Paquet out;
    out << SMSG_CHAT_MESSAGE;
    out << "<strong>" + client->getPseudo() + "</strong> " + message;

    envoyerATous(out);
}
