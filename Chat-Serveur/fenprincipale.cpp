#include "fenprincipale.h"
#include "ui_fenprincipale.h"

using namespace std;

#if defined ( WIN32 ) || defined ( WIN64 )
#define ENDL "\r\n"
#include <windows.h>
#else
#define ENDL "\n"
#endif

#define console(a) console(a)

FenPrincipale::FenPrincipale(QObject *parent) : QObject(parent)
{
#if defined ( WIN32 ) || defined ( WIN64 )
    system(QString("TITLE OokChat Server " + VERSION).toStdString().c_str());
#endif
    QCoreApplication::instance()->setApplicationName("OokChat Server");
    QCoreApplication::instance()->setApplicationVersion(VERSION);

    console("                                                      ");
    console("------------------------------------------------------");
    console("-                                                    -");
    console("-                   OokChat Server                   -");
    console("-                    "+VERSION+"                     -");
    console("-                                                    -");
    console("-                 Created by Frugebul                -");
    console("-                                                    -");
    console("------------------------------------------------------");
    console("                                                      ");
    console("Lancement du serveur...");

    //Préparation du serveur
    if (!chargerFichier())
        return;
    if (connecterBDD())
        chargerChannels();
    else
        return;


    //Démarrage du serveur
    m_serveur = new QTcpServer(this);
    if (!m_serveur->listen(QHostAddress::Any, m_serverPort))
    {
        //Erreur de démarrage
        console("Le serveur n'a pas pu démarrer: " + m_serveur->errorString());
    }
    else
    {
        //Serveur démarré
        connect(m_serveur, SIGNAL(newConnection()), this, SLOT(nouvelleConnexion()));
        console("Serveur démarré sur le port: " + QString::number(m_serveur->serverPort()));
    }
}

bool FenPrincipale::connecterBDD()
{
    //Ouverture de la BDD
    console("Connexion à la base de données... " + m_SQLAdresse + " " + m_SQLDatabase);
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(m_SQLAdresse);
    db.setDatabaseName(m_SQLDatabase);
    bool ok = db.open(m_SQLLogin, m_SQLPassword);
    if (ok)
    {
        console("Connexion à la base de données réussie !");
        return true;
    }
    else
    {
        console("Connexion à la base de données échouée");
        console("Raison: " + db.lastError().text());
        return false;
    }
}

bool FenPrincipale::chargerFichier()
{
    QFile confFile("server.conf");
    console("Chargement du fichier de configuration... " + confFile.fileName());
    if (confFile.open(QIODevice::ReadOnly))
    {
        //Vérification de la version du fichier.
        if (VERSION_CONFIG != QString(confFile.readLine()).remove("CONFIG_VERSION=").remove(ENDL).toInt())
        {
            console("ERREUR: Le fichier de configuration n'est pas à la bonne version.");
            return false;
        }

        //Chargement depuis le fichier.
        m_serverPort =                      QString(confFile.readLine()).remove("SERVER_PORT=").remove(ENDL).toInt();
        m_SQLAdresse =                      QString(confFile.readLine()).remove("SQL_ADDRESS=").remove(ENDL);
        m_SQLDatabase =                     QString(confFile.readLine()).remove("SQL_DATABASE=").remove(ENDL);
        m_SQLLogin =                        QString(confFile.readLine()).remove("SQL_LOGIN=").remove(ENDL);
        m_SQLPassword =                     QString(confFile.readLine()).remove("SQL_PASSWORD=").remove(ENDL);
        m_pingInterval =                    QString(confFile.readLine()).remove("PING_INTERVAL=").remove(ENDL).toInt();
        m_maxPingsPending =                 QString(confFile.readLine()).remove("MAX_PINGS_PENDING=").remove(ENDL).toInt();
        m_nickMinLength =                   QString(confFile.readLine()).remove("NICK_MIN_LENGTH=").remove(ENDL).toInt();
        m_nickMaxLength =                   QString(confFile.readLine()).remove("NICK_MAX_LENGTH=").remove(ENDL).toInt();
        m_accountNameMinLength =            QString(confFile.readLine()).remove("ACCOUNT_NAME_MIN_LENGTH=").remove(ENDL).toInt();
        m_levelMax =                        QString(confFile.readLine()).remove("LVL_MAX=").remove(ENDL).toInt();
        m_registerLevel =                   QString(confFile.readLine()).remove("REGISTER_LVL=").remove(ENDL).toInt();
        m_kickLevel =                       QString(confFile.readLine()).remove("KICK_LVL=").remove(ENDL).toInt();
        m_banLevel =                        QString(confFile.readLine()).remove("BAN_LVL=").remove(ENDL).toInt();
        m_voiceLevel =                      QString(confFile.readLine()).remove("VOICE_LVL=").remove(ENDL).toInt();
        m_promoteLevel =                    QString(confFile.readLine()).remove("PROMOTE_LVL=").remove(ENDL).toInt();
        m_whoisLevel =                      QString(confFile.readLine()).remove("WHOIS_LVL=").remove(ENDL).toInt();
        m_channelCreateLevel =              QString(confFile.readLine()).remove("CHANNEL_CREATE_LVL=").remove(ENDL).toInt();
        m_channelCreatePersistantLevel =    QString(confFile.readLine()).remove("CHANNEL_CREATE_PERSISTANT_LVL=").remove(ENDL).toInt();
        m_channelDeleteLevel =              QString(confFile.readLine()).remove("CHANNEL_DELETE_LVL=").remove(ENDL).toInt();
        m_channelEditLevel =                QString(confFile.readLine()).remove("CHANNEL_EDIT_LVL=").remove(ENDL).toInt();

        console("Chargement du fichier de configuration réussi.");
        return true;
    }
    else
    {
        console("ERREUR: Le fichier de configuration n'a pas pu être ouvert.");
        return false;
    }
}

