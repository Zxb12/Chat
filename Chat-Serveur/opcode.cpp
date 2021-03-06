#include "opcode.h"

OpCodeHandler OpCodeTable[NB_OPCODES] =
{
    {"CMSG_HELLO",                          NOT_CHECKED, &FenPrincipale::handleHello},
    {"SMSG_HELLO",                          NEVER,       &FenPrincipale::handleServerSide},
    {"CMSG_AUTH_LOGIN",                     CHECKED,     &FenPrincipale::handleAuthLogin},
    {"SMSG_AUTH_OK",                        NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_AUTH_INCORRECT_LOGIN",           NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_AUTH_ACCT_ALREADY_IN_USE",       NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_AUTH_IP_BANNED",                 NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_AUTH_ACCT_BANNED",               NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_AUTH_ERROR",                     NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_AUTH_INCORRECT_VERSION",         NEVER,       &FenPrincipale::handleServerSide},
    {"CMSG_SET_NICK",                       AUTHED,      &FenPrincipale::handleSetNick},
    {"SMSG_NICK_ALREADY_IN_USE",            NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_NICK_TOO_SHORT",                 NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_NICK_TOO_LONG",                  NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_KICK",                           NEVER,       &FenPrincipale::handleServerSide},
    {"CMSG_CHAT_MESSAGE",                   AUTHED,      &FenPrincipale::handleChatMessage},
    {"SMSG_CHAT_MESSAGE",                   NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_INVALID_MESSAGE",                NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_INVALID_NICK",                   NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_USER_JOINED",                    NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_USER_LEFT",                      NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_USER_RENAMED",                   NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_PING",                           NEVER,       &FenPrincipale::handleServerSide},
    {"CMSG_PONG",                           NOT_CHECKED, &FenPrincipale::handlePing},
    {"CMSG_REGISTER",                       AUTHED,      &FenPrincipale::handleRegister},
    {"SMSG_REG_OK",                         NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_REG_ACCT_ALREADY_EXISTS",        NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_REG_INVALID_NICK",               NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_REG_ERROR",                      NEVER,       &FenPrincipale::handleServerSide},
    {"CMSG_KICK",                           AUTHED,      &FenPrincipale::handleKick},
    {"CMSG_BAN",                            AUTHED,      &FenPrincipale::handleBan},
    {"CMSG_VOICE",                          AUTHED,      &FenPrincipale::handleVoice},
    {"CMSG_LVL_MOD",                        AUTHED,      &FenPrincipale::handleLevelMod},
    {"SMSG_NOT_AUTHORIZED",                 NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_USER_DOESNT_EXIST",              NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_LVL_MOD_ERROR",                  NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_USER_KICKED",                    NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_USER_BANNED",                    NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_USER_VOICED",                    NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_LVL_MOD_OK",                     NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_LVL_MOD_INVALID_LEVEL",          NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_LVL_MOD_ACCT_DOESNT_EXIST",      NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_LVL_MOD_LEVEL_TOO_HIGH",         NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_LVL_MOD_NOT_YOURSELF",           NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_LVL_CHANGED",                    NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_NO_INTERACT_HIGHER_LEVEL",       NEVER,       &FenPrincipale::handleServerSide},
    {"CMSG_WHOIS",                          AUTHED,      &FenPrincipale::handleWhoIs},
    {"SMSG_WHOIS",                          NEVER,       &FenPrincipale::handleServerSide},
    {"CMSG_UPDATE_CLIENTS_LIST",            AUTHED,      &FenPrincipale::handleUpdateClientsList},
    {"SMSG_CLIENTS_LIST",                   NEVER,       &FenPrincipale::handleServerSide},
    {"CMSG_LOGOUT",                         AUTHED,      &FenPrincipale::handleLogout},
    {"CMSG_SET_LOGOUT_MSG",                 AUTHED,      &FenPrincipale::handleSetLogoutMsg},
    {"CMSG_UPDATE_CHANNEL",                 AUTHED,      &FenPrincipale::handleUpdateChannel},
    {"SMSG_CHANNEL",                        NEVER,       &FenPrincipale::handleServerSide},
    {"CMSG_CHANNEL_JOIN",                   AUTHED,      &FenPrincipale::handleChannelJoin},
    {"SMSG_CHANNEL_JOIN",                   NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_CHANNEL_LEAVE",                  NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_CHANNEL_WRONG_PASSWORD",         NEVER,       &FenPrincipale::handleServerSide},
    {"SMSG_CHANNEL_LVL_TOO_LOW",            NEVER,       &FenPrincipale::handleServerSide},
};

