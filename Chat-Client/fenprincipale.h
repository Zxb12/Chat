#ifndef FENPRINCIPALE_H
#define FENPRINCIPALE_H

#define TAILLE_MDP_MIN  5

#include <QWidget>
#include <QTcpSocket>
#include <QTime>
#include <QCryptographicHash>

#include "../shared/paquet.h"
#include "opcode.h"

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

public slots:
    void donneesRecues();
    void connecte();
    void deconnecte();
    void erreurSocket(QAbstractSocket::SocketError erreur);

public:
    void paquetRecu(Paquet*);

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
    void handleChatCommands(QString&);

private:
    Ui::FenPrincipale *ui;

    //Socket
    QTcpSocket *m_socket;
    quint16 m_taillePaquet;

    QString m_pseudo;
    QString m_acctName;

private slots:
    void on_pseudo_returnPressed();
    void on_connecter_clicked();
    void on_envoyer_clicked();
    void on_message_returnPressed();

};

#endif // FENPRINCIPALE_H