void FenPrincipale::chargerChannels()
{
    QSqlQuery query;
    query.exec("SELECT * FROM channel WHERE `default` = 1");
    if (query.next())
    {
        Channel* channel = new Channel(query.value(0).toUInt(), query.value(1).toString(),
                                       query.value(3).toUInt(), query.value(2).toString(),
                                       query.value(5).toString(), query.value(6).toUInt(),
                                       true, this);
        m_channels.append(channel);
        connect (channel, SIGNAL(channelNeedsToBeRemoved(Channel*)), this, SLOT(supprimerChannel(Channel*)));
    }
    else
        console("ERREUR: Channel: Aucun channel par défaut spécifié.");

    query.clear();
    query.exec("SELECT * FROM channel WHERE `default` <> 1 ORDER BY id");
    while (query.next())
    {
        Channel* channel = new Channel(query.value(0).toUInt(), query.value(1).toString(),
                                       query.value(3).toUInt(), query.value(2).toString(),
                                       query.value(5).toString(), query.value(6).toUInt(),
                                       false, this);
        m_channels.append(channel);
        connect (channel, SIGNAL(channelNeedsToBeRemoved(Channel*)), this, SLOT(supprimerChannel(Channel*)));
    }
}

FenPrincipale::~FenPrincipale()
{
    delete m_serveur;
}

void FenPrincipale::console(QString txt)
{
#if defined ( WIN32 ) || defined ( WIN64 )
    txt = QTime::currentTime().toString() + " " + txt;
    char* tmpBufOem = new char[txt.size()+1];
    CharToOemA(txt.toAscii().data(), tmpBufOem);
    cout << tmpBufOem << endl;
    delete[] tmpBufOem;
#else
    qDebug() << QTime::currentTime().toString() + " " + txt;
#endif
}

void FenPrincipale::nouvelleConnexion()
{
    //Nouvelle connexion : on crée l'objet Client, on fait les connexions
    Client *nouveauClient = new Client(m_serveur->nextPendingConnection(), this);
    connect(nouveauClient, SIGNAL(deconnecte()), this, SLOT(decoClient()));
    connect(nouveauClient, SIGNAL(console(QString)), this, SLOT(console(QString)));
    connect(nouveauClient, SIGNAL(paquetRecu(Paquet*)), this, SLOT(paquetRecu(Paquet*)));

    //On ajoute le client à la liste
    m_clients << nouveauClient;
    console("Nouveau client, IP : " + nouveauClient->getSocket()->peerAddress().toString());
}

void FenPrincipale::decoClient()
{
    //On détermine quel client se déconnecte
    Client *client = qobject_cast<Client *>(sender());

    //On vérifie qu'il existe
    if (!client)
        return;

    console("Un client s'est déconnecté.");

    //On envoie le message de déconnexion à tous si la personne a un pseudo.
    if (!client->getPseudo().isEmpty())
    {
        Paquet out;
        out << SMSG_USER_LEFT;
        out << client->getPseudo() << client->getLogoutMessage() << client->getHashIP(), client->getLoginLevel();
        envoyerAuChannel(out, client->getChannel());
        client->getChannel()->removeUser(client);
    }

    //On le supprime de la liste de clients
    m_clients.removeOne(client);

    //Et on le désalloue
    delete client;
}

void FenPrincipale::kickClient(Client *client)
{
    console("Le client " + client->getSocket()->peerAddress().toString() + " a été kické.");
    Paquet out;
    out << SMSG_KICK;
    out >> client->getSocket();
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
        console("ERREUR: Un client a envoyé un opCode non géré(" + QString::number(opCode) + "). Kick du client.");

        //On vide le paquet.
        in->clear();
        delete in;

        //On supprime le client.

        kickClient(client);
        return;
    }

    OpCodeHandler handler = OpCodeTable[opCode];

    console("Paquet reçu: " + QString(handler.nom) + "(" + QString::number(opCode) + ")");

    //On vérifie que le client a le droit d'envoyer ce paquet.
    switch (handler.state)
    {
    case NOT_CHECKED:   //Toujours possible
        (this->*handler.f)(in, client);
        break;

    case CHECKED:
        if (client->getSessionState() < CHECKED)
            kickClient(client);
        else
            (this->*handler.f)(in, client);
        break;

    case AUTHED:
        if (client->getSessionState() < AUTHED)
            kickClient(client);
        else
            (this->*handler.f)(in, client);
        break;
    case NEVER:
        console("Le paquet reçu n'est pas géré.");
        kickClient(client);
        break;
    }

    //Libération de la mémoire.
    delete in;
}

