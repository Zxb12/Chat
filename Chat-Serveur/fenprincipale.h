#ifndef FENPRINCIPALE_H
#define FENPRINCIPALE_H

#define VERSION         QString("Chat-0.0.7a")
#define VERSION_CONFIG  quint32(2010111201)  //YYYYMMDD + Numéro de la version du jour

#include <iostream>

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTime>
#include <QDataStream>

#include <QtSql>
#include <QFile>

#include "opcode.h"
#include "../shared/paquet.h"
#include "client.h"
#include "channel.h"

class Channel;

namespace Ui
{
    class FenPrincipale;
}

class FenPrincipale : public QObject
{
    Q_OBJECT
public:
    FenPrincipale(QObject *parent = 0);
    ~FenPrincipale();

    bool connecterBDD();
    bool chargerFichier();
    void chargerChannels();
    void envoyerAuServeur(Paquet&);
    void envoyerAuChannel(Paquet&, Channel*);

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
    void handleLogout(Paquet*, Client*);
    void handleSetLogoutMsg(Paquet*, Client*);
    void handleUpdateChannel(Paquet*, Client*);
    void handleChannelJoin(Paquet*, Client*);
    void handleChannelCreate(Paquet*, Client*);
    void handleChannelDelete(Paquet*, Client*);
    void handleChannelEdit(Paquet*, Client*);


public slots:
    void console(QString);

    void nouvelleConnexion();
    void decoClient();
    void kickClient(Client *);

    void paquetRecu(Paquet*);
    void supprimerChannel(Channel*);

public:
    //Accesseurs
    quint32 getPingInterval() { return m_pingInterval; }
    quint16 getMaxPingsPending() { return m_maxPingsPending; }


private:
    QTcpServer *m_serveur;
    QList<Client *> m_clients;
    QList<Channel *> m_channels;

    //Configuration
    QString m_SQLAdresse, m_SQLDatabase, m_SQLLogin, m_SQLPassword;
    quint16 m_serverPort;
    quint32 m_pingInterval;
    quint16 m_maxPingsPending, m_nickMinLength, m_nickMaxLength, m_accountNameMinLength;
    quint8  m_levelMax, m_registerLevel, m_kickLevel, m_banLevel, m_voiceLevel,
            m_promoteLevel, m_whoisLevel, m_channelCreateLevel, m_channelCreatePersistantLevel,
            m_channelDeleteLevel, m_channelEditLevel;
};

#endif // FENPRINCIPALE_H
