#include "opcode.h"

OpCodeHandler OpCodeTable[NB_OPCODES] =
{
    {"CMSG_HELLO", &FenPrincipale::handleHello},
    {"SMSG_HELLO", &FenPrincipale::handleServerSide},
    {"CMSG_AUTH_SET_NAME", &FenPrincipale::handleAuthSetName},
    {"SMSG_AUTH_OK", &FenPrincipale::handleServerSide},
    {"SMSG_AUTH_NAME_ALREADY_IN_USE", &FenPrincipale::handleServerSide},
    {"SMSG_AUTH_IP_BANNED", &FenPrincipale::handleServerSide},
    {"SMSG_AUTH_NAME_TOO_SHORT", &FenPrincipale::handleServerSide},
    {"SMSG_KICK", &FenPrincipale::handleServerSide},
    {"CMSG_CHAT_MESSAGE", &FenPrincipale::handleChatMessage},
    {"SMSG_CHAT_MESSAGE", &FenPrincipale::handleServerSide},
    {"SMSG_INVALID_MESSAGE", &FenPrincipale::handleServerSide},
    {"SMSG_NAME_NOT_SET", &FenPrincipale::handleServerSide}
};