void FenPrincipale::supprimerChannel(Channel *channel)
{
    m_channels.removeOne(channel);

    //Suppression du channel dans la BDD
    QSqlQuery query;
    query.prepare("DELETE FROM channel WHERE id = :id");
    query.bindValue(":id", channel->getId());
    if (!query.exec())
    {
        console(query.lastError().text());
    }
    delete channel;

    //Mise à jour de la liste des channels chez les clients.
    foreach (Client* i_client, m_clients)
        handleUpdateChannel(0, i_client);
}

void FenPrincipale::envoyerAuServeur(Paquet &paquet)
{
    //Envoi du paquet à tous les clients.
    foreach(Client *client, m_clients)
    {
        paquet >> client->getSocket();
    }
}

void FenPrincipale::envoyerAuChannel(Paquet &paquet, Channel *channel)
{
    //Envoi du paquet à tous les clients.
    foreach(Client *client, channel->getClients())
    {
        paquet >> client->getSocket();
    }
}

void FenPrincipale::changerChannel(Client *client, Channel *channel)
{
    if (channel != client->getChannel())
    {
        Paquet out;
        out << SMSG_CHANNEL_LEAVE << client->getPseudo() << client->getHashIP() << client->getLoginLevel() << client->getChannel()->getTitre();
        envoyerAuChannel(out, client->getChannel());
        client->getChannel()->removeUser(client);

        channel->addUser(client);
        out.clear();
        out << SMSG_CHANNEL_JOIN << client->getPseudo() << client->getHashIP() << client->getLoginLevel() << channel->getTitre();
        envoyerAuChannel(out, channel);
    }
}

void FenPrincipale::handleHello(Paquet* in, Client* client)
{
    QString clientVersion;
    *in >> clientVersion;

    //On vérifie la version.
    if (clientVersion != VERSION)
    {
        console("Le client " + client->getSocket()->peerAddress().toString() +
                " a essayé de se connecter avec un client à la mauvaise version (" + clientVersion + ").");
        Paquet out;
        out << SMSG_AUTH_INCORRECT_VERSION;
        out << VERSION;
        out.send(client->getSocket());
        kickClient(client);
        return;
    }

    client->setSessionState(CHECKED);

    //On peut se connecter.
    Paquet out;
    out << SMSG_HELLO;
    out.send(client->getSocket());
}

void FenPrincipale::handleServerSide(Paquet* /*in*/, Client* /*client*/)
{
    console("Paquet reçu avec un opCode de serveur.");
}

