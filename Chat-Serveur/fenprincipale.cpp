#include "fenprincipale.h"
#include "ui_fenprincipale.h"

#define CONSOLE(a)  ui->console->append(QTime::currentTime().toString() + " " + a)

FenPrincipale::FenPrincipale(QWidget *parent) : QWidget(parent), ui(new Ui::FenPrincipale)
{
    ui->setupUi(this);

    m_serveur = new QTcpServer(this);

    //Démarrage du serveur
    if (!m_serveur->listen(QHostAddress::Any, PORT_SERVEUR))
    {
        //Erreur de démarrage
        ui->etatServeur->setText("Le serveur n'a pas pu démarrer : " + m_serveur->errorString());
    }
    else
    {
        //Serveur démarré
        ui->etatServeur->setText("Serveur démarré sur le port : <strong>" + QString::number(m_serveur->serverPort()) + "</strong>");
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
    //Nouvelle connexion : on crée l'objet Client, on fait les connexions
    Client *nouveauClient = new Client(m_serveur->nextPendingConnection());
    connect(nouveauClient, SIGNAL(deconnecte()), this, SLOT(decoClient()));
    connect(nouveauClient, SIGNAL(console(QString)), this, SLOT(console(QString)));
    connect(nouveauClient, SIGNAL(paquetRecu(QDataStream*)), this, SLOT(paquetRecu(QDataStream*)));

    //On ajoute le client à la liste
    m_clients << nouveauClient;
    CONSOLE("Nouveau client, IP : " + nouveauClient->getSocket()->peerAddress().toString());
}

void FenPrincipale::decoClient()
{
    //On détermine quel client se déconnecte
    Client *client = qobject_cast<Client *>(sender());

    //On vérifie qu'il existe
    if (!client)
        return;

    CONSOLE("Un client s'est déconnecté.");

    //On le supprime de la liste de clients
    m_clients.removeOne(client);

    //Et on le désalloue
    delete client;
}

void FenPrincipale::kickClient(Client *client)
{
    CONSOLE("Le client " + client->getSocket()->peerAddress().toString() + " a été kické.");
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

    //On détermine d'où vient le paquet
    Client *client = qobject_cast<Client *>(sender());

    //On vérifie que le client existe
    if (!client)
        return;

    //On récupère l'OpCode
    quint16 opCode;
    *in >> opCode;

    if (opCode > NB_OPCODES)
    {
        CONSOLE("ERREUR: Un client a envoyé un opCode non géré(" + QString::number(opCode) + "). Kick du client.");

        //On vide le paquet.
        in->clear();
        delete in;

        //On supprime le client.

        kickClient(client);
        return;
    }

    OpCodeHandler handler = OpCodeTable[opCode];

    CONSOLE("Paquet reçu: " + handler.nom);

    //Lancement de la fonction associée.
    (this->*handler.f)(in, client);

    //Libération de la mémoire.
    delete in;
}

void FenPrincipale::envoyerATous(Paquet &paquet)
{
    //Envoi du paquet à tous les clients.
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
    CONSOLE("Paquet reçu avec un opCode de serveur.");
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
        CONSOLE("ERREUR: Nommage impossible, nom déjà utilisé.");

        Paquet out;
        out << SMSG_AUTH_NAME_ALREADY_IN_USE;
        out.send(client->getSocket());

        return;
    }
    else
    {
        CONSOLE("Client nommé: " + pseudo);
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

    //On vérifie si le message n'est pas vide.
    if (message.isEmpty())
    {
        Paquet out;
        out << SMSG_INVALID_MESSAGE;
        out.send(client->getSocket());
        return;
    }

    //On vérifie que le client a un pseudo.
    if (client->getPseudo().isEmpty())
    {
        Paquet out;
        out << SMSG_NAME_NOT_SET;
        out.send(client->getSocket());
        return;
    }

    //Si on est arrivés là, on peut envoyer le message à tous.
    //Préparation du paquet
    Paquet out;
    out << SMSG_CHAT_MESSAGE;
    out << "<strong>" + client->getPseudo() + "</strong> " + message;

    envoyerATous(out);
}
