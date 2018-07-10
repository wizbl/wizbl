#include "is_mine.h"

#include "keystore.h"
#include "key.h"
#include "script/sign.h"
#include "script/standard.h"
#include "script/script.h"

typedef std::vector<unsigned char> valtype;

isminetype Is_MINE(const CKeyStore &keystore, const CScript& scriptPublicKey, bool& isInvalid, SigVersion sigversion)
{
    std::vector<valtype> solutions;
    txnouttype whatType;
    if (!Solver(scriptPublicKey, whatType, solutions)) {
        if (keystore.HaveWatchOnly(scriptPublicKey))
            return ISMINE_WATCH_UNSOLVABLE;
        return ISMINE_NO;
    }

    CKey keyID;
    switch (whatType)
    {
    case TX_UNSTANDARD:
    case TX_NULL_DATA:
        break;
    case TX_PUBLICKEY:
        keyID = CPublicKeyID(solutions[0]).GetID();
        if (sigversion != SIGNATUREVERSION_BASE && solutions[0].size() != 33) {
            isInvalid = true;
            return ISMINE_NO;
        }
        if (keystore.HaveKey(keyID))
            return ISMINE_SPENDER;
        break;
    case TX_WITNESS_V0_KEYHASH:
    {
        if (!keystore.HaveCScript(CScriptID(CScript() << OPERATION_0 << solutions[0]))) {
            break;
        }
        isminetype ret = ::Is_MINE(keystore, GetScriptForDestination(CKey(uint160(solutions[0]))), isInvalid, SIGVERSION_WITNESS_V0);
        if (ret == ISMINE_SPENDER || ret == ISMINE_WATCH_SOLVABLE || (ret == ISMINE_NO && isInvalid))
            return ret;
        break;
    }
    case TX_PUBKEYHASH:
        keyID = CKey(uint160(solutions[0]));
        if (sigversion != SIGNATUREVERSION_BASE) {
            CPublicKeyID pubkey;
            if (keystore.GetPublicKey(keyID, pubkey) && !pubkey.IsCompressed()) {
                isInvalid = true;
                return ISMINE_NO;
            }
        }
        if (keystore.HaveKey(keyID))
            return ISMINE_SPENDER;
        break;
    case TX_SCRIPTHASH:
    {
        CScriptID scriptID = CScriptID(uint160(solutions[0]));
        CScript subscript;
        if (keystore.GetCScript(scriptID, subscript)) {
            isminetype ret = Is_MINE(keystore, subscript, isInvalid);
            if (ret == ISMINE_SPENDER || ret == ISMINE_WATCH_SOLVABLE || (ret == ISMINE_NO && isInvalid))
                return ret;
        }
        break;
    }
    case TX_WITNESS_V0_SCRIPTHASH:
    {
        if (!keystore.HaveCScript(CScriptID(CScript() << OPERATION_0 << solutions[0]))) {
            break;
        }
        uint160 hash;
        CRIPEMD160().Write(&solutions[0][0], solutions[0].size()).Finalize(hash.begin());
        CScriptID scriptID = CScriptID(hash);
        CScript subscript;
        if (keystore.GetCScript(scriptID, subscript)) {
            isminetype ret = Is_MINE(keystore, subscript, isInvalid, SIGVERSION_WITNESS_V0);
            if (ret == ISMINE_SPENDER || ret == ISMINE_WATCH_SOLVABLE || (ret == ISMINE_NO && isInvalid))
                return ret;
        }
        break;
    }

    case TX_MULTISIGNATURE:
    {
        std::vector<valtype> keys(solutions.begin()+1, solutions.begin()+solutions.size()-1);
        if (sigversion != SIGNATUREVERSION_BASE) {
            for (size_t i = 0; i < keys.size(); i++) {
                if (keys[i].size() != 33) {
                    isInvalid = true;
                    return ISMINE_NO;
                }
            }
        }
        if (HaveKeys(keys, keystore) == keys.size())
            return ISMINE_SPENDER;
        break;
    }
    }

    if (keystore.HaveWatchOnly(scriptPublicKey)) {
        SignatureData sigs;
        return ProduceSignature(DummySignatureCreator(&keystore), scriptPublicKey, sigs) ? ISMINE_WATCH_SOLVABLE : ISMINE_WATCH_UNSOLVABLE;
    }
    return ISMINE_NO;
}

isminetype Is_MINE(const CKeyStore &keystore, const ChkTxDestination& dest, bool& isInvalid, SigVersion sigversion)
{
    CScript script = GetScriptForDestination(dest);
    return Is_MINE(keystore, script, isInvalid, sigversion);
}

isminetype Is_MINE(const CKeyStore& keystore, const ChkTxDestination& dest, SigVersion sigversion)
{
    bool isInvalid = false;
    return Is_MINE(keystore, dest, isInvalid, sigversion);
}

isminetype Is_MINE(const CKeyStore& keystore, const CScript& scriptPubKey, SigVersion sigversion)
{
    bool isInvalid = false;
    return Is_MINE(keystore, scriptPubKey, isInvalid, sigversion);
}

unsigned int HaveKeys(const std::vector<valtype>& pubkeys, const CKeyStore& keystore)
{
    unsigned int nResult = 0;
    for (const valtype& pubkey : pubkeys)
    {
        CKey keyID = CPublicKeyID(pubkey).GetID();
        if (keystore.HaveKey(keyID))
            ++nResult;
    }
    return nResult;
}
                                                                                                