void FenPrincipale::handleAuthLogin(Paquet* in, Client* client)
{
    QString pseudo, login;
    QByteArray pwhash;
    quint8 loginLevel;
    quint32 id = 0;

    *in >> login >> pwhash >> pseudo;

    login = login.simplified().toUpper();
    pwhash = pwhash.toHex();
    pseudo = pseudo.simplified();
    pseudo.remove(' '); //On supprime tous les espaces du pseudo.

    //On vérifie le ban IP
    QSqlQuery query;
    query.prepare("SELECT * FROM ban_ip WHERE ip = :ip");
    query.bindValue(":ip", client->getSocket()->peerAddress().toString());
    if (!query.exec())  //On n'autorise pas le login en cas de problème avec la BDD
    {
        console("ERREUR SQL: " + query.lastError().databaseText());
        Paquet out;
        out << SMSG_AUTH_ERROR;
        out >> client->getSocket();
        kickClient(client);
        return;
    }

    if (query.next())
    {
        //On a trouvé un enregistrement de ban
        QDateTime finBan = query.value(2).toDateTime();
        if (finBan < QDateTime::currentDateTime() && !(finBan == query.value(1).toDateTime()))  //Si on n'a pas un ban infini
        {
            //Le ban est terminé, on le supprime.
            query.prepare("DELETE FROM ban_ip WHERE ip = :ip");
            query.bindValue(":ip", client->getSocket()->peerAddress().toString());
            if (!query.exec())
                console("ERREUR SQL: " + query.lastError().databaseText());
        }
        else
        {
            //Le compte est banni.
            Paquet out;
            out << SMSG_AUTH_IP_BANNED;
            if (finBan != query.value(1).toDateTime())
                out << (quint32) QDateTime::currentDateTime().secsTo(finBan);
            else
                out << (quint32) 0;
            out << query.value(4).toString();
            out >> client->getSocket();
            kickClient(client);
            return;
        }
    }

    //Notre client a spécifié un compte
    if (!login.isEmpty())
    {
        query.prepare("SELECT * FROM account where login = :login AND pwhash = :pwhash");
        query.bindValue(":login", login);
        query.bindValue(":pwhash", pwhash);
        if (!query.exec())
        {
            console("ERREUR SQL: " + query.lastError().databaseText());
            Paquet out;
            out << SMSG_AUTH_ERROR;
            out >> client->getSocket();
            kickClient(client);
            return;
        }

        if (!query.next())
        {
            //On n'a trouvé aucun enregistrement: compte ou mdp incorrect !
            console("Un client a essayé de se connecter avec un mauvais login/mdp.");
            Paquet out;
            out << SMSG_AUTH_INCORRECT_LOGIN;
            out >> client->getSocket();
            kickClient(client);
            return;
        }

        //On a trouvé un enregistrement correspondant au nom et au mot de passe !
        //On récupère son niveau et son id
        loginLevel = query.value(3).toInt();
        id = query.value(0).toInt();

        //On vérifie s'il n'est pas banni.
        query.prepare("SELECT * FROM ban_account WHERE account_id = :id");
        query.bindValue(":id", id);
        if (!query.exec())
            console("ERREUR SQL: " + query.lastError().databaseText());
        if (query.next())
        {
            //On a trouvé un enregistrement de ban
            QDateTime finBan = query.value(2).toDateTime();
            if (finBan < QDateTime::currentDateTime() && !(finBan == query.value(1).toDateTime()))  //Si on n'a pas un ban infini
            {
                //Le ban est terminé, on le supprime.
                query.prepare("DELETE FROM ban_account WHERE account_id = :id");
                query.bindValue(":id", id);
                if (!query.exec())
                    console("ERREUR SQL: " + query.lastError().databaseText());
            }
            else
            {
                //Le compte est banni.
                Paquet out;
                out << SMSG_AUTH_ACCT_BANNED;
                if (finBan != query.value(1).toDateTime())
                    out << (quint32) QDateTime::currentDateTime().secsTo(finBan);
                else
                    out << (quint32) 0;
                out << query.value(4).toString();
                out >> client->getSocket();
                kickClient(client);
                return;
            }
        }

        //On vérifie si personne n'est déjà connecté sous ce nom.
        foreach(Client* i_client, m_clients)
        {
            if (i_client->getAccount() == login)
            {
                console("Un client a essayé de se connecter avec un compte déjà utilisé.");
                Paquet out;
                out << SMSG_AUTH_ACCT_ALREADY_IN_USE;
                out >> client->getSocket();
                kickClient(client);
                return;
            }
        }

        //Le compte est OK.
    }
    else
    {
        //Pas de compte spécifié
        loginLevel = 0;
    }
    //Vérifie la taille du pseudo
    if (pseudo.size() < m_nickMinLength)
    {
        console("ERREUR: Nommage impossible, pseudo trop court.");
        Paquet out;
        out << SMSG_NICK_TOO_SHORT;
        out >> client->getSocket();
        kickClient(client);
        return;
    }

    //Vérifie si le pseudo est déjà utilisé
    foreach(Client* i_client, m_clients)
    {
        if (i_client->getPseudo().compare(pseudo, Qt::CaseInsensitive) == 0)
        {
            console("ERREUR: Nommage impossible, nom déjà utilisé.");
            Paquet out;
            out << SMSG_NICK_ALREADY_IN_USE;
            out >> client->getSocket();
            kickClient(client);
            return;
        }
    }

    //Client authentifié.
    console("Client authentifié : " + pseudo);
    client->setPseudo(pseudo);
    client->setAccount(login);
    client->setLoginLevel(loginLevel);
    client->setIdCompte(id);
    client->setSessionState(AUTHED);

    //Recherche du channel par défaut.
    foreach (Channel *i_channel, m_channels)
    {
        if (i_channel->isDefault())
        {
            i_channel->addUser(client);
            break;
        }
    }

    Paquet out;
    out << SMSG_AUTH_OK;
    out << pseudo;
    out >> client->getSocket();

    out.clear();
    out << SMSG_USER_JOINED;
    out << pseudo << client->getHashIP() << loginLevel;
    envoyerAuChannel(out, client->getChannel());

    return;

}

