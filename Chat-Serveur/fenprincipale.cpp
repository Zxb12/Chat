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
    out.send(client->getSocket());
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

//OpCode re�u lors de la premi�re connexion du client.
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

    //Notre client a sp�cifi� un compte
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
            //On n'a trouv� aucun enregistrement: compte ou mdp incorrect !
            CONSOLE("Un client a essay� de se connecter avec un mauvais login/mdp.");
            Paquet out;
            out << SMSG_AUTH_INCORRECT_LOGIN;
            out.send(client->getSocket());
            kickClient(client);
            return;
        }

        //On a trouv� un enregistrement correspondant au nom et au mot de passe !
        //On r�cup�re son niveau
        authLevel = query.value(3).toInt();

        //On v�rifie si personne n'est d�j� connect� sous ce nom.
        foreach(Client* i_client, m_clients)
        {
            if (i_client->getAccount() == login)
            {
                CONSOLE("Un client a essay� de se connecter avec un compte d�j� utilis�.");
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
        //Pas de compte sp�cifi�
        authLevel = 0;
    }
    //V�rifie la taille du pseudo
    if (pseudo.size() < TAILLE_PSEUDO_MIN)
    {
        CONSOLE("ERREUR: Nommage impossible, pseudo trop court.");
        Paquet out;
        out << SMSG_NICK_TOO_SHORT;
        out.send(client->getSocket());
        kickClient(client);
        return;
    }

    //V�rifie si le pseudo est d�j� utilis�
    foreach(Client* i_client, m_clients)
    {
        if (i_client->getPseudo().compare(pseudo, Qt::CaseInsensitive) == false)
        {
            CONSOLE("ERREUR: Nommage impossible, nom d�j� utilis�.");
            Paquet out;
            out << SMSG_NICK_ALREADY_IN_USE;
            out.send(client->getSocket());
            kickClient(client);
            return;
        }
    }

    //Client authentifi�.
    CONSOLE("Client authentifi� : " + pseudo);
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

    //V�rifie la taille du pseudo
    if (pseudo.size() < TAILLE_PSEUDO_MIN)
    {
        CONSOLE("ERREUR: Nommage impossible, pseudo trop court.");
        Paquet out;
        out << SMSG_NICK_TOO_SHORT;
        out.send(client->getSocket());
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
            out.send(client->getSocket());
            return;
        }
    }

    //Renommage du client
    CONSOLE("Client nomm�: " + pseudo);
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

    //On v�rifie si le message n'est pas vide.
    if (message.isEmpty())
    {
        Paquet out;
        out << SMSG_INVALID_MESSAGE;
        out.send(client->getSocket());
        return;
    }

    //On v�rifie que le client a un pseudo valide.
    if (pseudo.size() < TAILLE_PSEUDO_MIN)
    {
        Paquet out;
        out << SMSG_INVALID_NICK;
        out.send(client->getSocket());
        return;
    }

    //Si on est arriv�s l�, on peut envoyer le message � tous.
    //Pr�paration du paquet
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

    *in >> login;
    *in >> pwhash;

    //Traitement des donn�es
    login = login.simplified().toUpper();
    pwhash = pwhash.toHex();

    //V�rification de la taille du login
    if (login.size() < TAILLE_COMPTE_MIN)
    {
        Paquet out;
        out << SMSG_REG_INVALID_NICK;
        out.send(client->getSocket());
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
        out.send(client->getSocket());
        return;
    }

    //Si on a trouv� un compte portant ce nom, on arr�te
    if (query.next())
    {
        Paquet out;
        out << SMSG_REG_ACCT_ALREADY_EXISTS;
        out.send(client->getSocket());
        return;
    }

    //Insersion dans la base de donn�es.
    query.prepare("INSERT INTO account (login, pwhash) VALUES (:login, :pwhash)");
    query.bindValue(":login", login);
    query.bindValue(":pwhash", pwhash);
    if (!query.exec())
    {
        //Erreur de requ�te.
        CONSOLE("ERREUR SQL: " + query.lastError().databaseText());
        Paquet out;
        out << SMSG_REG_ERROR;
        out.send(client->getSocket());
        return;
    }

    CONSOLE("Nouveau compte enregistr�: " + login);
    Paquet out;
    out << SMSG_REG_OK;
    out.send(client->getSocket());
}
