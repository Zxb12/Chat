#include "channel.h"
#include "client.h"

Channel::Channel(quint32 id, QString titre, quint8 lvl, QString pass, bool defaultChannel, FenPrincipale *parent) : m_parent(parent), m_id(id),
                 m_titre(titre), m_reqLevel(lvl), m_password(pass), m_defaultChannel (defaultChannel), m_users()
{
    connect(this, SIGNAL(console(QString)), m_parent, SLOT(console(QString)));
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
    }
    else
        console("ERREUR: Channel: Le client " + client->getPseudo() + " est déjà dans le channel " + m_titre);
}

void Channel::removeUser(Client *client)
{
    if (m_users.removeOne(client))
    {
        client->setChannel(0);
    }
    else
        console("ERREUR: Channel: Client inexistant: " + client->getPseudo() + " dans " + m_titre);
}
