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
    // On annonce sur la fen�tre qu'on est en train de se connecter
    CONSOLE("Tentative de connexion en cours...");
    ui->connecter->setEnabled(false);

    m_socket->abort(); // On d�sactive les connexions pr�c�dentes s'il y en a
    m_socket->connectToHost(ui->adresse->text(), ui->port->value()); // On se connecte au serveur demand�
}

// Envoi d'un message au serveur
void FenPrincipale::on_envoyer_clicked()
{
    QString msg = ui->message->text();
    handleChatCommands(msg);

    if (m_socket->isWritable() && !msg.isEmpty())
    {
        Paquet out;
        out << CMSG_CHAT_MESSAGE << msg;
        out >> m_socket;
    }

    ui->message->clear(); // On vide la zone d'�criture du message
    ui->message->setFocus(); // Et on remet le curseur � l'int�rieur
}

// Appuyer sur la touche Entr�e a le m�me effet que cliquer sur le bouton "Envoyer"
void FenPrincipale::on_message_returnPressed()
{
    on_envoyer_clicked();
}

// On a re�u un paquet (ou un sous-paquet)
void FenPrincipale::donneesRecues()
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

// Ce slot est appel� lorsque la connexion au serveur a r�ussi
void FenPrincipale::connecte()
{
    CONSOLE("Connexion r�ussie !");
    ui->connecter->setEnabled(true);

    QByteArray pwhash = QCryptographicHash::hash(ui->password->text().toUtf8(), QCryptographicHash::Sha1);

    //On demande au serveur de nous attribuer un pseudo.
    Paquet out;
    out << CMSG_AUTH_LOGIN << ui->login->text() << pwhash << ui->pseudo->text();
    out >> m_socket;

    //On s�lectionne la zone de message.
    ui->message->setFocus();
}

// Ce slot est appel� lorsqu'on est d�connect� du serveur
void FenPrincipale::deconnecte()
{
    CONSOLE("D�connect� du serveur");
    m_pseudo.clear();
    ui->chat->setEnabled(false);
    ui->message->setEnabled(false);
    ui->envoyer->setEnabled(false);
}

// Ce slot est appel� lorsqu'il y a une erreur
void FenPrincipale::erreurSocket(QAbstractSocket::SocketError erreur)
{
    switch(erreur) // On affiche un message diff�rent selon l'erreur qu'on nous indique
    {
    case QAbstractSocket::HostNotFoundError:
        CHAT("ERREUR : le serveur n'a pas pu �tre trouv�. V�rifiez l'IP et le port.");
        break;
    case QAbstractSocket::ConnectionRefusedError:
        CHAT("ERREUR : le serveur a refus� la connexion. V�rifiez si le programme \"serveur\" a bien �t� lanc�. V�rifiez aussi l'IP et le port.");
        break;
    case QAbstractSocket::RemoteHostClosedError:
        CHAT("ERREUR : le serveur a coup� la connexion.");
        break;
    default:
        CHAT("ERREUR : " + m_socket->errorString() + "");
    }

    ui->connecter->setEnabled(true);
}

void FenPrincipale::paquetRecu(Paquet *in)
{
    //On r�cup�re l'OpCode
    quint16 opCode;
    *in >> opCode;

    //OpCode invalide
    if (opCode > NB_OPCODES)
    {
        CONSOLE("ERREUR: Le serveur a envoy� un opCode non g�r�(" + QString::number(opCode) + ").");

        //On vide le paquet.
        in->clear();
        delete in;

        return;
    }

    OpCodeHandler handler = OpCodeTable[opCode];

    CONSOLE("Paquet re�u: " + handler.nom + "(" + QString::number(opCode) + ")");

    //Lancement de la fonction associ�e.
    (this->*handler.f)(in, opCode);

    //Lib�ration de la m�moire.
    delete in;
}

void FenPrincipale::handleClientSide(Paquet *in, quint16 opCode)
{
    CONSOLE("ERREUR: OpCode de client re�u.");
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
        CHAT("ERREUR: Ce compte est d�j� connect�.");
        break;
    case SMSG_AUTH_ACCT_BANNED:
        CHAT("ERREUR: Ce compte a �t� banni.");
        break;
    case SMSG_AUTH_IP_BANNED:
        CHAT("ERREUR: Votre IP a �t� bannie");
        break;
    case SMSG_AUTH_ERROR:
        CHAT("ERREUR: Erreur d'authentifiation.");
        break;
    case SMSG_AUTH_OK:
        CHAT("Authentification r�ussie.");

        ui->chat->setEnabled(true);
        ui->message->setEnabled(true);
        ui->envoyer->setEnabled(true);

        *in >> m_pseudo;
        
        break;
    default:
        CONSOLE("ERREUR: Paquet non g�r� dans handleAuth");
        break;
    }
}