void FenPrincipale::handleSetNick(Paquet *in, Client *client)
{
    QString pseudo, ancienPseudo;
    *in >> pseudo;
    ancienPseudo = client->getPseudo();

    pseudo = pseudo.simplified();
    pseudo.remove(' '); //On enlève les espaces du pseudo.

    //Vérifie la taille du pseudo
    if (pseudo.size() < m_nickMinLength)
    {
        console("ERREUR: Nommage impossible, pseudo trop court.");
        Paquet out;
        out << SMSG_NICK_TOO_SHORT;
        out >> client->getSocket();
        return;
    }
    if (pseudo.size() > m_nickMaxLength)
    {
        console("ERREUR: Nommage impossible, pseudo trop long.");
        Paquet out;
        out << SMSG_NICK_TOO_LONG;
        out >> client->getSocket();
        return;
    }

    //Vérifie si le pseudo est déjà utilisé
    foreach(Client* i_client, m_clients)
    {
        if (i_client->getPseudo().compare(pseudo, Qt::CaseInsensitive) == 0)
        {
            //Si on modifie notre pseudo, on peut changer nos majuscules/minuscules.
            if (i_client == client && i_client->getPseudo() != pseudo)
                continue;

            console("ERREUR: Nommage impossible, nom déjà utilisé.");
            Paquet out;
            out << SMSG_NICK_ALREADY_IN_USE;
            out >> client->getSocket();
            return;
        }
    }

    //Renommage du client
    console("Client nommé: " + pseudo);
    client->setPseudo(pseudo);

    Paquet out;
    out << SMSG_USER_RENAMED;
    out << ancienPseudo << pseudo;
    envoyerAuChannel(out, client->getChannel());

    return;

}

void FenPrincipale::handleChatMessage(Paquet *in, Client *client)
{
    QString message;
    *in >> message;
    QString pseudo = client->getPseudo();

    //Traitement du message
    message = message.trimmed();

    //On vérifie si le message n'est pas vide.
    if (message.isEmpty())
    {
        Paquet out;
        out << SMSG_INVALID_MESSAGE;
        out >> client->getSocket();
        return;
    }

    //On vérifie que le client a un pseudo valide.
    if (pseudo.size() < m_nickMinLength)
    {
        Paquet out;
        out << SMSG_INVALID_NICK;
        out >> client->getSocket();
        return;
    }

    //Si on est arrivés là, on peut envoyer le message à tous.
    //Préparation du paquet
    Paquet out;
    out << SMSG_CHAT_MESSAGE;
    out << pseudo << message;

    envoyerAuChannel(out, client->getChannel());

    //Archivage dans la BDD
    QSqlQuery query;
    query.prepare("INSERT INTO chat_history (name, channel, message, date) "
                  "VALUES (:name, :channel, :message, :date)");
    query.bindValue(":name", pseudo);
    query.bindValue(":channel", client->getChannel()->getTitre());
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
    if (client->getPseudo().isEmpty())
        console(client->getSocket()->peerAddress().toString() + ": ping " + QString::number(diff) + " ms");
    else
        console(client->getPseudo() + ": ping " + QString::number(diff) + " ms");

    client->setPing(diff);
}

void FenPrincipale::handleRegister(Paquet *in, Client *client)
{
    QString login;
    QByteArray pwhash;

    *in >> login >> pwhash;

    //Vérification des droits.
    if (client->getLoginLevel() < m_registerLevel)
    {
        Paquet out;
        out << SMSG_NOT_AUTHORIZED;
        out >> client->getSocket();
        return;
    }

    //Traitement des données
    login = login.simplified().toUpper();
    pwhash = pwhash.toHex();

    //Vérification de la taille du login
    if (login.size() < m_accountNameMinLength)
    {
        Paquet out;
        out << SMSG_REG_INVALID_NICK;
        out >> client->getSocket();
        return;
    }

    //Vérification des doublons.
    QSqlQuery query;
    query.prepare("SELECT * FROM account WHERE login = :login");
    query.bindValue(":login", login);
    if (!query.exec())
    {
        //Erreur de requête.
        console("ERREUR SQL: " + query.lastError().databaseText());
        Paquet out;
        out << SMSG_REG_ERROR;
        out >> client->getSocket();
        return;
    }

    //Si on a trouvé un compte portant ce nom, on arrête
    if (query.next())
    {
        Paquet out;
        out << SMSG_REG_ACCT_ALREADY_EXISTS;
        out >> client->getSocket();
        return;
    }

    //Insersion dans la base de données.
    query.prepare("INSERT INTO account (login, pwhash, level) VALUES (:login, :pwhash, 1)");
    query.bindValue(":login", login);
    query.bindValue(":pwhash", pwhash);
    if (!query.exec())
    {
        //Erreur de requête.
        console("ERREUR SQL: " + query.lastError().databaseText());
        Paquet out;
        out << SMSG_REG_ERROR;
        out >> client->getSocket();
        return;
    }



    console("Nouveau compte enregistré: " + login);
    Paquet out;
    out << SMSG_REG_OK << login;
    out >> client->getSocket();
}

