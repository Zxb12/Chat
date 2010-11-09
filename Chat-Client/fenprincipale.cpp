#include "fenprincipale.h"
#include "ui_fenprincipale.h"

#define CONSOLE(a) ui->console->append(QTime::currentTime().toString() + " " + a)
#define ERREUR     "<strong style=\"color:#ff0000\">Erreur: </strong>"
#define SUCCES     "<strong style=\"color:#33ff66\">Succ�s: </strong>"

FenPrincipale::FenPrincipale(QMainWindow *parent) : QMainWindow(parent), ui(new Ui::FenPrincipale), m_taillePaquet(0), m_acctName(""),
m_quitOnDisconnect(false), m_html("")
{
    ui->setupUi(this);

    //Pr�paration de l'UI
    this->setWindowTitle("OokChat - " + VERSION);
    ui->statusbar->addWidget(new QLabel("Pr�t"));
    chargeConfig();

    connect(ui->connecter, SIGNAL(clicked()),                               this, SLOT(seConnecter()));
    connect(ui->pseudo, SIGNAL(returnPressed()),                            this, SLOT(seConnecter()));
    connect(ui->password, SIGNAL(returnPressed()),                          this, SLOT(seConnecter()));
    connect(ui->message, SIGNAL(returnPressed()),                           this, SLOT(envoyerMessage()));
    connect(ui->envoyer, SIGNAL(clicked()),                                 this, SLOT(envoyerMessage()));
    connect(ui->listeChannels, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(changerChannel(QListWidgetItem*)));
    connect(ui->listeChannels, SIGNAL(itemEntered(QListWidgetItem*)),       this, SLOT(changerChannel(QListWidgetItem*)));

    //Menu
    connect(ui->actionConnecter, SIGNAL(triggered()),                       this, SLOT(seConnecter()));
    connect(ui->actionDeconnexion, SIGNAL(triggered()),                     this, SLOT(seDeconnecter()));
    connect(ui->actionKick, SIGNAL(triggered()),                            this, SLOT(ui_kick()));
    connect(ui->actionBan, SIGNAL(triggered()),                             this, SLOT(ui_ban()));
    connect(ui->actionEnregistrement, SIGNAL(triggered()),                  this, SLOT(ui_register()));
    connect(ui->actionModificationDeNiveau, SIGNAL(triggered()),            this, SLOT(ui_modLevel()));
    connect(ui->actionMessageDeDeconnexion, SIGNAL(triggered()),            this, SLOT(ui_logoutMessage()));
    connect(ui->actionRenommer, SIGNAL(triggered()),                        this, SLOT(ui_renommer()));
    connect(ui->actionQuitter, SIGNAL(triggered()),                         qApp, SLOT(quit()));

    QMenu *menu = new QMenu("Chat", this);
    menu->addAction("Pas d'actions d�finies !");

    m_sysTray = new QSystemTrayIcon(QIcon(":/icones/32"), this);
    connect(m_sysTray, SIGNAL(messageClicked()),                            this, SLOT(premierPlan()));
    m_sysTray->setContextMenu(menu);
    m_sysTray->show();

    m_socket = new QTcpSocket(this);
    connect(m_socket, SIGNAL(readyRead()),                                  this, SLOT(donneesRecues()));
    connect(m_socket, SIGNAL(connected()),                                  this, SLOT(connecte()));
    connect(m_socket, SIGNAL(disconnected()),                               this, SLOT(deconnecte()));
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)),          this, SLOT(erreurSocket(QAbstractSocket::SocketError)));

    ui->chat->setEnabled(false);
    ui->message->setEnabled(false);
    ui->envoyer->setEnabled(false);
    ui->actionDeconnexion->setEnabled(false);
    ui->actionKick->setEnabled(false);
    ui->actionBan->setEnabled(false);
    ui->actionEnregistrement->setEnabled(false);
    ui->actionModificationDeNiveau->setEnabled(false);
    ui->listeConnectes->setEnabled(false);
    ui->listeChannels->setEnabled(false);

    ui->chat->setHtml(m_html);
}

FenPrincipale::~FenPrincipale()
{
    delete m_socket;
    delete m_sysTray;
    delete ui;
}

