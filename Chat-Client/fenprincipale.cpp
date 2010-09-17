#include "fenprincipale.h"
#include "ui_fenprincipale.h"

#define CONSOLE(a) ui->console->append(QTime::currentTime().toString() + " " + a)
#define CHAT(a)    ui->chat->appendHtml(a)

FenPrincipale::FenPrincipale(QWidget *parent) : QWidget(parent), ui(new Ui::FenPrincipale), m_taillePaquet(0), m_acctName("")
{
    ui->setupUi(this);

    m_socket = new QTcpSocket(this);
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(donneesRecues()));
    connect(m_socket, SIGNAL(connected()), this, SLOT(connecte()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(deconnecte()));
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(erreurSocket(QAbstractSocket::SocketError)));

    ui->chat->setEnabled(false);
    ui->message->setEnabled(false);
    ui->envoyer->setEnabled(false);

    ui->connecter->setFocus();
}

FenPrincipale::~FenPrincipale()
{
    delete m_socket;
    delete ui;
}

void FenPrincipale::on_pseudo_returnPressed()
{
    on_connecter_clicked();
}


void FenPrincipale::on_connecter_clicked()
{
    // On annonce sur la fenêtre qu'on est en train de se connecter
    CONSOLE("Tentative de connexion en cours...");
    ui->connecter->setEnabled(false);

    m_socket->abort(); // On désactive les connexions précédentes s'il y en a
    m_socket->connectToHost(ui->adresse->text(), ui->port->value()); // On se connecte au serveur demandé
}

// Envoi d'un message au serveur
void FenPrincipale::on_envoyer_clicked()
{
    QString msg = ui->message->text();
    handleChatCommands(msg);

    if (m_socket->isWritable() && !msg.isEmpty())
    {
        Paquet out;
        out << CMSG_CHAT_MESSAGE;
        out << msg;
        out.send(m_socket);
    }

    ui->message->clear(); // On vide la zone d'écriture du message
    ui->message->setFocus(); // Et on remet le curseur à l'intérieur
}

// Appuyer sur la touche Entrée a le même effet que cliquer sur le bouton "Envoyer"
void FenPrincipale::on_message_returnPressed()
{
    on_envoyer_clicked();
}

// On a reçu un paquet (ou un sous-paquet)
void FenPrincipale::donneesRecues()
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

// Ce slot est appelé lorsque la connexion au serveur a réussi
void FenPrincipale::connecte()
{
    CONSOLE("Connexion réussie !");
    ui->connecter->setEnabled(true);

    QByteArray pwhash = QCryptographicHash::hash(ui->password->text().toUtf8(), QCryptographicHash::Sha1);

    //On demande au serveur de nous attribuer un pseudo.
    Paquet out;
    out << CMSG_AUTH_LOGIN;
    out << ui->login->text();
    out << pwhash;
    out << ui->pseudo->text();

    out.send(m_socket);

    //On sélectionne la zone de message.
    ui->message->setFocus();
}

// Ce slot est appelé lorsqu'on est déconnecté du serveur
void FenPrincipale::deconnecte()
{
    CONSOLE("Déconnecté du serveur");
    m_pseudo.clear();
    ui->chat->setEnabled(false);
    ui->message->setEnabled(false);
    ui->envoyer->setEnabled(false);
}

// Ce slot est appelé lorsqu'il y a une erreur
void FenPrincipale::erreurSocket(QAbstractSocket::SocketError erreur)
{
    switch(erreur) // On affiche un message différent selon l'erreur qu'on nous indique
    {
    case QAbstractSocket::HostNotFoundError:
        CONSOLE("ERREUR : le serveur n'a pas pu être trouvé. Vérifiez l'IP et le port.");
        break;
    case QAbstractSocket::ConnectionRefusedError:
        CONSOLE("ERREUR : le serveur a refusé la connexion. Vérifiez si le programme \"serveur\" a bien été lancé. Vérifiez aussi l'IP et le port.");
        break;
    case QAbstractSocket::RemoteHostClosedError:
        CONSOLE("ERREUR : le serveur a coupé la connexion.");
        break;
    default:
        CONSOLE("ERREUR : " + m_socket->errorString() + "");
    }

    CHAT("Impossible de se connecter.");

    ui->connecter->setEnabled(true);
}