void FenPrincipale::handleKick(Paquet *in, Client *client)
{
    //On vérifie le niveau d'authentification
    if (client->getLoginLevel() < m_kickLevel)
    {
        Paquet out;
        out << SMSG_NOT_AUTHORIZED;
        out >> client->getSocket();
        return;
    }

    //On vérifie que la personne existe
    QString pseudo, raison;
    *in >> pseudo >> raison;

    raison.simplified();

    if (raison.isEmpty())
        raison = "No reason set.";

    Client* clientAKicker = NULL;
    foreach (Client *i_client, m_clients)
    {
        if (i_client->getPseudo().compare(pseudo, Qt::CaseInsensitive) == 0)
        {
            clientAKicker = i_client;
            break;
        }
    }

    //Si on a pas trouvé, on ne kick pas
    if (!clientAKicker)
    {
        Paquet out;
        out << SMSG_USER_DOESNT_EXIST;
        out >> client->getSocket();
        return;
    }

    //On vérifie que la cible n'est pas de plus haut niveau.
    if (clientAKicker->getLoginLevel() >= client->getLoginLevel())
    {
        Paquet out;
        out << SMSG_NO_INTERACT_HIGHER_LEVEL;
        out >> client->getSocket();
        return;
    }

    //On annonce le kick
    Paquet out;
    out << SMSG_USER_KICKED;
    out << client->getPseudo(); //On dit qui a kické.
    out << clientAKicker->getPseudo(); //On prend le pseudo réel (majuscules...)
    out << raison;
    envoyerAuChannel(out, client->getChannel());

    //On kicke le client.
    kickClient(clientAKicker);
}