void FenPrincipale::seConnecter()
{
    seDeconnecter(); // On d�sactive les connexions pr�c�dentes s'il y en a

    //On vide la fen�tre de chat
    ui->chat->clear();
    m_html.clear();

    //On tente de se connecter
    CONSOLE("Tentative de connexion en cours...");
    ui->connecter->setEnabled(false);
    ui->actionConnecter->setEnabled(false);
    m_socket->connectToHost(ui->adresse->text(), ui->port->value()); // On se connecte au serveur demand�
}

void FenPrincipale::seDeconnecter()
{
    m_socket->disconnectFromHost();
}

// Envoi d'un message au serveur
void FenPrincipale::envoyerMessage()
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

void FenPrincipale::premierPlan()
{
    this->raise();
    this->activateWindow();
    ui->message->setFocus();
}

void FenPrincipale::appendChat(QString str1, QString str2 = "")
{
    /* str1: header
       str2: texte
       si str2 est vide, str1 est le texte. */

    if (str2.isEmpty())
    {
        str2 = str1;
        str1.clear();
    }

    //Pr�paration du HTML
    m_html += "<tr><td align=\"right\">" + str1 + "</td><td>" + str2 + "</td></tr>";

    //On efface le HTML et on le remet.
    ui->chat->clear();
    ui->chat->append(m_html); //Append fait scroll en bas

}

void FenPrincipale::afficheBulle(QString titre, QString msg, QSystemTrayIcon::MessageIcon icone = QSystemTrayIcon::Information, int duree = 10000)
{
    //On fait flasher l'application
    QApplication::alert(this);

    //Affichage d'une infobulle si la fen�tre n'a pas le focus.
    if (!QApplication::focusWidget() && m_sysTray->supportsMessages())
        m_sysTray->showMessage("OokChat - " + titre, msg, icone, duree);
}

void FenPrincipale::changerChannel(QListWidgetItem *item)
{
    //Recherche du channel
    Channel channel = m_channels[ui->listeChannels->row(item)];
    quint32 id = channel.id;
    QString password = "";
    if (channel.protege)
    {
        password = QInputDialog::getText(this, "OokChat", "Mot de passe du canal", QLineEdit::Password);
        if (password.isEmpty())
            return;
    }

    Paquet out;
    out << CMSG_CHANNEL_JOIN << id << password;
    out >> m_socket;
}

void FenPrincipale::closeEvent(QCloseEvent */*event*/)
{
    //Ouverture du fichier.
    QFile file("chat.conf");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QMessageBox::warning(this, "Erreur de sauvegarde", "Impossible d'ouvrir le fichier de configuration.\n"
                                                           "Les param�res ne peuvent pas �tre sauvegard�s.");
        return;
    }

    //Pr�paration du stream
    QDataStream out(&file);

    out << VERSION_CONFIG << ui->adresse->text() << quint16(ui->port->value()) << ui->pseudo->text() << ui->login->text() << m_logoutMessage;

    file.close();
}

void FenPrincipale::chargeConfig()
{
    //Ouverture du fichier.
    QFile file("chat.conf");
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "Erreur de chargement", "Impossible d'ouvrir le fichier de configuration.\n"
                                                           "Les param�res ne peuvent pas �tre charg�s.");
        return;
    }

    //Pr�paration du stream
    QDataStream in(&file);
    QString adresse, pseudo, login;
    quint16 port;
    quint32 versionFichier;

    in >> versionFichier >> adresse >> port >> pseudo >> login >> m_logoutMessage;

    //V�rification de la version du fichier.
    if (versionFichier != VERSION_CONFIG)
    {
        QMessageBox::warning(this, "Erreur de chargement", "Le fichier de configuration est � la mauvaise version.\n"
                                                           "Les param�tes ne peuvent pas �tre charg�s.");
        return;
    }

    //Attribution des valeurs
    ui->adresse->setText(adresse);
    ui->port->setValue(port);
    ui->pseudo->setText(pseudo);
    ui->login->setText(login);

    ui->password->setFocus();
    if (ui->login->text().isEmpty())
        ui->login->setFocus();
    if (ui->pseudo->text().isEmpty())
        ui->pseudo->setFocus();
    if (ui->port->value() == 0)
        ui->port->setFocus();
    if (ui->adresse->text().isEmpty())
        ui->adresse->setFocus();

}