void FenPrincipale::paquetRecu(Paquet *in)
{
    //On récupère l'OpCode
    quint16 opCode;
    *in >> opCode;

    //OpCode invalide
    if (opCode > NB_OPCODES)
    {
        CONSOLE("ERREUR: Le serveur a envoyé un opCode non géré(" + QString::number(opCode) + ").");

        //On vide le paquet.
        in->clear();
        delete in;

        return;
    }

    OpCodeHandler handler = OpCodeTable[opCode];

    CONSOLE("Paquet reçu: " + handler.nom + "(" + QString::number(opCode) + ")");

    //Lancement de la fonction associée.
    (this->*handler.f)(in, opCode);

    //Libération de la mémoire.
    delete in;
}

void FenPrincipale::handleClientSide(Paquet *in, quint16 opCode)
{
    CONSOLE("ERREUR: OpCode de client reçu.");
}

void FenPrincipale::handleHello(Paquet *in, quint16 opCode)
{

}

void FenPrincipale::handleAuth(Paquet *in, quint16 opCode)
{
    switch (opCode)
    {
    case SMSG_AUTH_INCORRECT_LOGIN:
        CHAT("ERREUR: Identifiant ou mot de passe de connexion incorrect.");
        break;
    case SMSG_AUTH_ACCT_ALREADY_IN_USE:
        CHAT("ERREUR: Ce compte est déjà connecté.");
        break;
    case SMSG_AUTH_ACCT_BANNED:
        CHAT("ERREUR: Ce compte a été banni.");
        break;
    case SMSG_AUTH_IP_BANNED:
        CHAT("ERREUR: Votre IP a été bannie");
        break;
    case SMSG_AUTH_ERROR:
        CHAT("ERREUR: Erreur d'authentifiation.");
        break;
    case SMSG_NICK_ALREADY_IN_USE:
        CHAT("ERREUR: Impossible de se nommer ainsi, le pseudo est déjà utilisé.");
        break;
    case SMSG_NICK_TOO_SHORT:
        CHAT("ERREUR: Pseudo trop court.");
        break;
    case SMSG_AUTH_OK:
        CHAT("Authentification réussie.");

        ui->chat->setEnabled(true);
        ui->message->setEnabled(true);
        ui->envoyer->setEnabled(true);

        *in >> m_pseudo;
        
        break;
    default:
        CONSOLE("ERREUR: Paquet non géré dans handleAuth");
        break;
    }
}

void FenPrincipale::handleKick(Paquet *in, quint16 opCode)
{
    CHAT("<em>Vous avez été kické par le serveur.</em>");
}

void FenPrincipale::handleChat(Paquet *in, quint16 opCode)
{
    switch (opCode)
    {
    case SMSG_INVALID_MESSAGE:
        CHAT("ERREUR: Le message envoyé est invalide");
        break;
    case SMSG_INVALID_NICK:
        CHAT("ERREUR: Impossible d'envoyer un message, votre pseudo est invalide ou indéfini.");
        break;
    case SMSG_CHAT_MESSAGE:
        {
            QString pseudo, message;
            *in >> pseudo;
            *in >> message;

            message = "<strong>&lt;" + pseudo + "&gt;</strong> " + message;

            CHAT(message);
        }
        break;
    default:
        CONSOLE("ERREUR: Paquet non géré dans handleChat");
        break;
    }
}

