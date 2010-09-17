#include "opcode.h"

OpCodeHandler OpCodeTable[NB_OPCODES] =
{
    {"CMSG_HELLO",                          &FenPrincipale::handleHello},
    {"SMSG_HELLO",                          &FenPrincipale::handleServerSide},
    {"CMSG_AUTH_LOGIN",                     &FenPrincipale::handleAuthLogin},
    {"SMSG_AUTH_OK",                        &FenPrincipale::handleServerSide},
    {"SMSG_AUTH_INCORRECT_LOGIN",           &FenPrincipale::handleServerSide},
    {"SMSG_AUTH_ACCT_ALREADY_IN_USE",       &FenPrincipale::handleServerSide},
    {"SMSG_AUTH_IP_BANNED",                 &FenPrincipale::handleServerSide},
    {"SMSG_AUTH_ACCT_BANNED",               &FenPrincipale::handleServerSide},
    {"SMSG_AUTH_ERROR",                     &FenPrincipale::handleServerSide},
    {"CMSG_SET_NICK",                       &FenPrincipale::handleSetNick},
    {"SMSG_NICK_ALREADY_IN_USE",            &FenPrincipale::handleServerSide},
    {"SMSG_NICK_TOO_SHORT",                 &FenPrincipale::handleServerSide},
    {"SMSG_KICK",                           &FenPrincipale::handleServerSide},
    {"CMSG_CHAT_MESSAGE",                   &FenPrincipale::handleChatMessage},
    {"SMSG_CHAT_MESSAGE",                   &FenPrincipale::handleServerSide},
    {"SMSG_INVALID_MESSAGE",                &FenPrincipale::handleServerSide},
    {"SMSG_INVALID_NICK",                   &FenPrincipale::handleServerSide},
    {"SMSG_USER_JOINED",                    &FenPrincipale::handleServerSide},
    {"SMSG_USER_LEFT",                      &FenPrincipale::handleServerSide},
    {"SMSG_USER_RENAMED",                   &FenPrincipale::handleServerSide},
    {"SMSG_PING",                           &FenPrincipale::handleServerSide},
    {"CMSG_PONG",                           &FenPrincipale::handlePing},
};
