#ifndef FENPRINCIPALE_H
#define FENPRINCIPALE_H

#include <QWidget>
#include <QTcpSocket>
#include <QTime>

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
    void handleChatCommands(QString&);

private:
    Ui::FenPrincipale *ui;

    QTcpSocket *m_socket;
    QString m_pseudo;
    quint16 m_taillePaquet;

private slots:
    void on_pseudo_returnPressed();
    void on_connecter_clicked();
    void on_envoyer_clicked();
    void on_message_returnPressed();

};

#endif // FENPRINCIPALE_H
