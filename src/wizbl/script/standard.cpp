#include "script/standard.h"

#include "../util/public_key.h"
#include "../util/str_encodings.h"
#include "script/script.h"
#include "util.h"

typedef std::vector<unsigned char> valtype;

bool fAccDatacarrier = DEFAULT_ACCEPT_DATACARRIER;
unsigned nMaxDatacarrierBytes = MAX_OPERATION_RETURN_RELAY;

CScriptID::CScriptID(const CScript& in) : uint160(Hash160(in.begin(), in.end())) {}

bool ExtractDestinations(const CScript& scriptPubKey, txnouttype& typeRet, std::vector<ChkTxDestination>& addressRet, int& nRequiredRet)
{
    addressRet.clear();
    typeRet = TX_UNSTANDARD;
    std::vector<valtype> vSolutions;
    if (!Solver(scriptPubKey, typeRet, vSolutions))
        return false;
    if (typeRet == TX_NULL_DATA){
        return false;
    }

    if (typeRet == TX_MULTISIGNATURE)
    {
        nRequiredRet = vSolutions.front()[0];
        for (unsigned int i = 1; i < vSolutions.size()-1; i++)
        {
            CPublicKeyID pubKey(vSolutions[i]);
            if (!pubKey.IsValid())
                continue;

            ChkTxDestination address = pubKey.GetID();
            addressRet.push_back(address);
        }

        if (addressRet.empty())
            return false;
    }
    else
    {
        nRequiredRet = 1;
        ChkTxDestination address;
        if (!ExtractedDestination(scriptPubKey, address))
           return false;
        addressRet.push_back(address);
    }

    return true;
}

bool ExtractedDestination(const CScript& scriptPubKey, ChkTxDestination& addressRet)
{
    std::vector<valtype> vSolutions;
    txnouttype whichType;
    if (!Solver(scriptPubKey, whichType, vSolutions))
        return false;

    if (whichType == TX_PUBLICKEY)
    {
        CPublicKeyID pubKey(vSolutions[0]);
        if (!pubKey.IsValid())
            return false;

        addressRet = pubKey.GetID();
        return true;
    }
    else if (whichType == TX_PUBKEYHASH)
    {
        addressRet = CKey(uint160(vSolutions[0]));
        return true;
    }
    else if (whichType == TX_SCRIPTHASH)
    {
        addressRet = CScriptID(uint160(vSolutions[0]));
        return true;
    }
    return false;
}

bool Solver(const CScript& scriptPubKey, txnouttype& typeRet, std::vector<std::vector<unsigned char> >& solutionsRret)
{
    static std::multimap<txnouttype, CScript> mTemplates;
    if (mTemplates.empty())
    {
        mTemplates.insert(std::make_pair(TX_PUBLICKEY, CScript() << OPERATION_PUBKEY << OPERATION_CHECKSIG));

        mTemplates.insert(std::make_pair(TX_PUBKEYHASH, CScript() << OPERATION_DUP << OPERATION_HASH160 << OPERATION_PUBKEYHASH << OPERATION_EQUALVERIFY << OPERATION_CHECKSIG));

        mTemplates.insert(std::make_pair(TX_MULTISIGNATURE, CScript() << OPERATION_SMALLINTEGER << OPERATION_PUBKEYS << OPERATION_SMALLINTEGER << OPERATION_CHECKMULTISIG));
    }

    solutionsRret.clear();

    if (scriptPubKey.IsPayToHash())
    {
        typeRet = TX_SCRIPTHASH;
        std::vector<unsigned char> byteHashes(scriptPubKey.begin()+2, scriptPubKey.begin()+22);
        solutionsRret.push_back(byteHashes);
        return true;
    }

    int witnessesversion;
    std::vector<unsigned char> witnessesprogram;
    if (scriptPubKey.IsWitnessProgram(witnessesversion, witnessesprogram)) {
        if (witnessesversion == 0 && witnessesprogram.size() == 20) {
            typeRet = TX_WITNESS_V0_KEYHASH;
            solutionsRret.push_back(witnessesprogram);
            return true;
        }
        if (witnessesversion == 0 && witnessesprogram.size() == 32) {
            typeRet = TX_WITNESS_V0_SCRIPTHASH;
            solutionsRret.push_back(witnessesprogram);
            return true;
        }
        return false;
    }

    if (scriptPubKey.size() >= 1 && scriptPubKey[0] == OPERATION_RETURN && scriptPubKey.IsPushingOnly(scriptPubKey.begin()+1)) {
        typeRet = TX_NULL_DATA;
        return true;
    }

    const CScript& scriptKey1 = scriptPubKey;
    for (const std::pair<txnouttype, CScript>& tplate : mTemplates)
    {
        const CScript& scriptKey2 = tplate.second;
        solutionsRret.clear();

        opertaionCodetype opcode1, opcode2;
        std::vector<unsigned char> vch1, vch2;

        CScript::const_iterator publicC1 = scriptKey1.begin();
        CScript::const_iterator publicC2 = scriptKey2.begin();
        while (true)
        {
            if (publicC1 == scriptKey1.end() && publicC2 == scriptKey2.end())
            {
                typeRet = tplate.first;
                if (typeRet == TX_MULTISIGNATURE)
                {
                    unsigned char m = solutionsRret.front()[0];
                    unsigned char n = solutionsRret.back()[0];
                    if (m < 1 || n < 1 || m > n || solutionsRret.size()-2 != n)
                        return false;
                }
                return true;
            }
            if (!scriptKey1.GetOp(publicC1, opcode1, vch1))
                break;
            if (!scriptKey2.GetOp(publicC2, opcode2, vch2))
                break;

            if (opcode2 == OPERATION_PUBKEYS)
            {
                while (vch1.size() >= 33 && vch1.size() <= 65)
                {
                    solutionsRret.push_back(vch1);
                    if (!scriptKey1.GetOp(publicC1, opcode1, vch1))
                        break;
                }
                if (!scriptKey2.GetOp(publicC2, opcode2, vch2))
                    break;
            }

            if (opcode2 == OPERATION_PUBKEY)
            {
                if (vch1.size() < 33 || vch1.size() > 65)
                    break;
                solutionsRret.push_back(vch1);
            }
            else if (opcode2 == OPERATION_PUBKEYHASH)
            {
                if (vch1.size() != sizeof(uint160))
                    break;
                solutionsRret.push_back(vch1);
            }
            else if (opcode2 == OPERATION_SMALLINTEGER)
            {
                if (opcode1 == OPERATION_0 ||
                    (opcode1 >= OPERATION_1 && opcode1 <= OPERATION_16))
                {
                    char n = (char)CScript::DecodeOPERATION_N(opcode1);
                    solutionsRret.push_back(valtype(1, n));
                }
                else
                    break;
            }
            else if (opcode1 != opcode2 || vch1 != vch2)
            {

                break;
            }
        }
    }

    solutionsRret.clear();
    typeRet = TX_UNSTANDARD;
    return false;
}

