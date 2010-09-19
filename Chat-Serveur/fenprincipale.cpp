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
            CONSOLE("Connexion r�ussie � la BDD");
        }
        else
        {
            CONSOLE("Connexion �chou�e � la BDD: " + db.lastError().text());
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
    //Nouvelle connexion : on cr�e l'objet Client, on fait les connexions
    Client *nouveauClient = new Client(m_serveur->nextPendingConnection());
    connect(nouveauClient, SIGNAL(deconnecte()), this, SLOT(decoClient()));
    connect(nouveauClient, SIGNAL(console(QString)), this, SLOT(console(QString)));
    connect(nouveauClient, SIGNAL(paquetRecu(Paquet*)), this, SLOT(paquetRecu(Paquet*)));

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

    //On envoie le message de d�connexion � tous si la personne a un pseudo.
    if (!client->getPseudo().isEmpty())
    {
        Paquet out;
        out << SMSG_USER_LEFT;
        out << client->getPseudo();
        envoyerATous(out);
    }

    //On le supprime de la liste de clients
    m_clients.removeOne(client);

    //Et on le d�salloue
    delete client;
}

void FenPrincipale::kickClient(Client *client)
{
    CONSOLE("Le client " + client->getSocket()->peerAddress().toString() + " a �t� kick�.");
    Paquet out;
    out << SMSG_KICK;
    out >> client->getSocket();
    client->getSocket()->disconnectFromHost();
}

void FenPrincipale::paquetRecu(Paquet *in)
{
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

    CONSOLE("Paquet re�u: " + handler.nom + "(" + QString::number(opCode) + ")");

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
        paquet >> client->getSocket();
    }
}

void FenPrincipale::handleHello(Paquet* in, Client* client)
{
    Paquet out;
    out << SMSG_HELLO;

    out >> client->getSocket();
}

void FenPrincipale::handleServerSide(Paquet* in, Client* client)
{
    CONSOLE("Paquet re�u avec un opCode de serveur.");
}

