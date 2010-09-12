#include "fenprincipale.h"
#include "ui_fenprincipale.h"

#define CONSOLE(a) ui->console->append(QTime::currentTime().toString() + " " + a)
#define CHAT(a)    ui->chat->appendHtml(a)

FenPrincipale::FenPrincipale(QWidget *parent) : QWidget(parent), ui(new Ui::FenPrincipale), m_taillePaquet(0)
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
}

FenPrincipale::~FenPrincipale()
{
    delete m_socket;
    delete ui;
}

void FenPrincipale::on_renommer_released()
{
    if (m_socket->isWritable())
    {
        Paquet out;
        out << CMSG_AUTH_SET_NAME;
        out << ui->pseudo->text();
        out.send(m_socket);
    }
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
    if (m_socket->isWritable())
    {
        Paquet out;
        out << CMSG_CHAT_MESSAGE;
        out << ui->message->text();
        out.send(m_socket);

        ui->message->clear(); // On vide la zone d'�criture du message
        ui->message->setFocus(); // Et on remet le curseur � l'int�rieur
    }
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

    //On demande au serveur de nous attribuer un pseudo.
    Paquet out;
    out << CMSG_AUTH_SET_NAME;
    out << ui->pseudo->text();

    out.send(m_socket);

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
        CONSOLE("ERREUR : le serveur n'a pas pu �tre trouv�. V�rifiez l'IP et le port.");
        break;
    case QAbstractSocket::ConnectionRefusedError:
        CONSOLE("ERREUR : le serveur a refus� la connexion. V�rifiez si le programme \"serveur\" a bien �t� lanc�. V�rifiez aussi l'IP et le port.");
        break;
    case QAbstractSocket::RemoteHostClosedError:
        CONSOLE("ERREUR : le serveur a coup� la connexion.");
        break;
    default:
        CONSOLE("ERREUR : " + m_socket->errorString() + "");
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

    CONSOLE("Paquet re�u: " + handler.nom);

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
    case SMSG_AUTH_NAME_ALREADY_IN_USE:
        CHAT("ERREUR: Impossible de se nommer ainsi, le pseudo est d�j� utilis�.");
        break;
    case SMSG_AUTH_IP_BANNED:
        CHAT("ERREUR: Votre IP a �t� bannie");
        break;
    case SMSG_AUTH_NAME_TOO_SHORT:
        CHAT("ERREUR: Pseudo trop court.");
        break;
    case SMSG_AUTH_OK:
        CHAT("Authentification r�ussie.");

        ui->chat->setEnabled(true);
        ui->message->setEnabled(true);
        ui->envoyer->setEnabled(true);
        
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
    case SMSG_INVALID_MESSAGE:
        CHAT("ERREUR: Le message envoy� est invalide");
        break;
    case SMSG_NAME_NOT_SET:
        CHAT("ERREUR: Impossible d'envoyer un message, vous n'avez pas de pseudo d�fini");
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
        CONSOLE("ERREUR: Paquet non g�r� dans handleChat");
        break;
    }
}