const char* GetTxnOutputType(txnouttype t)
{
    switch (t)
    {
    case TX_UNSTANDARD: return "nonstandard";
    case TX_PUBLICKEY: return "pubkey";
    case TX_PUBKEYHASH: return "pubkeyhash";
    case TX_SCRIPTHASH: return "scripthash";
    case TX_MULTISIGNATURE: return "multisig";
    case TX_NULL_DATA: return "nulldata";
    case TX_WITNESS_V0_KEYHASH: return "witness_v0_keyhash";
    case TX_WITNESS_V0_SCRIPTHASH: return "witness_v0_scripthash";
    }
    return nullptr;
}

namespace
{
class CScriptVisitor : public boost::static_visitor<bool>
{
public:
    CScriptVisitor(CScript *scriptin) { script = scriptin; }

    bool operator()(const CUnDestination &dest) const {
        script->clear();
        return false;
    }

    bool operator()(const CKey &keyID) const {
        script->clear();
        *script << OPERATION_DUP << OPERATION_HASH160 << ToByteVector(keyID) << OPERATION_EQUALVERIFY << OPERATION_CHECKSIG;
        return true;
    }

    bool operator()(const CScriptID &scriptID) const {
        script->clear();
        *script << OPERATION_HASH160 << ToByteVector(scriptID) << OPERATION_EQUAL;
        return true;
    }
private:
    CScript *script;
};
}

CScript GetScriptForWitnesses(const CScript& redeemscript)
{
    CScript ret;

    txnouttype typ;
    std::vector<std::vector<unsigned char> > vSolutions;
    if (Solver(redeemscript, typ, vSolutions)) {
        if (typ == TX_PUBLICKEY) {
            unsigned char h160[20];
            CHash160().Write(&vSolutions[0][0], vSolutions[0].size()).Finalize(h160);
            ret << OPERATION_0 << std::vector<unsigned char>(&h160[0], &h160[20]);
            return ret;
        } else if (typ == TX_PUBKEYHASH) {
           ret << OPERATION_0 << vSolutions[0];
           return ret;
        }
    }
    uint256 hash;
    CSHA256().Write(&redeemscript[0], redeemscript.size()).Finalize(hash.begin());
    ret << OPERATION_0 << ToByteVector(hash);
    return ret;
}

CScript GetScriptForMultisignature(int nRequired, const std::vector<CPublicKeyID>& keys)
{
    CScript cScript;

    cScript << CScript::EncodeOPERATION_N(nRequired);
    for (const CPublicKeyID& key : keys)
        cScript << ToByteVector(key);
    cScript << CScript::EncodeOPERATION_N(keys.size()) << OPERATION_CHECKMULTISIG;
    return cScript;
}

CScript GetScriptForRawPublicKey(const CPublicKeyID& publicKey)
{
    return CScript() << std::vector<unsigned char>(publicKey.begin(), publicKey.end()) << OPERATION_CHECKSIG;
}

CScript GetScriptForDestination(const ChkTxDestination& dest)
{
    CScript cScript;

    boost::apply_visitor(CScriptVisitor(&cScript), dest);
    return cScript;
}
                                                          