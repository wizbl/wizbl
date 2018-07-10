
#ifndef WIZBLCOIN_SCRIPT_INTERPRETER_H
#define WIZBLCOIN_SCRIPT_INTERPRETER_H

#include "script_error.h"
#include "primitives/transaction.h"

#include <vector>
#include <stdint.h>
#include <string>

class CPublicKeyID;
class CScript;
class ChkTx;
class uint256;

enum
{
    SIGHASH_ALL = 1,
    SIGHASH_NONE = 2,
    SIGHASH_SINGLE = 3,
    SIGHASH_FORKID = 0x40,
    SIGHASH_ANYONECANPAY = 0x80,
};

enum
{
    FORKID_BCC = 0,
    FORKID_WBL = 79,
};

static const int FORKID_IN_USE = FORKID_WBL;

enum
{
    SCRIPT_VERIFY_NONE      = 0,
    SCRIPT_VERIFY_P2SH      = (1U << 0),
    SCRIPT_VERIFY_STRICTENC = (1U << 1),
    SCRIPT_VERIFY_DERSIG    = (1U << 2),
    SCRIPT_VERIFY_LOW_S     = (1U << 3),
    SCRIPT_VERIFY_NULLDUMMY = (1U << 4),
    SCRIPT_VERIFY_SIGPUSHONLY = (1U << 5),
    SCRIPT_VERIFY_MINIMALDATA = (1U << 6),
    SCRIPT_VERIFY_DISCOURAGE_UPGRADABLE_NOPS  = (1U << 7),
    SCRIPT_VERIFY_CLEANSTACK = (1U << 8),
    SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY = (1U << 9),
    SCRIPT_VERIFY_CHECKSEQUENCEVERIFY = (1U << 10),
    SCRIPT_VERIFY_WITNESS = (1U << 11),
    SCRIPT_VERIFY_DISCOURAGE_UPGRADABLE_WITNESS_PROGRAM = (1U << 12),
    SCRIPT_VERIFY_MINIMALIF = (1U << 13),
    SCRIPT_VERIFY_NULLFAIL = (1U << 14),
    SCRIPT_VERIFY_WITNESS_PUBKEYTYPE = (1U << 15),
    SCRIPT_ALLOW_NON_FORKID = (1U << 17),
};

bool CheckSignatureEncoding(const std::vector<unsigned char> &vchSig, unsigned int flags, ScriptError* serror);

struct PrecomputedTxData
{
    uint256 hashPrevouts, hashSeq, hashOutputs;

    PrecomputedTxData(const ChkTx& tx);
};

enum SigVersion
{
    SIGNATUREVERSION_BASE = 0,
    SIGVERSION_WITNESS_V0 = 1,
};

uint256 SignatureHash(const CScript &scriptCode, const ChkTx& txTo, unsigned int nIn, int nHashType, const CAmount& amount, SigVersion sigversion, const PrecomputedTxData* cache = nullptr, const int forkid=FORKID_IN_USE);

class BaseSignatureChecker
{
public:
	virtual bool CheckSeq(const CScriptNum& nSeq) const
	    {
	         return false;
	    }

    virtual bool CheckLockTime(const CScriptNum& nLockTime) const
    {
         return false;
    }

    virtual bool CheckSig(const std::vector<unsigned char>& scriptSig, const std::vector<unsigned char>& vchPubKey, const CScript& scriptCode, SigVersion sigversion) const
        {
            return false;
        }

    virtual ~BaseSignatureChecker() {}
};

class TxSignatureChecker : public BaseSignatureChecker
{
public:
	    bool CheckSeq(const CScriptNum& nSeq) const override;
	    bool CheckLockTime(const CScriptNum& nLockTime) const override;
	bool CheckSig(const std::vector<unsigned char>& scriptSig, const std::vector<unsigned char>& vchPubKey, const CScript& scriptCode, SigVersion sigversion) const override;
    TxSignatureChecker(const ChkTx* txToIn, unsigned int nInIn, const CAmount& amountIn, const PrecomputedTxData& txdataIn) : txTo(txToIn), nIn(nInIn), amount(amountIn), txdata(&txdataIn) {}
    TxSignatureChecker(const ChkTx* txToIn, unsigned int nInIn, const CAmount& amountIn) : txTo(txToIn), nIn(nInIn), amount(amountIn), txdata(nullptr) {}

protected:
    virtual bool VerifyingSignature(const std::vector<unsigned char>& vchSig, const CPublicKeyID& vchPubKey, const uint256& sighash) const;
private:
    const CAmount amount;
    const PrecomputedTxData* txdata;
    const ChkTx* txTo;
    unsigned int nIn;

};

class MutableTxSignatureChecker : public TxSignatureChecker
{
public:
    MutableTxSignatureChecker(const CMutableTx* txToIn, unsigned int nInIn, const CAmount& amountIn) : TxSignatureChecker(&txTo, nInIn, amountIn), txTo(*txToIn) {}
private:
    const ChkTx txTo;
};

size_t CountWitnessSigOps(const CScript& scriptSig, const CScript& scriptPubKey, const CScriptWitnesses* witness, unsigned int flags);
bool VerifyScript(const CScript& scriptSig, const CScript& scriptPubKey, const CScriptWitnesses* witness, unsigned int flags, const BaseSignatureChecker& checker, ScriptError* serror = nullptr);
#endif
                                                                                                   