// Ce slot est appel� lorsque la connexion au serveur a r�ussi
void FenPrincipale::connecte()
{
    CONSOLE("Connexion r�ussie !");
    ui->connecter->setEnabled(true);
    ui->actionConnecter->setEnabled(true);

    //On envoie le Hello
    Paquet out;
    out << CMSG_HELLO;
    out << VERSION;
    out >> m_socket;
}

// Ce slot est appel� lorsqu'on est d�connect� du serveur
void FenPrincipale::deconnecte()
{
    CONSOLE("D�connect� du serveur");
    appendChat("D�connect� du serveur.");
    m_pseudo.clear();
    ui->chat->setEnabled(false);
    ui->message->setEnabled(false);
    ui->envoyer->setEnabled(false);
    ui->actionDeconnexion->setEnabled(false);
    ui->actionKick->setEnabled(false);
    ui->actionBan->setEnabled(false);
    ui->actionEnregistrement->setEnabled(false);
    ui->actionModificationDeNiveau->setEnabled(false);
    ui->listeConnectes->setEnabled(false);
    ui->listeConnectes->clear();
    ui->listeChannels->setEnabled(false);
    ui->listeChannels->clear();
    if (m_quitOnDisconnect)
        qApp->quit();

    afficheBulle("D�connexion", "Vous avez �t� d�connect� du serveur.");
}