void FenPrincipale::handleUserModification(Paquet *in, quint16 opCode)
{
    switch (opCode)
    {
    case SMSG_USER_JOINED:
        {
            QString pseudo;
            *in >> pseudo;

            CHAT("<em>" + pseudo + " s'est joint au Chat.</em>");
            break;
        }
    case SMSG_USER_LEFT:
        {
            QString pseudo;
            *in >> pseudo;

            CHAT("<em>" + pseudo + " a quitté le Chat.</em>");
            break;
        }
    case SMSG_USER_RENAMED:
        {
            QString ancienPseudo, nouveauPseudo;
            *in >> ancienPseudo;
            *in >> nouveauPseudo;

            //Si l'ancien pseudo correspond à notre pseudo, on fait la mise à jour.
            if (m_pseudo == ancienPseudo)
                m_pseudo = ancienPseudo;

            CHAT("<em>" + ancienPseudo + " s'appelle maintenant " + nouveauPseudo + ".</em>");
            break;
        }
    default:
        CONSOLE("ERREUR: Paquet non géré dans handleUserModification");
        break;
    }
}
void FenPrincipale::handlePing(Paquet *in, quint16 opCode)
{
    quint32 time;
    *in >> time;

    Paquet out;
    out << CMSG_PONG;
    out << time;
    out.send(m_socket);
}

void FenPrincipale::handleRegister(Paquet *in, quint16 opCode)
{
    switch (opCode)
    {
    case SMSG_REG_OK:
        CHAT("L'enregistrement a réussi.");
        break;
    case SMSG_REG_ACCT_ALREADY_EXISTS:
        CHAT("ERREUR: Ce compte existe déjà.");
        break;
    case SMSG_REG_INVALID_NICK:
        CHAT("ERREUR: Nom de compte trop court ou invalide.");
        break;
    case SMSG_REG_ERROR:
        CHAT("ERREUR: Le serveur n'a pas pu vous enregistrer.");
        break;
    default:
        CONSOLE("ERREUR: Paquet non géré dans handleRegister");
        break;
    }
}

void FenPrincipale::handleChatCommands(QString &msg)
{
    //On quitte si le message n'est pas une commande.
    if (!msg.startsWith('/'))
        return;

    //Extraction des arguments
    QStringList args = msg.split(' ');

    //Commande en lowcase
    args[0] = args[0].toLower();

    if (args[0] == "/nick")
    {
        if (args.size() < 2)
        {
            CHAT("Vous devez définir un pseudo !");
            msg.clear();
            return;
        }

        //Si le pseudo comporte des espaces (plusieurs args) on l'assemble.
        QString pseudo;
        for (int i = 1; i < args.size(); i++)
            pseudo += args[i];

        Paquet out;
        out << CMSG_SET_NICK;
        out << pseudo;
        out.send(m_socket);
    }
    else if (args[0] == "/afk")
    {
        Paquet out;
        out << CMSG_SET_NICK;
        if (m_pseudo.endsWith("_AFK"))
        {
            m_pseudo.chop(4);
            out << m_pseudo;
        }
        else
        {
            m_pseudo += "_AFK";
            out << m_pseudo;
        }
        out.send(m_socket);
    }
    else if (args[0] == "/quit")
    {
        this->close();
    }
    else if (args[0] == "/register")
    {
        //Si on n'a pas assez d'arguments, on abandonne
        if (args.size() < 3)
        {
            CHAT("ERREUR: Syntaxe de la commande incorrecte.");
            msg.clear();
            return;
        }

        //Le mdp est toute la partie droite de la commande.
        QByteArray pw;
        for (int i = 2; i < args.size(); i++)
            pw += args[i] + " ";
        pw.chop(1); //Pour supprimer le dernier espace.

        //Vérification de la taille du mdp
        if (pw.size() < TAILLE_MDP_MIN)
        {
            CHAT("ERREUR: Mot de passe trop court");
            msg.clear();
            return;
        }

        //Enregistrement.
        Paquet out;
        out << CMSG_REGISTER;
        out << args[1]; //Login
        out << QCryptographicHash::hash(pw, QCryptographicHash::Sha1);  //Hash mdp
        out.send(m_socket);
    }
    else
    {
        CHAT("ERREUR: Commande chat invalide.");
    }
    msg.clear();
}
