#ifndef CHANNEL_H
#define CHANNEL_H

#include <QObject>
#include <QList>
#include <QString>

class FenPrincipale;
class Client;

class Channel : public QObject
{
Q_OBJECT
public:
    Channel(quint32, QString, quint8, QString, bool = false, FenPrincipale* = 0);
    ~Channel();

    void addUser(Client*);
    void removeUser(Client*);

    //Accesseurs
    quint32 getId() { return m_id; }
    QString getTitre() { return m_titre; }
    quint8 getReqLevel() { return m_reqLevel; }
    QString getPassword() { return m_password; }
    QList<Client *> getClients() { return m_users; }
    bool isDefault() { return m_defaultChannel; }

signals:
    void console(QString);

public slots:

protected:
    FenPrincipale *m_parent;
    quint32 m_id;
    QString m_titre;
    quint8 m_reqLevel;
    QString m_password;
    bool m_defaultChannel;

    QList<Client *> m_users;
};

#endif // CHANNEL_H
