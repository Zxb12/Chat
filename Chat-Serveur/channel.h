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
    Channel(quint32, QString, quint8, QString, QString, bool, bool = false, FenPrincipale* = 0);
    ~Channel();

    void addUser(Client*);
    void removeUser(Client*);

    //Accesseurs
    quint32 getId() { return m_id; }
    QString getTitre() { return m_titre; }
    void setTitre(QString titre) { m_titre = titre; }
    quint8 getReqLevel() { return m_reqLevel; }
    void setReqLevel(quint8 reqLevel) { m_reqLevel = reqLevel; }
    QString getPassword() { return m_password; }
    void setPassword(QString pass) { m_password = pass; }
    QString getCreatedBy() { return m_createdBy; }
    QList<Client *> getClients() { return m_users; }
    bool isDefault() { return m_defaultChannel; }
    bool isPersistant() { return m_persistant; }
    void setPersistant(bool persistant) { m_persistant = persistant; }

signals:
    void console(QString);
    void channelNeedsToBeRemoved(Channel*);

public slots:

protected:
    FenPrincipale *m_parent;
    quint32 m_id;
    QString m_titre;
    quint8 m_reqLevel;
    QString m_password;
    QString m_createdBy;
    bool m_persistant;
    bool m_defaultChannel;

    QList<Client *> m_users;
};

#endif // CHANNEL_H