// Ce slot est appel� lorsqu'il y a une erreur
void FenPrincipale::erreurSocket(QAbstractSocket::SocketError erreur)
{
    switch(erreur) // On affiche un message diff�rent selon l'erreur qu'on nous indique
    {
    case QAbstractSocket::HostNotFoundError:
        appendChat(ERREUR, "Le serveur n'a pas pu �tre trouv�. V�rifiez l'IP et le port.");
        break;
    case QAbstractSocket::ConnectionRefusedError:
        appendChat(ERREUR, "Le serveur a refus� la connexion. V�rifiez l'adresse, le port et le statut du serveur.");
        break;
    case QAbstractSocket::RemoteHostClosedError:
        break;
    default:
        appendChat(ERREUR, m_socket->errorString());
    }

    ui->connecter->setEnabled(true);
    ui->actionConnecter->setEnabled(true);
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

void FenPrincipale::handleClientSide(Paquet */*in*/, quint16 /*opCode*/)
{
    CONSOLE("ERREUR: OpCode de client re�u.");
}

void FenPrincipale::handleHello(Paquet */*in*/, quint16 /*opCode*/)
{
    QByteArray pwhash = QCryptographicHash::hash(ui->password->text().toUtf8(), QCryptographicHash::Sha1);

    //Authentification
    Paquet out;
    out << CMSG_AUTH_LOGIN << ui->login->text() << pwhash << ui->pseudo->text();
    out >> m_socket;
}

void FenPrincipale::handleAuth(Paquet *in, quint16 opCode)
{
    switch (opCode)
    {
    case SMSG_AUTH_INCORRECT_LOGIN:
        appendChat(ERREUR, "Identifiant ou mot de passe de connexion incorrect.");
        break;
    case SMSG_AUTH_ACCT_ALREADY_IN_USE:
        appendChat(ERREUR, "Ce compte est d�j� connect�.");
        break;
    case SMSG_AUTH_ACCT_BANNED:
        {
            QDateTime finBan = QDateTime::currentDateTime();
            quint32 duree;
            QString raison;
            *in >> duree >> raison;
            if (duree)  //Cas d'un ban � dur�e d�termin�e.
            {
                finBan.addSecs(duree);
                appendChat(ERREUR, "Ce compte a �t� banni. Fin du ban le: " + finBan.toString());
            }
            else        //Cas d'un ban d�finitif.
            {
                appendChat(ERREUR, "Ce compte a �t� banni d�finitivement.");
            }
            appendChat("Raison: " + raison);
            break;
        }
    case SMSG_AUTH_IP_BANNED:
        {
            QDateTime finBan = QDateTime::currentDateTime();
            quint32 duree;
            QString raison;
            *in >> duree >> raison;
            if (duree)  //Cas d'un ban � dur�e d�termin�e.
            {
                finBan = finBan.addSecs(duree);
                appendChat(ERREUR, "Votre IP a �t� bannie. Fin du ban le: " + finBan.toString());
            }
            else        //Cas d'un ban d�finitif.
            {
                appendChat(ERREUR, "Votre IP a �t� bannie d�finitivement.");
            }
            appendChat("Raison: " + raison);
            break;
        }
    case SMSG_AUTH_INCORRECT_VERSION:
        {
            QString version;
            *in >> version;
            appendChat("Votre version (" + QString(VERSION) + ") est incompatible avec celle du serveur (" + version + ").");
            break;
        }
    case SMSG_AUTH_ERROR:
        appendChat(ERREUR, "Erreur d'authentifiation.");
        break;
    case SMSG_AUTH_OK:
        {
            appendChat("Authentification r�ussie.");

            //On active le chat
            ui->chat->setEnabled(true);
            ui->message->setEnabled(true);
            ui->envoyer->setEnabled(true);
            ui->actionDeconnexion->setEnabled(true);
            ui->actionKick->setEnabled(true);
            ui->actionBan->setEnabled(true);
            ui->actionEnregistrement->setEnabled(true);
            ui->actionModificationDeNiveau->setEnabled(true);
            ui->listeConnectes->setEnabled(true);
            ui->listeChannels->setEnabled(true);

            //On s�lectionne la zone de message.
            ui->message->setFocus();

            *in >> m_pseudo;

            Paquet out;
            out << CMSG_UPDATE_CLIENTS_LIST;
            out >> m_socket;

            out.clear();
            out << CMSG_UPDATE_CHANNEL;
            out >> m_socket;

            out.clear();
            out << CMSG_SET_LOGOUT_MSG;
            out << m_logoutMessage;
            out >> m_socket;

            break;
        }
    default:
        CONSOLE("ERREUR: Paquet non g�r� dans handleAuth");
        break;
    }
}

void FenPrincipale::handleKick(Paquet */*in*/, quint16 /*opCode*/)
{
    appendChat("<em>Vous avez �t� kick� par le serveur.</em>");
}

void FenPrincipale::handleChat(Paquet *in, quint16 opCode)
{
    switch (opCode)
    {
    case SMSG_CHAT_MESSAGE:
        {
            QString pseudo, message;
            *in >> pseudo >> message;

            //Affichage de l'infobulle
            afficheBulle("Nouveau message de " + pseudo, message);

            pseudo = "<strong>&lt;" + Qt::escape(pseudo) + "&gt;</strong> ";

            appendChat(pseudo, Qt::escape(message));

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

            //Mise � jour de la liste des connect�s
            ui->listeConnectes->addItem(pseudo);

            appendChat("-->", "<em>" + Qt::escape(pseudo) + " (" + hash + ", " + QString::number(level) + ") s'est joint au Chat.</em>");
            afficheBulle("Connexion", pseudo + " s'est joint au Chat.");
            break;
        }
    case SMSG_USER_LEFT:
        {
            QString pseudo, raison;
            QByteArray hash;
            quint8 level;
            *in >> pseudo >> raison >> hash >> level;

            //Recherche le pseudo, prend sa ligne et le supprime.
            delete ui->listeConnectes->takeItem(
                    ui->listeConnectes->row(
                            ui->listeConnectes->findItems(pseudo, Qt::MatchExactly).first()));

            appendChat("&lt;--", "<em>" + Qt::escape(pseudo) + " (" + hash + ", " + QString::number(level) + ") a quitt� le Chat : " + Qt::escape(raison) + "</em>");
            afficheBulle("D�connexion", pseudo + " a quitt� le Chat.");
            break;
        }
    case SMSG_USER_RENAMED:
        {
            QString ancienPseudo, nouveauPseudo;
            *in >> ancienPseudo >> nouveauPseudo;

            //Si l'ancien pseudo correspond � notre pseudo, on fait la mise � jour.
            if (m_pseudo == ancienPseudo)
            {
                m_pseudo = nouveauPseudo;
                ui->pseudo->setText(nouveauPseudo);
            }

            //Mise � jour de la liste de connect�s
            ui->listeConnectes->findItems(ancienPseudo, Qt::MatchExactly).first()->setText(nouveauPseudo);

            appendChat("<em>" + Qt::escape(ancienPseudo) + " s'appelle maintenant " + Qt::escape(nouveauPseudo) + ".</em>");
            afficheBulle("Utilisateur renomm�", ancienPseudo + " s'appelle maintenant " + nouveauPseudo + ".");
            break;
        }
    case SMSG_USER_KICKED:
        {
            QString pseudo, kickPar, raison;
            *in >> kickPar >> pseudo >> raison;

            appendChat("&lt;--", "<em> " + Qt::escape(pseudo) + " a �t� kick� par " + Qt::escape(kickPar) + ". Raison: " + Qt::escape(raison) + "</em>");
            break;
        }
    case SMSG_USER_BANNED:
        {
            QString pseudo, banPar, raison;
            *in >> banPar >> pseudo >> raison;

            appendChat("&lt;--", "<em> " + Qt::escape(pseudo) + " a �t� banni par " + Qt::escape(banPar) + ". Raison: " + Qt::escape(raison) + "</em>");
            break;
        }
    case SMSG_USER_VOICED:
        {
            QString pseudo, voicePar;
            *in >> pseudo >> voicePar;

            appendChat("<em> " + Qt::escape(pseudo) + " a �t� voic� par " + Qt::escape(voicePar) + ".</em>");
            break;
        }
    case SMSG_CHANNEL_JOIN:
        {
            QString pseudo, channel;
            QByteArray hash;
            quint8 level;
            *in >> pseudo >> hash >> level >> channel;

            //Mise � jour de la liste des connect�s
            ui->listeConnectes->addItem(pseudo);

            appendChat("-->", "<em> " + Qt::escape(pseudo) + " (" + hash + ", " + QString::number(level) + ") a rejoint le canal " + channel + ".</em>");
            break;
        }
    case SMSG_CHANNEL_LEAVE:
        {
            QString pseudo, channel;
            QByteArray hash;
            quint8 level;
            *in >> pseudo >> hash >> level >> channel;

            //Recherche le pseudo, prend sa ligne et le supprime.
            delete ui->listeConnectes->takeItem(
                    ui->listeConnectes->row(
                            ui->listeConnectes->findItems(pseudo, Qt::MatchExactly).first()));

            appendChat("&lt;--", "<em> " + Qt::escape(pseudo) + " (" + hash + ", " + QString::number(level) + ") a quitt� le canal " + channel + ".</em>");
            break;
        }
    default:
        CONSOLE("ERREUR: Paquet non g�r� dans handleUserModification");
        break;
    }
}

void FenPrincipale::handlePing(Paquet *in, quint16 /*opCode*/)
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
            appendChat(SUCCES, "L'enregistrement a r�ussi.");
            QString login;
            *in >> login;
            ui->login->setText(login);
            ui->password->clear();
            break;
        }
    case SMSG_REG_ACCT_ALREADY_EXISTS:
        appendChat(ERREUR, "Ce compte existe d�j�.");
        break;
    case SMSG_REG_INVALID_NICK:
        appendChat(ERREUR, "Nom de compte trop court ou invalide.");
        break;
    case SMSG_REG_ERROR:
        appendChat(ERREUR, "Le serveur n'a pas pu vous enregistrer.");
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
        appendChat(ERREUR, "La modification du niveau a �chou�.");
        break;
    case SMSG_LVL_MOD_INVALID_LEVEL:
        appendChat(ERREUR, "Ce niveau d'administration n'existe pas.");
        break;
    case SMSG_LVL_MOD_ACCT_DOESNT_EXIST:
        appendChat(ERREUR, "Modification �chou�e, le compte n'existe pas.");
        break;
    case SMSG_LVL_MOD_LEVEL_TOO_HIGH:
        appendChat(ERREUR, "Modification �chou�e, nous ne pouvez pas modifier le niveau d'un compte au-del� de votre niveau.");
        break;
    case SMSG_LVL_MOD_NOT_YOURSELF:
        appendChat(ERREUR, "Vous ne pouvez pas modifier votre niveau..");
        break;
    case SMSG_LVL_MOD_OK:
        {
            QString compte;
            *in >> compte;

            appendChat(SUCCES, "<em>Le niveau du compte " + compte + " a �t� chang�.</em>");
            break;
        }
    case SMSG_LVL_CHANGED:
        {
            QString pseudo;
            quint8 level;

            *in >> pseudo >> level;

            appendChat("<em>" + pseudo + " a modifi� votre niveau d'administration au niveau " + QString::number(level) + ".</em>");
            break;
        }
    default:
        CONSOLE("ERREUR: Paquet non g�r� dans handleLevelMod");
        break;
    }

}

