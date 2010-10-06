#ifndef FENPRINCIPALE_H
#define FENPRINCIPALE_H

#define VERSION         QString("Chat-0.0.1a")

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTime>
#include <QDataStream>

#include <QtSql>
#include <QFile>

#include "opcode.h"
#include "../shared/paquet.h"
#include "client.h"

namespace Ui
{
    class FenPrincipale;
}

class FenPrincipale : public QWidget
{
    Q_OBJECT
public:
    FenPrincipale(QWidget *parent = 0);
    ~FenPrincipale();

    bool chargerFichier();
    void envoyerATous(Paquet&);
    void connecterBDD();

    //Handlers des opCodes reçus.
    void handleServerSide(Paquet*, Client*);
    void handleHello(Paquet*, Client*);
    void handleAuthLogin(Paquet*, Client*);
    void handleSetNick(Paquet*, Client*);
    void handleChatMessage(Paquet*, Client*);
    void handlePing(Paquet*, Client*);
    void handleRegister(Paquet*, Client*);
    void handleKick(Paquet*, Client*);
    void handleBan(Paquet*, Client*);
    void handleVoice(Paquet*, Client*);
    void handleLevelMod(Paquet*, Client*);
    void handleWhoIs(Paquet*, Client*);
    void handleUpdateClientsList(Paquet*, Client*);


public slots:
    void console(QString);

    void nouvelleConnexion();
    void decoClient();
    void kickClient(Client *);

    void paquetRecu(Paquet*);

public:
    //Accesseurs
    quint32 getPingInterval() { return m_pingInterval; }
    quint16 getMaxPingsPending() { return m_maxPingsPending; }


private:
    Ui::FenPrincipale *ui;

    QTcpServer *m_serveur;
    QList<Client *> m_clients;

    //Configuration
    QString m_SQLAdresse, m_SQLDatabase, m_SQLLogin, m_SQLPassword;
    quint16 m_serverPort;
    quint32 m_pingInterval;
    quint16 m_maxPingsPending, m_nickMinLength, m_accountNameMinLength, m_levelMax,
            m_registerLevel, m_kickLevel, m_banLevel, m_voiceLevel, m_promoteLevel,
            m_whoisLevel;
};

#endif // FENPRINCIPALE_H
