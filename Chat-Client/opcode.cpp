#include "opcode.h"

OpCodeHandler OpCodeTable[NB_OPCODES] =
{
    {"CMSG_HELLO", &FenPrincipale::handleClientSide},
    {"SMSG_HELLO", &FenPrincipale::handleHello},
    {"CMSG_AUTH_SET_NAME", &FenPrincipale::handleClientSide},
    {"SMSG_AUTH_OK", &FenPrincipale::handleAuth},
    {"SMSG_AUTH_NAME_ALREADY_IN_USE", &FenPrincipale::handleAuth},
    {"SMSG_AUTH_IP_BANNED", &FenPrincipale::handleAuth},
    {"SMSG_AUTH_NAME_TOO_SHORT", &FenPrincipale::handleAuth},
    {"SMSG_KICK", &FenPrincipale::handleKick},
    {"CMSG_CHAT_MESSAGE", &FenPrincipale::handleClientSide},
    {"SMSG_CHAT_MESSAGE", &FenPrincipale::handleChat},
    {"SMSG_INVALID_MESSAGE", &FenPrincipale::handleChat},
    {"SMSG_NAME_NOT_SET", &FenPrincipale::handleChat}
};