//OpCode re�u lors de la premi�re connexion du client.
void FenPrincipale::handleAuthLogin(Paquet* in, Client* client)
{
    QString pseudo, login;
    QByteArray pwhash;
    quint8 authLevel;
    quint32 id;

    *in >> login >> pwhash >> pseudo;

    login = login.simplified().toUpper();
    pwhash = pwhash.toHex();
    pseudo = pseudo.simplified();
    pseudo.remove(' '); //On supprime tous les espaces du pseudo.

    //On v�rifie le ban IP
    QSqlQuery query;
    query.prepare("SELECT * FROM ban_ip WHERE ip = :ip");
    query.bindValue(":ip", client->getSocket()->peerAddress().toString());
    if (!query.exec())
    {
        CONSOLE("ERREUR SQL: " + query.lastError().databaseText());
        Paquet out;
        out << SMSG_AUTH_ERROR;
        out >> client->getSocket();
        kickClient(client);
        return;
    }

    if (query.next())
    {
        //On a trouv� un enregistrement de ban
        QDateTime finBan = query.value(2).toDateTime();
        if (finBan < QDateTime::currentDateTime() && !(finBan == query.value(1).toDateTime()))  //Si on n'a pas un ban infini
        {
            //Le ban est termin�, on le supprime.
            query.prepare("DELETE FROM ban_ip WHERE ip = :ip");
            query.bindValue(":ip", client->getSocket()->peerAddress().toString());
            if (!query.exec())
                CONSOLE("ERREUR SQL: " + query.lastError().databaseText());
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

    //Notre client a sp�cifi� un compte
    if (!login.isEmpty())
    {
        query.prepare("SELECT * FROM account where login = :login AND pwhash = :pwhash");
        query.bindValue(":login", login);
        query.bindValue(":pwhash", pwhash);
        if (!query.exec())
        {
            CONSOLE("ERREUR SQL: " + query.lastError().databaseText());
            Paquet out;
            out << SMSG_AUTH_ERROR;
            out >> client->getSocket();
            kickClient(client);
            return;
        }

        if (!query.next())
        {
            //On n'a trouv� aucun enregistrement: compte ou mdp incorrect !
            CONSOLE("Un client a essay� de se connecter avec un mauvais login/mdp.");
            Paquet out;
            out << SMSG_AUTH_INCORRECT_LOGIN;
            out >> client->getSocket();
            kickClient(client);
            return;
        }

        //On a trouv� un enregistrement correspondant au nom et au mot de passe !
        //On r�cup�re son niveau et son id
        authLevel = query.value(3).toInt();
        id = query.value(0).toInt();

        //On v�rifie s'il n'est pas banni.
        query.prepare("SELECT * FROM ban_account WHERE account_id = :id");
        query.bindValue(":id", id);
        if (!query.exec())
            CONSOLE("ERREUR SQL: " + query.lastError().databaseText());
        if (query.next())
        {
            //On a trouv� un enregistrement de ban
            QDateTime finBan = query.value(2).toDateTime();
            if (finBan < QDateTime::currentDateTime() && !(finBan == query.value(1).toDateTime()))  //Si on n'a pas un ban infini
            {
                //Le ban est termin�, on le supprime.
                query.prepare("DELETE FROM ban_account WHERE account_id = :id");
                query.bindValue(":id", id);
                if (!query.exec())
                    CONSOLE("ERREUR SQL: " + query.lastError().databaseText());
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

        //On v�rifie si personne n'est d�j� connect� sous ce nom.
        foreach(Client* i_client, m_clients)
        {
            if (i_client->getAccount() == login)
            {
                CONSOLE("Un client a essay� de se connecter avec un compte d�j� utilis�.");
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
        //Pas de compte sp�cifi�
        authLevel = 0;
    }
    //V�rifie la taille du pseudo
    if (pseudo.size() < TAILLE_PSEUDO_MIN)
    {
        CONSOLE("ERREUR: Nommage impossible, pseudo trop court.");
        Paquet out;
        out << SMSG_NICK_TOO_SHORT;
        out >> client->getSocket();
        kickClient(client);
        return;
    }

    //V�rifie si le pseudo est d�j� utilis�
    foreach(Client* i_client, m_clients)
    {
        if (i_client->getPseudo().compare(pseudo, Qt::CaseInsensitive) == 0)
        {
            CONSOLE("ERREUR: Nommage impossible, nom d�j� utilis�.");
            Paquet out;
            out << SMSG_NICK_ALREADY_IN_USE;
            out >> client->getSocket();
            kickClient(client);
            return;
        }
    }

    //Client authentifi�.
    CONSOLE("Client authentifi� : " + pseudo);
    client->setPseudo(pseudo);
    client->setAccount(login);
    client->setAuthLevel(authLevel);
    client->setIdCompte(id);

    Paquet out;
    out << SMSG_AUTH_OK;
    out << pseudo;
    out >> client->getSocket();

    out.clear();
    out << SMSG_USER_JOINED;
    out << pseudo << client->getHashIP() << authLevel;
    envoyerATous(out);

    return;

}

void FenPrincipale::handleSetNick(Paquet *in, Client *client)
{
    QString pseudo, ancienPseudo;
    *in >> pseudo;
    ancienPseudo = client->getPseudo();

    pseudo = pseudo.simplified();
    pseudo.remove(' '); //On enl�ve les espaces du pseudo.

    //V�rifie la taille du pseudo
    if (pseudo.size() < TAILLE_PSEUDO_MIN)
    {
        CONSOLE("ERREUR: Nommage impossible, pseudo trop court.");
        Paquet out;
        out << SMSG_NICK_TOO_SHORT;
        out >> client->getSocket();
        return;
    }

    //V�rifie si le pseudo est d�j� utilis�
    foreach(Client* i_client, m_clients)
    {
        if (i_client->getPseudo().compare(pseudo, Qt::CaseInsensitive) == 0)
        {
            CONSOLE("ERREUR: Nommage impossible, nom d�j� utilis�.");
            Paquet out;
            out << SMSG_NICK_ALREADY_IN_USE;
            out >> client->getSocket();
            return;
        }
    }

    //Renommage du client
    CONSOLE("Client nomm�: " + pseudo);
    client->setPseudo(pseudo);

    Paquet out;
    out << SMSG_USER_RENAMED;
    out << ancienPseudo << pseudo;
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

    //On v�rifie si le message n'est pas vide.
    if (message.isEmpty())
    {
        Paquet out;
        out << SMSG_INVALID_MESSAGE;
        out >> client->getSocket();
        return;
    }

    //On v�rifie que le client a un pseudo valide.
    if (pseudo.size() < TAILLE_PSEUDO_MIN)
    {
        Paquet out;
        out << SMSG_INVALID_NICK;
        out >> client->getSocket();
        return;
    }

    //Si on est arriv�s l�, on peut envoyer le message � tous.
    //Pr�paration du paquet
    Paquet out;
    out << SMSG_CHAT_MESSAGE;
    out << pseudo << message;

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
    //On a re�u un ping, on remet le d�compte de kick � 0
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

    *in >> login >> pwhash;

    //V�rification des droits.
    if (client->getAuthLevel() < REGISTER_LVL)
    {
        Paquet out;
        out << SMSG_NOT_AUTHORIZED;
        out >> client->getSocket();
        return;
    }

    //Traitement des donn�es
    login = login.simplified().toUpper();
    pwhash = pwhash.toHex();

    //V�rification de la taille du login
    if (login.size() < TAILLE_COMPTE_MIN)
    {
        Paquet out;
        out << SMSG_REG_INVALID_NICK;
        out >> client->getSocket();
        return;
    }

    //V�rification des doublons.
    QSqlQuery query;
    query.prepare("SELECT * FROM account WHERE login = :login");
    query.bindValue(":login", login);
    if (!query.exec())
    {
        //Erreur de requ�te.
        CONSOLE("ERREUR SQL: " + query.lastError().databaseText());
        Paquet out;
        out << SMSG_REG_ERROR;
        out >> client->getSocket();
        return;
    }

    //Si on a trouv� un compte portant ce nom, on arr�te
    if (query.next())
    {
        Paquet out;
        out << SMSG_REG_ACCT_ALREADY_EXISTS;
        out >> client->getSocket();
        return;
    }

    //Insersion dans la base de donn�es.
    query.prepare("INSERT INTO account (login, pwhash, level) VALUES (:login, :pwhash, 1)");
    query.bindValue(":login", login);
    query.bindValue(":pwhash", pwhash);
    if (!query.exec())
    {
        //Erreur de requ�te.
        CONSOLE("ERREUR SQL: " + query.lastError().databaseText());
        Paquet out;
        out << SMSG_REG_ERROR;
        out >> client->getSocket();
        return;
    }



    CONSOLE("Nouveau compte enregistr�: " + login);
    Paquet out;
    out << SMSG_REG_OK << login;
    out >> client->getSocket();
}

void FenPrincipale::handleKick(Paquet *in, Client *client)
{
    //On v�rifie le niveau d'authentification
    if (client->getAuthLevel() < KICK_LVL)
    {
        Paquet out;
        out << SMSG_NOT_AUTHORIZED;
        out >> client->getSocket();
        return;
    }

    //On v�rifie que la personne existe
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

    //Si on a pas trouv�, on ne kick pas
    if (!clientAKicker)
    {
        Paquet out;
        out << SMSG_USER_DOESNT_EXIST;
        out >> client->getSocket();
        return;
    }

    //On v�rifie que la cible n'est pas de plus haut niveau.
    if (clientAKicker->getAuthLevel() >= client->getAuthLevel())
    {
        Paquet out;
        out << SMSG_NO_INTERACT_HIGHER_LEVEL;
        out >> client->getSocket();
        return;
    }

    //On annonce le kick
    Paquet out;
    out << SMSG_USER_KICKED;
    out << client->getPseudo(); //On dit qui a kick�.
    out << clientAKicker->getPseudo(); //On prend le pseudo r�el (majuscules...)
    out << raison;
    envoyerATous(out);

    //On kicke le client.
    kickClient(clientAKicker);
}

void FenPrincipale::handleBan(Paquet *in, Client *client)
{
    //On v�rifie le niveau d'authentification
    if (client->getAuthLevel() < BAN_LVL)
    {
        Paquet out;
        out << SMSG_NOT_AUTHORIZED;
        out >> client->getSocket();
        return;
    }

    //On v�rifie que la personne existe
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

    //Si on a pas trouv�, on ne ban pas
    if (!clientABannir)
    {
        Paquet out;
        out << SMSG_USER_DOESNT_EXIST;
        out >> client->getSocket();
        return;
    }

    //On v�rifie que la cible n'est pas plus haut niveau
    if (clientABannir->getAuthLevel() >= client->getAuthLevel())
    {
        Paquet out;
        out << SMSG_NO_INTERACT_HIGHER_LEVEL;
        out >> client->getSocket();
        return;
    }

    //On enregistre le ban si le client est connect� avec un compte.
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
            CONSOLE("ERREUR SQL: " + query.lastError().databaseText());
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
        CONSOLE("ERREUR SQL: " + query.lastError().databaseText());

    //On annonce le ban
    Paquet out;
    out << SMSG_USER_BANNED;
    out << client->getPseudo(); //On dit qui a banni.
    out << clientABannir->getPseudo(); //On prend le pseudo r�el (majuscules...)
    out << raison;
    envoyerATous(out);

    //On kick le client.
    kickClient(clientABannir);
}

void FenPrincipale::handleVoice(Paquet *in, Client *client)
{

}

void FenPrincipale::handleLevelMod(Paquet *in, Client *client)
{
    QString login;
    quint8 level;

    *in >> login >> level;

    login = login.toUpper();
    Client *clientAModifier = NULL;

    //On d�termine le client � modifier (s'il existe)
    foreach (Client *i_client, m_clients)
    {
        if (i_client->getAccount() == login)
        {
            clientAModifier = i_client;
            break;
        }
    }

    //V�rification du niveau
    if (level > LVL_MAX || level < 1)
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
        CONSOLE("ERREUR SQL: " + query.lastError().databaseText());
        Paquet out;
        out << SMSG_PROMOTE_ERROR;
        out >> client->getSocket();
        return;
    }

    if (!query.next())
    {
        //Compte non trouv�
        Paquet out;
        out << SMSG_LVL_MOD_ACCT_DOESNT_EXIST;
        out >> client->getSocket();
        return;
    }

    quint8 acctLevel = query.value(3).toInt();

    //V�rifications concernant le compte.
    if (client->getAuthLevel() <= acctLevel)
    {
        Paquet out;
        out << SMSG_NO_INTERACT_HIGHER_LEVEL;
        out >> client->getSocket();
        return;
    }
    if (acctLevel > client->getAuthLevel())
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
        CONSOLE("ERREUR SQL: " + query.lastError().databaseText());

    //Modification du niveau d'administration.
    Paquet out;

    if (clientAModifier)
    {
        clientAModifier->setAuthLevel(level);

        out << SMSG_LVL_CHANGED;
        out << client->getPseudo() << level;
        out >> clientAModifier->getSocket();
        out.clear();
    }

    out << SMSG_LVL_MOD_OK;
    out >> client->getSocket();
    return;
}
