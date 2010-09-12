#ifndef SHARED_H
#define SHARED_H

#define NB_OPCODES 0x0c

enum OpCodeValues
{
    CMSG_HELLO                      = 0x00,
    SMSG_HELLO                      = 0x01,
    CMSG_AUTH_SET_NAME              = 0x02,
    SMSG_AUTH_OK                    = 0x03,
    SMSG_AUTH_NAME_ALREADY_IN_USE   = 0x04,
    SMSG_AUTH_IP_BANNED             = 0x05,
    SMSG_AUTH_NAME_TOO_SHORT        = 0x06,
    SMSG_KICK                       = 0x07,
    CMSG_CHAT_MESSAGE               = 0x08,
    SMSG_CHAT_MESSAGE               = 0x09,
    SMSG_INVALID_MESSAGE            = 0x0a,
    SMSG_NAME_NOT_SET               = 0x0b,

};

#endif // SHARED_H