void FenPrincipale::handleKick(Paquet *in, quint16 opCode)
{
    CHAT("<em>Vous avez �t� kick� par le serveur.</em>");
}

void FenPrincipale::handleChat(Paquet *in, quint16 opCode)
{
    switch (opCode)
    {
    case SMSG_CHAT_MESSAGE:
        {
            QString pseudo, message;
            *in >> pseudo >> message;

            message = "<strong>&lt;" + pseudo + "&gt;</strong> " + message;

            CHAT(message);
        }
        break;
    default:
        CONSOLE("ERREUR: Paquet non g�r� dans handleChat");
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
            QByteArray hash;
            quint8 level;
            *in >> pseudo >> hash >> level;

            CHAT("<em>" + pseudo + " (" + hash + ", " + QString::number(level) + ") s'est joint au Chat.</em>");
            break;
        }
    case SMSG_USER_LEFT:
        {
            QString pseudo;
            *in >> pseudo;

            CHAT("<em>" + pseudo + " a quitt� le Chat.</em>");
            break;
        }
    case SMSG_USER_RENAMED:
        {
            QString ancienPseudo, nouveauPseudo;
            *in >> ancienPseudo >> nouveauPseudo;

            //Si l'ancien pseudo correspond � notre pseudo, on fait la mise � jour.
            if (m_pseudo == ancienPseudo)
                m_pseudo = ancienPseudo;

            CHAT("<em>" + ancienPseudo + " s'appelle maintenant " + nouveauPseudo + ".</em>");
            break;
        }
    case SMSG_USER_KICKED:
        {
            QString pseudo, kickPar;
            *in >> kickPar >> pseudo;

            CHAT("<em> " + pseudo + " a �t� kick� par " + kickPar + ".</em>");
            break;
        }
    case SMSG_USER_BANNED:
        {
            QString pseudo, banPar;
            *in >> banPar >> pseudo;

            CHAT("<em> " + pseudo + " a �t� banni par " + banPar + ".</em>");
            break;
        }
    case SMSG_USER_VOICED:
        {
            QString pseudo, voicePar;
            *in >> pseudo >> voicePar;

            CHAT("<em> " + pseudo + " a �t� voic� par " + voicePar + ".</em>");
            break;
        }
    default:
        CONSOLE("ERREUR: Paquet non g�r� dans handleUserModification");
        break;
    }
}

void FenPrincipale::handlePing(Paquet *in, quint16 opCode)
{
    quint32 time;
    *in >> time;

    Paquet out;
    out << CMSG_PONG << time;
    out >> m_socket;
}

void FenPrincipale::handleRegister(Paquet *in, quint16 opCode)
{
    switch (opCode)
    {
    case SMSG_REG_OK:
        {
            CHAT("L'enregistrement a r�ussi.");
            QString login;
            *in >> login;
            ui->login->setText(login);
            ui->password->clear();
            break;
        }
    case SMSG_REG_ACCT_ALREADY_EXISTS:
        CHAT("ERREUR: Ce compte existe d�j�.");
        break;
    case SMSG_REG_INVALID_NICK:
        CHAT("ERREUR: Nom de compte trop court ou invalide.");
        break;
    case SMSG_REG_ERROR:
        CHAT("ERREUR: Le serveur n'a pas pu vous enregistrer.");
        break;
    default:
        CONSOLE("ERREUR: Paquet non g�r� dans handleRegister");
        break;
    }
}

