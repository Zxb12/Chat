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
    connecterBDD();
}

void FenPrincipale::connecterBDD()
{
    QFile confFile("server.conf");
    if (confFile.open(QIODevice::ReadOnly))
    {

        //Chargement depuis le fichier.
        QString address = QString(confFile.readLine()).remove("SQL_ADDRESS=").remove("\r\n");
        QString database = QString(confFile.readLine()).remove("SQL_DATABASE=").remove("\r\n");
        QString login = QString(confFile.readLine()).remove("SQL_LOGIN=").remove("\r\n");
        QString pass = QString(confFile.readLine()).remove("SQL_PASSWORD=").remove("\r\n");

        //Ouverture de la BDD
        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
        db.setHostName(address);
        db.setDatabaseName(database);
        bool ok = db.open(login, pass);
        if (ok)
        {
            CONSOLE("Connexion réussie à la BDD");
        }
        else
        {
            CONSOLE("Connexion échouée à la BDD: " + db.lastError().text());
            return;
        }
    }
    else
    {
        CONSOLE("Impossible d'ouvrir server.conf");
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
    connect(nouveauClient, SIGNAL(paquetRecu(Paquet*)), this, SLOT(paquetRecu(Paquet*)));

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

    //On envoie le message de déconnexion à tous si la personne a un pseudo.
    if (!client->getPseudo().isEmpty())
    {
        Paquet out;
        out << SMSG_USER_LEFT;
        out << client->getPseudo();
        envoyerATous(out);
    }

    //On le supprime de la liste de clients
    m_clients.removeOne(client);

    //Et on le désalloue
    delete client;
}

void FenPrincipale::kickClient(Client *client)
{
    CONSOLE("Le client " + client->getSocket()->peerAddress().toString() + " a été kické.");
    Paquet out;
    out << SMSG_KICK;
    out.send(client->getSocket());
    client->getSocket()->disconnectFromHost();
}

void FenPrincipale::paquetRecu(Paquet *in)
{
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

    CONSOLE("Paquet reçu: " + handler.nom + "(" + QString::number(opCode) + ")");

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

//OpCode reçu lors de la première connexion du client.
void FenPrincipale::handleAuthLogin(Paquet* in, Client* client)
{
    QString pseudo, login;
    QByteArray pwhash;
    quint8 authLevel;

    *in >> login;
    *in >> pwhash;
    *in >> pseudo;

    login = login.simplified().toUpper();
    pwhash = pwhash.toHex();
    pseudo = pseudo.simplified();

    //Notre client a spécifié un compte
    if (!login.isEmpty())
    {
        QSqlQuery query;
        query.prepare("SELECT * FROM account where login = :login AND pwhash = :pwhash");
        query.bindValue(":login", login);
        query.bindValue(":pwhash", pwhash);
        if (!query.exec())
        {
            CONSOLE("ERREUR SQL: " + query.lastError().databaseText());
            Paquet out;
            out << SMSG_AUTH_ERROR;
            out.send(client->getSocket());
            kickClient(client);
            return;
        }

        if (!query.next())
        {
            //On n'a trouvé aucun enregistrement: compte ou mdp incorrect !
            CONSOLE("Un client a essayé de se connecter avec un mauvais login/mdp.");
            Paquet out;
            out << SMSG_AUTH_INCORRECT_LOGIN;
            out.send(client->getSocket());
            kickClient(client);
            return;
        }

        //On a trouvé un enregistrement correspondant au nom et au mot de passe !
        //On récupère son niveau
        authLevel = query.value(3).toInt();

        //On vérifie si personne n'est déjà connecté sous ce nom.
        foreach(Client* i_client, m_clients)
        {
            if (i_client->getAccount() == login)
            {
                CONSOLE("Un client a essayé de se connecter avec un compte déjà utilisé.");
                Paquet out;
                out << SMSG_AUTH_ACCT_ALREADY_IN_USE;
                out.send(client->getSocket());
                kickClient(client);
                return;
            }
        }

        //Le compte est OK.
    }
    else
    {
        //Pas de compte spécifié
        authLevel = 0;
    }
    //Vérifie la taille du pseudo
    if (pseudo.size() < TAILLE_PSEUDO_MIN)
    {
        CONSOLE("ERREUR: Nommage impossible, pseudo trop court.");
        Paquet out;
        out << SMSG_NICK_TOO_SHORT;
        out.send(client->getSocket());
        kickClient(client);
        return;
    }

    //Vérifie si le pseudo est déjà utilisé
    foreach(Client* i_client, m_clients)
    {
        if (i_client->getPseudo().compare(pseudo, Qt::CaseInsensitive) == false)
        {
            CONSOLE("ERREUR: Nommage impossible, nom déjà utilisé.");
            Paquet out;
            out << SMSG_NICK_ALREADY_IN_USE;
            out.send(client->getSocket());
            kickClient(client);
            return;
        }
    }

    //Client authentifié.
    CONSOLE("Client authentifié : " + pseudo);
    client->setPseudo(pseudo);
    client->setAccount(login);
    client->setAuthLevel(authLevel);

    Paquet out;
    out << SMSG_AUTH_OK;
    out << pseudo;
    out.send(client->getSocket());

    out.clear();
    out << SMSG_USER_JOINED;
    out << pseudo;
    envoyerATous(out);

    return;

}

void FenPrincipale::handleSetNick(Paquet *in, Client *client)
{
    QString pseudo, ancienPseudo;
    *in >> pseudo;
    ancienPseudo = client->getPseudo();

    pseudo = pseudo.simplified();

    //Vérifie la taille du pseudo
    if (pseudo.size() < TAILLE_PSEUDO_MIN)
    {
        CONSOLE("ERREUR: Nommage impossible, pseudo trop court.");
        Paquet out;
        out << SMSG_NICK_TOO_SHORT;
        out.send(client->getSocket());
        return;
    }

    //Vérifie si le pseudo est déjà utilisé
    foreach(Client* i_client, m_clients)
    {
        if (i_client->getPseudo().compare(pseudo, Qt::CaseInsensitive) == 0)
        {
            CONSOLE("ERREUR: Nommage impossible, nom déjà utilisé.");
            Paquet out;
            out << SMSG_NICK_ALREADY_IN_USE;
            out.send(client->getSocket());
            return;
        }
    }

    //Renommage du client
    CONSOLE("Client nommé: " + pseudo);
    client->setPseudo(pseudo);

    Paquet out;
    out << SMSG_USER_RENAMED;
    out << ancienPseudo;
    out << pseudo;
    envoyerATous(out);

    return;

}

void FenPrincipale::handleChatMessage(Paquet *in, Client *client)
{
    QString message;
    *in >> message;
    QString pseudo = client->getPseudo();

    //Traitement du message
    message = message.simplified();
    message.replace("<", "&lt;");
    message.replace(">", "&gt;");

    //On vérifie si le message n'est pas vide.
    if (message.isEmpty())
    {
        Paquet out;
        out << SMSG_INVALID_MESSAGE;
        out.send(client->getSocket());
        return;
    }

    //On vérifie que le client a un pseudo valide.
    if (pseudo.size() < TAILLE_PSEUDO_MIN)
    {
        Paquet out;
        out << SMSG_INVALID_NICK;
        out.send(client->getSocket());
        return;
    }

    //Si on est arrivés là, on peut envoyer le message à tous.
    //Préparation du paquet
    Paquet out;
    out << SMSG_CHAT_MESSAGE;
    out << pseudo;
    out << message;

    envoyerATous(out);

    //Archivage dans la BDD
    QSqlQuery query;
    query.prepare("INSERT INTO chat_history (name, message, date) "
                  "VALUES (:name, :message, :date)");
    query.bindValue(":name", pseudo);
    query.bindValue(":message", message);
    query.bindValue(":date", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"));
    query.exec();

}

void FenPrincipale::handlePing(Paquet *in, Client *client)
{
    //On a reçu un ping, on remet le décompte de kick à 0
    client->setPingsPending(0);

    quint32 diff;
    *in >> diff;

    QTime time(0, 0);
    time = time.addMSecs(diff);

    diff = time.msecsTo(QTime::currentTime());
    CONSOLE(client->getPseudo() + ": ping " + QString::number(diff) + " ms");

    client->setPing(diff);
}

void FenPrincipale::handleRegister(Paquet *in, Client *client)
{
    QString login;
    QByteArray pwhash;

    *in >> login;
    *in >> pwhash;

    //Traitement des données
    login = login.simplified().toUpper();
    pwhash = pwhash.toHex();

    //Vérification de la taille du login
    if (login.size() < TAILLE_COMPTE_MIN)
    {
        Paquet out;
        out << SMSG_REG_INVALID_NICK;
        out.send(client->getSocket());
        return;
    }

    //Vérification des doublons.
    QSqlQuery query;
    query.prepare("SELECT * FROM account WHERE login = :login");
    query.bindValue(":login", login);
    if (!query.exec())
    {
        //Erreur de requête.
        CONSOLE("ERREUR SQL: " + query.lastError().databaseText());
        Paquet out;
        out << SMSG_REG_ERROR;
        out.send(client->getSocket());
        return;
    }

    //Si on a trouvé un compte portant ce nom, on arrête
    if (query.next())
    {
        Paquet out;
        out << SMSG_REG_ACCT_ALREADY_EXISTS;
        out.send(client->getSocket());
        return;
    }

    //Insersion dans la base de données.
    query.prepare("INSERT INTO account (login, pwhash) VALUES (:login, :pwhash)");
    query.bindValue(":login", login);
    query.bindValue(":pwhash", pwhash);
    if (!query.exec())
    {
        //Erreur de requête.
        CONSOLE("ERREUR SQL: " + query.lastError().databaseText());
        Paquet out;
        out << SMSG_REG_ERROR;
        out.send(client->getSocket());
        return;
    }

    CONSOLE("Nouveau compte enregistré: " + login);
    Paquet out;
    out << SMSG_REG_OK;
    out.send(client->getSocket());
}
