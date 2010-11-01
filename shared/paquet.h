#ifndef PAQUET_H
#define PAQUET_H

#include <QtGlobal>
#include <QTcpSocket>
#include "shared.h"

class Paquet
{
public:
    Paquet();
    Paquet(QByteArray);
    Paquet(const Paquet&);

    //Manipulation
    Paquet& operator<<(const bool&);
    Paquet& operator<<(const quint8&);
    Paquet& operator<<(const quint16&);
    Paquet& operator<<(const quint32&);
    Paquet& operator<<(const quint64&);
    Paquet& operator<<(const QString&);
    Paquet& operator<<(const OpCodeValues&);
    Paquet& operator<<(const QByteArray&);

    Paquet& operator>>(bool&);
    Paquet& operator>>(quint8&);
    Paquet& operator>>(quint16&);
    Paquet& operator>>(quint32&);
    Paquet& operator>>(quint64&);
    Paquet& operator>>(QString&);
    Paquet& operator>>(QByteArray&);

    bool operator>>(QTcpSocket*);

    //Envoie le paquet pr�par� � la socket.
    //Doit calculer la taille du paquet avant.
    bool send(QTcpSocket*);
    void clear();

private:
    QByteArray m_paquet;
    QDataStream m_stream;
};

#endif // PAQUET_H
