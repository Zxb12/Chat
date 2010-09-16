#ifndef SHARED_H
#define SHARED_H

#define NB_OPCODES 0x12

enum OpCodeValues
{
    CMSG_HELLO                      = 0x00,
    SMSG_HELLO                      = 0x01,
    CMSG_AUTH_SET_NAME              = 0x02,
    CMSG_AUTH_RENAME                = 0x03,
    SMSG_AUTH_OK                    = 0x04,
    SMSG_AUTH_NAME_ALREADY_IN_USE   = 0x05,
    SMSG_AUTH_IP_BANNED             = 0x06,
    SMSG_AUTH_NAME_TOO_SHORT        = 0x07,
    SMSG_KICK                       = 0x08,
    CMSG_CHAT_MESSAGE               = 0x09,
    SMSG_CHAT_MESSAGE               = 0x0a,
    SMSG_INVALID_MESSAGE            = 0x0b,
    SMSG_NAME_NOT_SET               = 0x0c,
    SMSG_USER_JOINED                = 0x0d,
    SMSG_USER_LEFT                  = 0x0e,
    SMSG_USER_RENAMED               = 0x0f,
    SMSG_PING                       = 0x10,
    CMSG_PONG                       = 0x11,
};

#endif // SHARED_H