void FenPrincipale::handleError(Paquet */*in*/, quint16 opCode)
{
    switch (opCode)
    {
    case SMSG_NICK_ALREADY_IN_USE:
        appendChat(ERREUR, "Impossible de se nommer ainsi, le pseudo est d�j� utilis�.");
        break;
    case SMSG_NICK_TOO_SHORT:
        appendChat(ERREUR, "Pseudo trop court.");
        break;
    case SMSG_NICK_TOO_LONG:
        appendChat(ERREUR, "Pseudo trop long.");
        break;
    case SMSG_INVALID_MESSAGE:
        appendChat(ERREUR, "Le message envoy� est invalide");
        break;
    case SMSG_INVALID_NICK:
        appendChat(ERREUR, "Impossible d'envoyer un message, votre pseudo est invalide ou ind�fini.");
        break;
    case SMSG_NOT_AUTHORIZED:
        appendChat(ERREUR, "Vous ne disposez pas des privil�ges suffisants.");
        break;
    case SMSG_USER_DOESNT_EXIST:
        appendChat(ERREUR, "L'utilisateur sp�cifi� n'existe pas");
        break;
    case SMSG_NO_INTERACT_HIGHER_LEVEL:
        appendChat(ERREUR, "Impossible d'interagir avec un compte de niveau sp�rieur ou �gal au v�tre.");
        break;
    case SMSG_CHANNEL_WRONG_PASSWORD:
        appendChat(ERREUR, "Mot de passe du canal incorrect.");
        break;
    case SMSG_CHANNEL_LVL_TOO_LOW:
        appendChat(ERREUR, "Votre niveau est trop bas pour rejoindre ce canal.");
        break;
    default:
        CONSOLE("ERREUR: Paquet non g�r� dans handleError");
        break;
    }

}

