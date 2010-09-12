#ifndef OPCODE_H
#define OPCODE_H

#include "../shared/shared.h"
#include "fenprincipale.h"

class FenPrincipale;

struct OpCodeHandler
{
    const char* nom;
    void (FenPrincipale::*f)(Paquet *, quint16);
};

//On ajoute le mot-clé extern pour que la table puisse être utilisée ailleurs.
extern OpCodeHandler OpCodeTable[NB_OPCODES];

#endif // OPCODE_H
