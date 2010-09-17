#include "opcode.h"

OpCodeHandler OpCodeTable[NB_OPCODES] =
{
    {"CMSG_HELLO",                          &FenPrincipale::handleClientSide},
    {"SMSG_HELLO",                          &FenPrincipale::handleHello},
    {"CMSG_AUTH_LOGIN",                     &FenPrincipale::handleClientSide},
    {"SMSG_AUTH_OK",                        &FenPrincipale::handleAuth},
    {"SMSG_AUTH_INCORRECT_LOGIN",           &FenPrincipale::handleAuth},
    {"SMSG_AUTH_ACCT_ALREADY_IN_USE",       &FenPrincipale::handleAuth},
    {"SMSG_AUTH_IP_BANNED",                 &FenPrincipale::handleAuth},
    {"SMSG_AUTH_ACCT_BANNED",               &FenPrincipale::handleAuth},
    {"SMSG_AUTH_ERROR",                     &FenPrincipale::handleAuth},
    {"CMSG_SET_NICK",                       &FenPrincipale::handleClientSide},
    {"SMSG_NICK_ALREADY_IN_USE",            &FenPrincipale::handleAuth},
    {"SMSG_NICK_TOO_SHORT",                 &FenPrincipale::handleAuth},
    {"SMSG_KICK",                           &FenPrincipale::handleKick},
    {"CMSG_CHAT_MESSAGE",                   &FenPrincipale::handleClientSide},
    {"SMSG_CHAT_MESSAGE",                   &FenPrincipale::handleChat},
    {"SMSG_INVALID_MESSAGE",                &FenPrincipale::handleChat},
    {"SMSG_INVALID_NICK",                   &FenPrincipale::handleChat},
    {"SMSG_USER_JOINED",                    &FenPrincipale::handleUserModification},
    {"SMSG_USER_LEFT",                      &FenPrincipale::handleUserModification},
    {"SMSG_USER_RENAMED",                   &FenPrincipale::handleUserModification},
    {"SMSG_PING",                           &FenPrincipale::handlePing},
    {"CMSG_PONG",                           &FenPrincipale::handleClientSide},
};