void FenPrincipale::handleWhoIs(Paquet *in, quint16 /*opCode*/)
{
    QString pseudo, compte;
    quint8 niveau;
    quint16 ping;
    QByteArray hashIP;

    *in >> pseudo >> compte >> niveau >> ping >> hashIP;

    appendChat(SUCCES, "Whois: " + Qt::escape(pseudo));
    appendChat("Compte: " + Qt::escape(compte));
    appendChat("Niveau de compte: " + QString::number(niveau));
    appendChat("Ping: " + QString::number(ping) + "ms");
    appendChat("Hash de l'IP: " + hashIP);
}

void FenPrincipale::handleClientsList(Paquet *in, quint16 /*opCode*/)
{
    ui->listeConnectes->clear();

    quint32 size;
    QStringList pseudos;
    QString tempPseudo;
    *in >> size;
    for (quint32 i = 0; i < size; i++)
    {
        *in >> tempPseudo;
        pseudos << tempPseudo;
    }

    ui->listeConnectes->addItems(pseudos);
}

void FenPrincipale::handleChannel(Paquet *in, quint16 /*opCode*/)
{
    ui->listeChannels->clear();
    m_channels.clear();

    quint32 size;
    Channel channel;
    *in >> size;
    for (quint32 i = 0; i < size; i++)
    {
        *in >> channel.id;
        *in >> channel.nom;
        *in >> channel.protege;
        ui->listeChannels->addItem(channel.nom);
        m_channels.append(channel);
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
            appendChat(ERREUR, "Vous devez d�finir un pseudo !");
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
    else if (args[0] == "/register")
    {
        //Si on n'a pas assez d'arguments, on abandonne
        if (args.size() < 3)
        {
            appendChat(ERREUR, "Syntaxe de la commande incorrecte.");
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
            appendChat(ERREUR, "Mot de passe trop court");
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
            appendChat(ERREUR, "Syntaxe de la commande incorrecte.");
            msg.clear();
            return;
        }

        //On essaie d'extraire la raison de ban.
        QString raison = msg.section('\"', 1, 1);

        Paquet out;
        out << CMSG_KICK << args[1] << raison;
        out >> m_socket;
    }
    else if (args[0] == "/ban")
    {
        //Si on n'a pas assez d'arguments, on abandonne
        if (args.size() < 2)
        {
            appendChat(ERREUR, "Syntaxe de la commande incorrecte.");
            msg.clear();
            return;
        }

        quint32 duree = 0;  //0 = ban infini.
        QDateTime finBan = QDateTime::currentDateTime();

        //On essaie d'extraire le temps du ban.
        if (args.size() >= 4)
        {
            bool ok = false;
            duree = args[2].toUInt(&ok);
            if (ok)
            {
                //On ajoute la dur�e correspondante � la dur�e de ban.
                if      (args[3].toLower() == "min" || args[3].toLower() == "minute" || args[3].toLower() == "minutes")
                    finBan = finBan.addSecs(duree * 60);
                else if (args[3].toLower() == "h" || args[3].toLower() == "hour" || args[3].toLower() == "hours")
                    finBan = finBan.addSecs(duree * 60 * 60);
                else if (args[3].toLower() == "d" || args[3].toLower() == "day" || args[3].toLower() == "days")
                    finBan = finBan.addDays(duree);
                else if (args[3].toLower() == "mon" || args[3].toLower() == "month" || args[3].toLower() == "months")
                    finBan = finBan.addMonths(duree);
                else if (args[3].toLower() == "y" || args[3].toLower() == "year" || args[3].toLower() == "years")
                    finBan = finBan.addYears(duree);
                duree = QDateTime::currentDateTime().secsTo(finBan);
            }
        }

        //On essaie d'extraire la raison de ban.
        QString raison = msg.section('\"', 1, 1);

        Paquet out;
        out << CMSG_BAN << args[1] << duree << raison;
        out >> m_socket;
    }
    else if (args[0] == "/voice")
    {
        //Si on n'a pas assez d'arguments, on abandonne
        if (args.size() < 2)
        {
            appendChat(ERREUR, "Syntaxe de la commande incorrecte.");
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
            appendChat(ERREUR, "Syntaxe de la commande incorrecte.");
            msg.clear();
            return;
        }

        Paquet out;
        out << CMSG_LVL_MOD;
        out << args[1]; //Compte � promouvoir
        out << (quint8) args[2].toUInt(); //Level
        out >> m_socket;
    }
    else if (args[0] == "/who" || args[0] == "/whois")
    {
        //Si on n'a pas assez d'arguments, on abandonne
        if (args.size() < 2)
        {
            appendChat(ERREUR, "Syntaxe de la commande incorrecte.");
            msg.clear();
            return;
        }

        Paquet out;
        out << CMSG_WHOIS << args[1];
        out >> m_socket;
    }
    else if (args[0] == "/join")
    {
        if (args.size() < 2)
        {
            appendChat(ERREUR, "Syntaxe de la commande incorrecte");
            msg.clear();
            return;
        }

        //Extraction du nom du canal
        QString channel, pass;
        for (int i = 1; i < args.size(); i++)
            channel += args[i] + " ";
        channel = channel.trimmed();

        //Recherche de l'ID
        quint32 id = 0;
        foreach (Channel i_channel, m_channels)
        {
            if (i_channel.nom.compare(channel, Qt::CaseInsensitive) == 0)
            {
                id = i_channel.id;
                if (i_channel.protege)
                    pass = QInputDialog::getText(this, "OokChat", "Mot de passe du canal");
                break;
            }
        }

        if (!id)
        {
            appendChat(ERREUR, "Canal non trouv�");
            msg.clear();
            return;
        }

        Paquet out;
        out << CMSG_CHANNEL_JOIN << id << pass;
        out >> m_socket;
    }
    else if (args[0] == "/quit")
    {
        QString quitMessage;
        for (int i = 1; i < args.size(); i++)
            quitMessage += (args[i] + ' ');

        //On se pr�pare � quitter.
        m_quitOnDisconnect = true;

        if (!quitMessage.simplified().isEmpty())
            m_logoutMessage = quitMessage;

        Paquet out;
        out << CMSG_LOGOUT << quitMessage;
        out >> m_socket;
    }
    else if (args[0] == "/logout" || args[0] == "/deco")
    {
        QString logoutMessage;
        for (int i = 1; i < args.size(); i++)
            logoutMessage += (args[i] + ' ');

        if (!logoutMessage.simplified().isEmpty())
            m_logoutMessage = logoutMessage;

        //On se pr�pare � quitter.
        m_quitOnDisconnect = false;

        Paquet out;
        out << CMSG_LOGOUT << m_logoutMessage;
        out >> m_socket;
    }
    else
    {
        appendChat(ERREUR, "Commande chat invalide.");
    }
    msg.clear();
}

void FenPrincipale::ui_kick()
{
    QString quiKicker, raison;
    quiKicker = QInputDialog::getText(this, "OokChat", "Qui voulez-vous kicker ?").trimmed();

    //Si on n'a personne � kicker, on quitte
    if (quiKicker.isEmpty())
        return;

    raison = QInputDialog::getText(this, "OokChat", "Pour quelle raison ?").trimmed();

    Paquet out;
    out << CMSG_KICK << quiKicker << raison;
    out >> m_socket;
}

void FenPrincipale::ui_ban()
{
    QString quiBannir, raison;
    quint32 duree = 0;
    bool ok = false;
    FenBan f(this, &quiBannir, &duree, &raison, &ok);
    f.exec();

    if (ok)
    {
        Paquet out;
        out << CMSG_BAN << quiBannir << duree << raison;
        out >> m_socket;
    }
}

void FenPrincipale::ui_register()
{
    QString login, pass, passConfirme;
    login = QInputDialog::getText(this, "OokChat", "Entrez le nom de compte").trimmed();
    //Si on n'a pas de compte, on quitte
    if (login.isEmpty())
        return;

    pass = QInputDialog::getText(this, "OokChat", "Entrez votre mot de passe", QLineEdit::Password).trimmed();
    if (pass.isEmpty())
        return;
    if (pass.size() < TAILLE_MDP_MIN)
    {
        appendChat(ERREUR, "Le mot de passe sp�cifi� est trop court.");
        return;
    }

    passConfirme = QInputDialog::getText(this, "OokChat", "Confirmez votre mot de passe", QLineEdit::Password).trimmed();
    if (passConfirme.isEmpty())
        return;
    if (pass != passConfirme)   //V�rifie que les mots de passe correspondent
    {
        appendChat(ERREUR, "Les mots de passe ne correspondent pas.");
        return;
    }

    //On peut s'enregistrer
    QByteArray hash;
    hash = QCryptographicHash::hash(pass.toUtf8(), QCryptographicHash::Sha1);

    Paquet out;
    out << CMSG_REGISTER << login << hash;
    out >> m_socket;
}

void FenPrincipale::ui_modLevel()
{
    QString compte;
    quint8 level;
    bool ok = false;

    compte = QInputDialog::getText(this, "OokChat", "Quel compte modifier ?").trimmed();
    if (compte.isEmpty())   //V�rification des donn�es
        return;

    level = QInputDialog::getInt(this, "OokChat", "Mettre ce compte � quel niveau ?", 1, 1, 255, 1, &ok);
    if (!ok)
        return;

    Paquet out;
    out << CMSG_LVL_MOD << compte << level;
    out >> m_socket;
}

void FenPrincipale::ui_logoutMessage()
{
    QString message;

    message = QInputDialog::getText(this, "OokChat", "Entrez votre message de d�connexion").trimmed();
    if (message.isEmpty())
        return;

    m_logoutMessage = message;
    if (m_socket->isWritable())
    {
        Paquet out;
        out << CMSG_SET_LOGOUT_MSG << message;
        out >> m_socket;
    }
}

void FenPrincipale::ui_renommer()
{
    QString pseudo;

    pseudo = QInputDialog::getText(this, "OokChat", "Entrez votre nouveau pseudo").trimmed();
    if (pseudo.isEmpty())
        return;

    Paquet out;
    out << CMSG_SET_NICK << pseudo;
    out >> m_socket;
}
