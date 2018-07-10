
#ifndef WIZBLCOIN_SCRIPT_IS_MINE_H
#define WIZBLCOIN_SCRIPT_IS_MINE_H

#include "script/standard.h"

#include <stdint.h>

class CScript;
class CKeyStore;

enum isminetype
{
    ISMINE_NO = 0,
    ISMINE_WATCH_UNSOLVABLE = 1,
    ISMINE_WATCH_SOLVABLE = 2,
    ISMINE_WATCH_ONLY = ISMINE_WATCH_SOLVABLE | ISMINE_WATCH_UNSOLVABLE,
    ISMINE_SPENDER = 4,
    ISMINE_ALL = ISMINE_WATCH_ONLY | ISMINE_SPENDER
};
typedef uint8_t isminefilter;

isminetype Is_MINE(const CKeyStore& keystore, const ChkTxDestination& dest, SigVersion = SIGNATUREVERSION_BASE);
isminetype Is_MINE(const CKeyStore& keystore, const ChkTxDestination& dest, bool& isInvalid, SigVersion = SIGNATUREVERSION_BASE);
isminetype Is_MINE(const CKeyStore& keystore, const CScript& scriptPubKey, SigVersion = SIGNATUREVERSION_BASE);
isminetype Is_MINE(const CKeyStore& keystore, const CScript& scriptPubKey, bool& isInvalid, SigVersion = SIGNATUREVERSION_BASE);
#endif
                                                                                                                        