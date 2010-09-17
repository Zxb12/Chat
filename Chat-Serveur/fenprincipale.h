#ifndef FENPRINCIPALE_H
#define FENPRINCIPALE_H

#define PORT_SERVEUR        50180
#define TAILLE_PSEUDO_MIN   4
#define TAILLE_COMPTE_MIN   4

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

    void envoyerATous(Paquet&);
    void connecterBDD();

    //Handlers des opCodes re�us.
    void handleServerSide(Paquet*, Client*);
    void handleHello(Paquet*, Client*);
    void handleAuthLogin(Paquet*, Client*);
    void handleSetNick(Paquet*, Client*);
    void handleChatMessage(Paquet*, Client*);
    void handlePing(Paquet*, Client*);
    void handleRegister(Paquet*, Client*);


public slots:
    void console(QString);

    void nouvelleConnexion();
    void decoClient();
    void kickClient(Client *);

    void paquetRecu(Paquet*);

private:
    Ui::FenPrincipale *ui;

    QTcpServer *m_serveur;
    QList<Client *> m_clients;

};

#endif // FENPRINCIPALE_H
