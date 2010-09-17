#ifndef SHARED_H
#define SHARED_H

#define NB_OPCODES 0x1b

enum OpCodeValues
{
    CMSG_HELLO                      = 0x00,
    SMSG_HELLO                      = 0x01,
    CMSG_AUTH_LOGIN                 = 0x02,        //acct_name, acct_pwhash, nick
    SMSG_AUTH_OK                    = 0x03,
    SMSG_AUTH_INCORRECT_LOGIN       = 0x04,
    SMSG_AUTH_ACCT_ALREADY_IN_USE   = 0x05,
    SMSG_AUTH_IP_BANNED             = 0x06,
    SMSG_AUTH_ACCT_BANNED           = 0x07,
    SMSG_AUTH_ERROR                 = 0x08,
    CMSG_SET_NICK                   = 0x09,
    SMSG_NICK_ALREADY_IN_USE        = 0x0a,
    SMSG_NICK_TOO_SHORT             = 0x0b,
    SMSG_KICK                       = 0x0c,
    CMSG_CHAT_MESSAGE               = 0x0d,
    SMSG_CHAT_MESSAGE               = 0x0e,
    SMSG_INVALID_MESSAGE            = 0x0f,
    SMSG_INVALID_NICK               = 0x10,
    SMSG_USER_JOINED                = 0x11,
    SMSG_USER_LEFT                  = 0x12,
    SMSG_USER_RENAMED               = 0x13,
    SMSG_PING                       = 0x14,
    CMSG_PONG                       = 0x15,
    CMSG_REGISTER                   = 0x16,
    SMSG_REG_OK                     = 0x17,
    SMSG_REG_ACCT_ALREADY_EXISTS    = 0x18,
    SMSG_REG_INVALID_NICK           = 0x19,
    SMSG_REG_ERROR                  = 0x1a,
};

#endif // SHARED_H