void FenPrincipale::handleBan(Paquet *in, Client *client)
{
    //On vérifie le niveau d'authentification
    if (client->getLoginLevel() < m_kickLevel)
    {
        Paquet out;
        out << SMSG_NOT_AUTHORIZED;
        out >> client->getSocket();
        return;
    }

    //On vérifie que la personne existe
    QString pseudo, raison;
    quint32 duree;
    *in >> pseudo >> duree >> raison;

    raison.simplified();

    if (raison.isEmpty())
        raison = "No reason set.";

    Client* clientABannir = NULL;

    foreach (Client *i_client, m_clients)
    {
        if (i_client->getPseudo().compare(pseudo, Qt::CaseInsensitive) == 0)
        {
            clientABannir = i_client;
            break;
        }
    }

    //Si on a pas trouvé, on ne ban pas
    if (!clientABannir)
    {
        Paquet out;
        out << SMSG_USER_DOESNT_EXIST;
        out >> client->getSocket();
        return;
    }

    //On vérifie que la cible n'est pas plus haut niveau
    if (clientABannir->getLoginLevel() >= client->getLoginLevel())
    {
        Paquet out;
        out << SMSG_NO_INTERACT_HIGHER_LEVEL;
        out >> client->getSocket();
        return;
    }

    //On enregistre le ban si le client est connecté avec un compte.
    QSqlQuery query;
    if (!clientABannir->getAccount().isEmpty())
    {
        query.prepare("INSERT INTO ban_account (account_id, bandate, unbandate, bannedby, reason) "
                      "VALUES (:id, :bandate, :unbandate, :bannedby, :reason)");
        query.bindValue(":id", clientABannir->getIdCompte());
        query.bindValue(":bandate", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"));
        query.bindValue(":unbandate", QDateTime::currentDateTime().addSecs(duree).toString("yyyy-MM-dd hh:mm:ss:zzz"));
        query.bindValue(":bannedby", client->getPseudo());
        query.bindValue(":reason", raison);
        if (!query.exec())
            console("ERREUR SQL: " + query.lastError().databaseText());
    }

    //Ainsi que le ban IP.
    query.prepare("INSERT INTO ban_ip (ip, bandate, unbandate, bannedby, reason) "
                  "VALUES (:ip, :bandate, :unbandate, :bannedby, :reason)");
    query.bindValue(":ip", clientABannir->getSocket()->peerAddress().toString());
    query.bindValue(":bandate", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"));
    query.bindValue(":unbandate", QDateTime::currentDateTime().addSecs(duree).toString("yyyy-MM-dd hh:mm:ss:zzz"));
    query.bindValue(":bannedby", client->getPseudo());
    query.bindValue(":reason", raison);
    if (!query.exec())
        console("ERREUR SQL: " + query.lastError().databaseText());

    //On annonce le ban
    Paquet out;
    out << SMSG_USER_BANNED;
    out << client->getPseudo(); //On dit qui a banni.
    out << clientABannir->getPseudo(); //On prend le pseudo réel (majuscules...)
    out << raison;
    envoyerAuChannel(out, client->getChannel());

    //On kick le client.
    kickClient(clientABannir);
}

void FenPrincipale::handleVoice(Paquet */*in*/, Client */*client*/)
{

}

void FenPrincipale::handleLevelMod(Paquet *in, Client *client)
{
    QString login;
    quint8 level;

    *in >> login >> level;

    login = login.toUpper();
    Client *clientAModifier = NULL;

    //On détermine le client à modifier (s'il existe)
    foreach (Client *i_client, m_clients)
    {
        if (i_client->getAccount() == login)
        {
            clientAModifier = i_client;
            break;
        }
    }

    //Vérification du niveau
    if (level > m_levelMax || level < 1)
    {
        Paquet out;
        out << SMSG_LVL_MOD_INVALID_LEVEL;
        out >> client->getSocket();
        return;
    }

    //On ne modifie pas son niveau.
    if (login == client->getAccount())
    {
        Paquet out;
        out << SMSG_LVL_MOD_NOT_YOURSELF;
        out >> client->getSocket();
        return;
    }

    //Modification du compte.
    QSqlQuery query;
    query.prepare("SELECT * FROM account WHERE login = :login");
    query.bindValue(":login", login);
    if (!query.exec())
    {
        console("ERREUR SQL: " + query.lastError().databaseText());
        Paquet out;
        out << SMSG_PROMOTE_ERROR;
        out >> client->getSocket();
        return;
    }

    if (!query.next())
    {
        //Compte non trouvé
        Paquet out;
        out << SMSG_LVL_MOD_ACCT_DOESNT_EXIST;
        out >> client->getSocket();
        return;
    }

    quint8 acctLevel = query.value(3).toInt();

    //Vérifications concernant le compte.
    if (client->getLoginLevel() <= acctLevel)
    {
        Paquet out;
        out << SMSG_NO_INTERACT_HIGHER_LEVEL;
        out >> client->getSocket();
        return;
    }
    if (acctLevel > client->getLoginLevel())
    {
        Paquet out;
        out << SMSG_LVL_MOD_LEVEL_TOO_HIGH;
        out >> client->getSocket();
        return;
    }

    query.prepare("UPDATE account SET level = :level WHERE login = :login");
    query.bindValue(":level", level);
    query.bindValue(":login", login);
    if (!query.exec())
        console("ERREUR SQL: " + query.lastError().databaseText());

    //Modification du niveau d'administration.
    Paquet out;

    if (clientAModifier)
    {
        clientAModifier->setLoginLevel(level);

        out << SMSG_LVL_CHANGED;
        out << client->getPseudo() << level;
        out >> clientAModifier->getSocket();
        out.clear();
    }

    out << SMSG_LVL_MOD_OK;
    out >> client->getSocket();
    return;
}

void FenPrincipale::handleWhoIs(Paquet *in, Client *client)
{
    if (client->getLoginLevel() < m_whoisLevel)
    {
        Paquet out;
        out << SMSG_NOT_AUTHORIZED;
        out >> client->getSocket();
        return;
    }

    QString pseudo;
    Client *clientCible = NULL;
    *in >> pseudo;

    //On recherche le client.
    foreach (Client* i_client, m_clients)
    {
        if (i_client->getPseudo().compare(pseudo, Qt::CaseInsensitive) == false)
        {
            clientCible = i_client;
            break;
        }
    }

    if (!clientCible)
    {
        //On n'a pas trouvé de client avec ce pseudo.
        Paquet out;
        out << SMSG_USER_DOESNT_EXIST;
        out >> client->getSocket();
        return;
    }

    //On envoie les informations.
    //Pseudo, compte, niveau, ping, hashIP
    Paquet out;
    out << SMSG_WHOIS;
    out << clientCible->getPseudo();
    out << clientCible->getAccount();
    out << clientCible->getLoginLevel();
    out << clientCible->getPing();
    out << clientCible->getHashIP();
    out >> client->getSocket();
}

void FenPrincipale::handleUpdateClientsList(Paquet */*in*/, Client *client)
{
    //OpCode - size - pseudos
    QStringList pseudos;
    quint32 size = 0;

    foreach (Client* i_client, client->getChannel()->getClients())
    {
        size++;
        pseudos << i_client->getPseudo();
    }

    Paquet out;
    out << SMSG_CLIENTS_LIST;
    out << size;
    for (quint32 i = 0; i < size; i++)
        out << pseudos[i];
    out.send(client->getSocket());
}

void FenPrincipale::handleLogout(Paquet *in, Client *client)
{
    QString raison;
    *in >> raison;

    client->setLogoutMessage(raison);

    client->getSocket()->disconnectFromHost();
}

void FenPrincipale::handleSetLogoutMsg(Paquet *in, Client *client)
{
    QString msg;
    *in >> msg;

    client->setLogoutMessage(msg);
}

void FenPrincipale::handleUpdateChannel(Paquet */*in*/, Client *client)
{
    //OpCode - size - channels
    quint32 size = m_channels.size();

    Paquet out;
    out << SMSG_CHANNEL << size;

    for (quint32 i = 0; i < size; i++)
    {
        out << m_channels[i]->getId();
        out << m_channels[i]->getTitre();
        out << !m_channels[i]->getPassword().isEmpty();
    }

    out >> client->getSocket();

}

void FenPrincipale::handleChannelJoin(Paquet *in, Client *client)
{
    quint32 id;
    QString password;
    *in >> id >> password;

    Channel *channel = 0;
    foreach(Channel *i_channel, m_channels)
    {
        if (i_channel->getId() == id)
        {
            channel = i_channel;
            break;
        }
    }

    if (!channel)
    {
        console("ERREUR: Channel non trouvé pour le client " + client->getPseudo());
        return;
    }

    //Si on va dans notre channel actuel, il ne se passe rien
    if (channel->getId() == client->getChannel()->getId())
        return;

    //Vérifie le niveau
    if (client->getLoginLevel() < channel->getReqLevel())
    {
        Paquet out;
        out << SMSG_CHANNEL_LVL_TOO_LOW;
        out >> client->getSocket();
        return;
    }

    //Vérifie le mot de passe
    if (channel->getPassword() != password)
    {
        Paquet out;
        out << SMSG_CHANNEL_WRONG_PASSWORD;
        out >> client->getSocket();
        return;
    }

    //Notification
    changerChannel(client, channel);

    //Mise à jour de la liste de connectés
    handleUpdateClientsList(0, client);
}

void FenPrincipale::handleChannelCreate(Paquet *in, Client *client)
{
    QString titre, password;
    quint8 reqLevel;
    bool persistant;

    *in >> titre >> password >> reqLevel >> persistant;

    //Vérification des autorisations.
    if (client->getLoginLevel() < m_channelCreateLevel)
    {
        Paquet out;
        out << SMSG_NOT_AUTHORIZED;
        out >> client->getSocket();
        return;
    }
    if (persistant && client->getLoginLevel() < m_channelCreatePersistantLevel)
    {
        Paquet out;
        out << SMSG_NOT_AUTHORIZED;
        out >> client->getSocket();
        return;
    }

    //Ajout de l'enregistrement dans la base de données.
    QSqlQuery query;
    query.prepare("INSERT INTO channel (name, password, join_level, created_by_acct, persistant) "
                  "VALUES (:name, :password, :join_level, :created_by_acct, :persistant)");
    query.bindValue(":name", titre);
    query.bindValue(":password", password);
    query.bindValue(":join_level", reqLevel);
    query.bindValue(":created_by_acct", client->getAccount());
    query.bindValue(":persistant", persistant);
    if (!query.exec())
    {
        console("Impossible de créer le canal " + titre + ". Raison: " + query.lastError().text());
        Paquet out;
        out << SMSG_CHANNEL_UNABLE_TO_CREATE;
        out >> client->getSocket();
        return;
    }

    //Recherche de l'ID du channel.
    query.prepare("SELECT id FROM channel ORDER BY id DESC LIMIT 1");
    if (!query.exec())
        console(query.lastError().text());
    query.first();
    quint32 id = query.value(0).toUInt();

    //Création de l'objet canal.
    Channel *channel = new Channel(id, titre, reqLevel, password, client->getAccount(), persistant, false, this);
    connect(channel, SIGNAL(channelNeedsToBeRemoved(Channel*)), this, SLOT(supprimerChannel(Channel*)));
    m_channels.append(channel);

    //Mise à jour des clients.
    foreach (Client* i_client, m_clients)
        handleUpdateChannel(0, i_client);

    //Le créateur du canal est automatiquement ajouté au canal
    changerChannel(client, channel);
}
void FenPrincipale::handleChannelDelete(Paquet *in, Client *client)
{
    quint32 id = 0;
    *in >> id;

    if (client->getLoginLevel() < m_channelDeleteLevel)
    {
        Paquet out;
        out << SMSG_NOT_AUTHORIZED;
        out >> client->getSocket();
        return;
    }

    Channel* channel = 0;
    foreach(channel, m_channels)
    {
        if (channel->getId() == id)
            break;
    }

    //Si on n'a pas trouvé notre canal ou si c'est le canal par défaut, on ne supprime rien
    if (!channel || channel->isDefault())
    {
        Paquet out;
        out << SMSG_CHANNEL_UNABLE_TO_DELETE;
        out >> client->getSocket();
        return;
    }

    //Suppression de la BDD
    QSqlQuery query;
    query.prepare("DELETE FROM channel WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec())
    {
        console(query.lastError().text());
        Paquet out;
        out << SMSG_CHANNEL_UNABLE_TO_DELETE;
        out >> client->getSocket();
        return;
    }

    //Déplacement de tous les membres du canal vers le canal par défaut.
    foreach(Channel* i_channel, m_channels)
    {
        if (i_channel->isDefault())
        {
            foreach(Client* i_client, channel->getClients())
            {
                changerChannel(i_client, i_channel);
            }
            break;
        }
    }

    //Suppression du canal du serveur
    m_channels.removeOne(channel);

    //Mise à jour des clients;
    foreach (Client* i_client, m_clients)
        handleUpdateChannel(0, i_client);
}
void FenPrincipale::handleChannelEdit(Paquet *in, Client *client)
{
}