void FenPrincipale::handleLevelMod(Paquet *in, quint16 opCode)
{
    switch (opCode)
    {
    case SMSG_PROMOTE_ERROR:
        CHAT("ERREUR: La promotion a �chou�.");
        break;
    case SMSG_PROMOTE_INVALID_LEVEL:
        CHAT("ERREUR: Ce niveau d'administration n'existe pas.");
        break;
    case SMSG_PROMOTE_ACCT_DOESNT_EXIST:
        CHAT("ERREUR: Promotion �chou�e, le compte n'existe pas.");
        break;
    case SMSG_PROMOTE_LEVEL_TOO_HIGH:
        CHAT("ERREUR: Promotion �chou�e, nous ne pouvez pas promouvoir un compte au-del� de votre niveau.");
        break;
    case SMSG_PROMOTE_NOT_YOURSELF:
        CHAT("ERREUR: Vous ne pouvez pas vous promouvoir vous-m�me.");
        break;
    case SMSG_PROMOTE_OK:
        {
            QString compte;
            *in >> compte;

            CHAT("<em>Le niveau du compte " + compte + " a �t� chang�.</em>");
            break;
        }
    case SMSG_PROMOTED:
        {
            QString pseudo;
            quint8 level;

            *in >> pseudo >> level;

            CHAT("<em>" + pseudo + " a modifi� votre niveau d'administration au niveau " + QString::number(level) + ".</em>");
            break;
        }
    default:
        CONSOLE("ERREUR: Paquet non g�r� dans handleLevelMod");
        break;
    }

}

void FenPrincipale::handleError(Paquet *in, quint16 opCode)
{
    switch (opCode)
    {
    case SMSG_NICK_ALREADY_IN_USE:
        CHAT("ERREUR: Impossible de se nommer ainsi, le pseudo est d�j� utilis�.");
        break;
    case SMSG_NICK_TOO_SHORT:
        CHAT("ERREUR: Pseudo trop court.");
        break;
    case SMSG_INVALID_MESSAGE:
        CHAT("ERREUR: Le message envoy� est invalide");
        break;
    case SMSG_INVALID_NICK:
        CHAT("ERREUR: Impossible d'envoyer un message, votre pseudo est invalide ou ind�fini.");
        break;
    case SMSG_NOT_AUTHORIZED:
        CHAT("ERREUR: Vous ne disposez pas des privil�ges suffisants.");
        break;
    case SMSG_USER_DOESNT_EXIST:
        CHAT("ERREUR: L'utilisateur sp�cifi� n'existe pas");
        break;
    case SMSG_NO_INTERACT_HIGHER_LEVEL:
        CHAT("ERREUR: Impossible d'interagir avec un compte de niveau sp�rieur ou �gal au v�tre.");
        break;
    default:
        CONSOLE("ERREUR: Paquet non g�r� dans handleError");
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
            CHAT("Vous devez d�finir un pseudo !");
            msg.clear();
            return;
        }

        //Si le pseudo comporte des espaces (plusieurs args) on l'assemble.
        QString pseudo;
        for (int i = 1; i < args.size(); i++)
            pseudo += args[i];

        Paquet out;
        out << CMSG_SET_NICK << pseudo;
        out >> m_socket;
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
        out >> m_socket;
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

        //V�rification de la taille du mdp
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
        out >> m_socket;
    }
    else if (args[0] == "/kick")
    {
        //Si on n'a pas assez d'arguments, on abandonne
        if (args.size() < 2)
        {
            CHAT("ERREUR: Syntaxe de la commande incorrecte.");
            msg.clear();
            return;
        }

        Paquet out;
        out << CMSG_KICK << args[1]; //Qui kicker
        out >> m_socket;
    }
    else if (args[0] == "/ban")
    {
        //Si on n'a pas assez d'arguments, on abandonne
        if (args.size() < 2)
        {
            CHAT("ERREUR: Syntaxe de la commande incorrecte.");
            msg.clear();
            return;
        }

        Paquet out;
        out << CMSG_BAN << args[1]; //Qui bannir
        out >> m_socket;
    }
    else if (args[0] == "/voice")
    {
        //Si on n'a pas assez d'arguments, on abandonne
        if (args.size() < 2)
        {
            CHAT("ERREUR: Syntaxe de la commande incorrecte.");
            msg.clear();
            return;
        }

        Paquet out;
        out << CMSG_VOICE << args[1]; //Qui voicer
        out >> m_socket;
    }
    else if (args[0] == "/setlevel")
    {
        //Si on n'a pas assez d'arguments, on abandonne
        if (args.size() < 3)
        {
            CHAT("ERREUR: Syntaxe de la commande incorrecte.");
            msg.clear();
            return;
        }

        Paquet out;
        out << CMSG_PROMOTE;
        out << args[1]; //Compte � promouvoir
        out << (quint8) args[2].toUInt(); //Level
        out >> m_socket;
    }
    else
    {
        CHAT("ERREUR: Commande chat invalide.");
    }
    msg.clear();
}
