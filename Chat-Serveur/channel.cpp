#include "channel.h"
#include "client.h"

#define CONSOLE(a) console(a)

Channel::Channel(quint32 id, QString titre, quint8 lvl, QString pass, bool defaultChannel, FenPrincipale *parent) : m_parent(parent), m_id(id),
                 m_titre(titre), m_reqLevel(lvl), m_password(pass), m_defaultChannel (defaultChannel), m_users()
{
    connect(this, SIGNAL(console(QString)), m_parent, SLOT(console(QString)));

    CONSOLE("Channel cr��: " + titre);
}

Channel::~Channel()
{

}

void Channel::addUser(Client *client)
{
    if (!m_users.contains(client))
    {
        m_users.append(client);
        client->setChannel(this);
        CONSOLE("Channel: Client ajout�: " + client->getPseudo());
    }
    else
        CONSOLE("ERREUR: Channel: Le client " + client->getPseudo() + " est d�j� dans le channel.");
}

void Channel::removeUser(Client *client)
{
    if (m_users.removeOne(client))
    {
        client->setChannel(0);
        CONSOLE("Channel: Client supprim�: " + client->getPseudo());
    }
    else
        CONSOLE("ERREUR: Channel: Client inexistant: " + client->getPseudo());
}
