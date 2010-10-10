#ifndef OPCODE_H
#define OPCODE_H

#include "../shared/shared.h"
#include "fenprincipale.h"

class FenPrincipale;
class Paquet;
class Client;

enum SessionState
{
    NOT_CHECKED,
    CHECKED,
    AUTHED,
    NEVER,
};

struct OpCodeHandler
{
    const char* nom;
    SessionState state;
    void (FenPrincipale::*f)(Paquet *, Client*);
};

//On ajoute le mot-clé extern pour que la table puisse être utilisée ailleurs.
extern OpCodeHandler OpCodeTable[NB_OPCODES];

#endif // OPCODE_H
