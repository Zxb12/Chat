#ifndef FENPRINCIPALE_H
#define FENPRINCIPALE_H

#define TAILLE_MDP_MIN  5
#define VERSION         QString("Chat-0.0.6b")
#define VERSION_CONFIG  quint32(2010101102)  //YYYYMMDD + Numéro de la version du jour

#include <QMainWindow>
#include <QTcpSocket>
#include <QTime>
#include <QCryptographicHash>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QFile>
#include <QMessageBox>
#include <QInputDialog>
#include <QListWidgetItem>

#include "../shared/paquet.h"
#include "opcode.h"
#include "fenban.h"

struct Channel
{
    QString nom;
    quint32 id;
    bool protege;
};

namespace Ui
{
    class FenPrincipale;
}

class FenPrincipale : public QMainWindow
{
    Q_OBJECT
public:
    FenPrincipale(QMainWindow *parent = 0);
    ~FenPrincipale();

public slots:
    void donneesRecues();
    void erreurSocket(QAbstractSocket::SocketError erreur);
    void seConnecter();
    void seDeconnecter();
    void connecte();
    void deconnecte();
    void envoyerMessage();
    void premierPlan();
    void changerChannel(QListWidgetItem*);

    void ui_kick();
    void ui_ban();
    void ui_register();
    void ui_modLevel();
    void ui_logoutMessage();
    void ui_renommer();

protected:
    void closeEvent(QCloseEvent *);

public:
    void paquetRecu(Paquet*);
    void appendChat(QString, QString);
    void afficheBulle(QString, QString, QSystemTrayIcon::MessageIcon, int);
    void chargeConfig();

    void handleClientSide(Paquet*, quint16);
    void handleHello(Paquet*, quint16);
    void handleAuth(Paquet*, quint16);
    void handleKick(Paquet*, quint16);
    void handleChat(Paquet*, quint16);
    void handleUserModification(Paquet*, quint16);
    void handlePing(Paquet*, quint16);
    void handleRegister(Paquet*, quint16);
    void handleLevelMod(Paquet*, quint16);
    void handleError(Paquet*, quint16);
    void handleWhoIs(Paquet*, quint16);
    void handleClientsList(Paquet*, quint16);
    void handleChannel(Paquet*, quint16);
    void handleChatCommands(QString&);

private:
    Ui::FenPrincipale *ui;

    QSystemTrayIcon *m_sysTray;

    //Socket
    QTcpSocket *m_socket;
    quint16 m_taillePaquet;

    QString m_pseudo;
    QString m_acctName;
    QString m_logoutMessage;

    QList<Channel> m_channels;

    bool m_quitOnDisconnect;
    QString m_html;
};

#endif // FENPRINCIPALE_H
