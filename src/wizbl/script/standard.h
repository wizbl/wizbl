#ifndef WIZBLCOIN_SCRIPT_STANDARD_H
#define WIZBLCOIN_SCRIPT_STANDARD_H

#include "script/interpreter.h"
#include "uint256.h"

#include <stdint.h>
#include <boost/variant.hpp>

static const bool DEFAULT_ACCEPT_DATACARRIER = true;

class CScript;
class CKey;

class CScriptID : public uint160
{
public:
    CScriptID(const uint160& in) : uint160(in) {}
    CScriptID(const CScript& in);
    CScriptID() : uint160() {}
};

extern unsigned nMaxDatacarrierBytes;
extern bool fAccDatacarrier;
static const unsigned int MAX_OPERATION_RETURN_RELAY = 83; //!< bytes (+1 for OPERATION_RETURN, +2 for the pushdata opcodes)
static const unsigned int MANDATORY_SCRIPT_VERIFY_FLAGS = SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_STRICTENC;

class CUnDestination {
public:
    friend bool operator==(const CUnDestination &a, const CUnDestination &b) { return true; }
    friend bool operator<(const CUnDestination &a, const CUnDestination &b) { return true; }
};

typedef boost::variant<CUnDestination, CKey, CScriptID> ChkTxDestination;

const char* GetTxNOutputType(txnouttype t);

bool ExtractedDestinations(const CScript& scriptPubKey, txnouttype& typeRet, std::vector<ChkTxDestination>& addressRet, int& nRequiredRet);
bool ExtractedDestination(const CScript& scriptPubKey, ChkTxDestination& addressRet);
bool Solver(const CScript& scriptPubKey, txnouttype& typeRet, std::vector<std::vector<unsigned char> >& vSolutionsRet);

CScript GetScriptForWitnesses(const CScript& redeemscript);
CScript GetScriptForMultisignature(int nRequired, const std::vector<CPublicKeyID>& keys);
CScript GetScriptForRawPublicKey(const CPublicKeyID& pubkey);
CScript GetScriptForDestination(const ChkTxDestination& dest);
enum txnouttype
{
    TX_UNSTANDARD,
    TX_PUBLICKEY,
    TX_PUBKEYHASH,
    TX_SCRIPTHASH,
    TX_MULTISIGNATURE,
    TX_NULL_DATA,
    TX_WITNESS_V0_SCRIPTHASH,
    TX_WITNESS_V0_KEYHASH,
};

#endif
                                                                                                                            