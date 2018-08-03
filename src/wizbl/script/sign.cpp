#include "script/sign.h"

#include "keystore.h"
#include "key.h"
#include "primitives/transaction.h"
#include "policy/policy.h"
#include "uint256.h"
#include "script/standard.h"

typedef std::vector<unsigned char> valtype;

static std::vector<valtype> CombineMultisignatures(const CScript& scriptPublicKey, const BaseSignatureChecker& checker,
                               const std::vector<valtype>& solutions,
                               const std::vector<valtype>& signatures1, const std::vector<valtype>& signatures2, SigVersion sigversion)
{
    std::set<valtype> allsigs;
    for (const valtype& v : signatures1)
    {
        if (!v.empty())
            allsigs.insert(v);
    }
    for (const valtype& v : signatures2)
    {
        if (!v.empty())
            allsigs.insert(v);
    }

    unsigned int nSignaturesRequired = solutions.front()[0];
    unsigned int nPublicKeys = solutions.size()-2;
    std::map<valtype, valtype> sigs;
    for (const valtype& sig : allsigs)
    {
        for (unsigned int i = 0; i < nPublicKeys; i++)
        {
            const valtype& publickey = solutions[i+1];
            if (sigs.count(publickey))
                continue;

            if (checker.CheckSignatures(sig, publickey, scriptPublicKey, sigversion))
            {
                sigs[publickey] = sig;
                break;
            }
        }
    }

    unsigned int nSigsHave = 0;
    std::vector<valtype> result; result.push_back(valtype()); // pop-one-too-many workaround
    for (unsigned int i = 0; i < nPublicKeys && nSigsHave < nSignaturesRequired; i++)
    {
        if (sigs.count(solutions[i+1]))
        {
            result.push_back(sigs[solutions[i+1]]);
            ++nSigsHave;
        }
    }

    for (unsigned int i = nSigsHave; i < nSignaturesRequired; i++)
        result.push_back(valtype());

    return result;
}

bool SignSignature(const CKeyStore &keystore, const ChkTx& txFrom, CMutableTx& txTo, unsigned int nIn, int nHashType)
{
    assert(nIn < txTo.vin.size());
    ChkTxIn& txin = txTo.vin[nIn];
    assert(txin.prevout.n < txFrom.vout.size());
    const ChkTxOut& txout = txFrom.vout[txin.prevout.n];

    return SignSignature(keystore, txout.scriptPubKey, txTo, nIn, txout.nValue, nHashType);
}

bool SignSignature(const CKeyStore &keystore, const CScript& fromPubKey, CMutableTx& txTo, unsigned int nIn, const CAmount& amount, int nHashType)
{
    assert(nIn < txTo.vin.size());

    ChkTx txToConst(txTo);
    TxSignatureCreator creator(&keystore, &txToConst, nIn, amount, nHashType);

    SignatureData sigdata;
    bool ret = ProduceSignature(creator, fromPubKey, sigdata);
    UpdateTx(txTo, nIn, sigdata);
    return ret;
}

void UpdateTx(CMutableTx& tx, unsigned int nIn, const SignatureData& data)
{
    assert(tx.vin.size() > nIn);
    tx.vin[nIn].scriptSignatures = data.scriptSignatures;
    tx.vin[nIn].scriptWitnesses = data.scriptWitnesses;
}

SignatureData DataFromTx(const CMutableTx& tx, unsigned int nIn)
{
    SignatureData data;
    assert(tx.vin.size() > nIn);
    data.scriptSignatures = tx.vin[nIn].scriptSignatures;
    data.scriptWitnesses = tx.vin[nIn].scriptWitnesses;
    return data;
}

bool ProduceSignature(const BaseSignatureCreator& creator, const CScript& fromPublicKey, SignatureData& signaturedata)
{
    CScript cScript = fromPublicKey;
    std::vector<valtype> result;
    txnouttype whichType;
    bool solved = SignatureStep(creator, cScript, result, whichType, SIGNATUREVERSION_BASE);
    bool P2SH = false;
    CScript subscript;
    signaturedata.scriptWitnesses.stack.clear();

    if (solved && whichType == TX_SCRIPTHASH)
    {
        cScript = subscript = CScript(result[0].begin(), result[0].end());
        solved = solved && SignatureStep(creator, cScript, result, whichType, SIGNATUREVERSION_BASE) && whichType != TX_SCRIPTHASH;
        P2SH = true;
    }

    if (solved && whichType == TX_WITNESS_V0_KEYHASH)
    {
        CScript witnessscript;
        witnessscript << OPERATION_DUP << OPERATION_HASH160 << ToByteVector(result[0]) << OPERATION_EQUALVERIFY << OPERATION_CHECKSIG;
        txnouttype subType;
        solved = solved && SignatureStep(creator, witnessscript, result, subType, SIGVERSION_WITNESS_V0);
        signaturedata.scriptWitnesses.stack = result;
        result.clear();
    }
    else if (solved && whichType == TX_WITNESS_V0_SCRIPTHASH)
    {
        CScript witnessscript(result[0].begin(), result[0].end());
        txnouttype subType;
        solved = solved && SignatureStep(creator, witnessscript, result, subType, SIGVERSION_WITNESS_V0) && subType != TX_SCRIPTHASH && subType != TX_WITNESS_V0_SCRIPTHASH && subType != TX_WITNESS_V0_KEYHASH;
        result.push_back(std::vector<unsigned char>(witnessscript.begin(), witnessscript.end()));
        signaturedata.scriptWitnesses.stack = result;
        result.clear();
    }

    if (P2SH) {
        result.push_back(std::vector<unsigned char>(subscript.begin(), subscript.end()));
    }
    signaturedata.scriptSignatures = PushAll(result);

    return solved && VerifyScript(signaturedata.scriptSignatures, fromPublicKey, &signaturedata.scriptWitnesses, STANDARD_SCRIPT_VERIFY_FLAGS, creator.Checker());
}

static CScript PushAll(const std::vector<valtype>& values)
{
    CScript cScript;
    for (const valtype& v : values) {
        if (v.size() == 0) {
            cScript << OPERATION_0;
        } else if (v.size() == 1 && v[0] >= 1 && v[0] <= 16) {
            cScript << CScript::EncodeOPERATION_N(v[0]);
        } else {
            cScript << v;
        }
    }
    return cScript;
}

static bool SignatureStep(const BaseSignatureCreator& creator, const CScript& scriptPubicKey,
                     std::vector<valtype>& ret, txnouttype& whichTypeRet, SigVersion signatureVersion)
{
    CScript cScript;
    uint160 h160;
    ret.clear();

    std::vector<valtype> solutions;
    if (!Solver(scriptPubicKey, whichTypeRet, solutions))
        return false;

    CKey key;
    switch (whichTypeRet)
    {
    case TX_UNSTANDARD:
    case TX_NULL_DATA:
        return false;
    case TX_PUBLICKEY:
        key = CPublicKeyID(solutions[0]).GetID();
        return Signature1(key, creator, scriptPubicKey, ret, signatureVersion);
    case TX_PUBKEYHASH:
        key = CKey(uint160(solutions[0]));
        if (!Signature1(key, creator, scriptPubicKey, ret, signatureVersion))
            return false;
        else
        {
            CPublicKeyID vch;
            creator.KeyStore().GetPublicKey(key, vch);
            ret.push_back(ToByteVector(vch));
        }
        return true;
    case TX_SCRIPTHASH:
        if (creator.KeyStore().GetCScript(uint160(solutions[0]), cScript)) {
            ret.push_back(std::vector<unsigned char>(cScript.begin(), cScript.end()));
            return true;
        }
        return false;

    case TX_MULTISIGNATURE:
        ret.push_back(valtype());
        return (SignatureN(solutions, creator, scriptPubicKey, ret, signatureVersion));

    case TX_WITNESS_V0_KEYHASH:
        ret.push_back(solutions[0]);
        return true;

    case TX_WITNESS_V0_SCRIPTHASH:
        CRIPEMD160().Write(&solutions[0][0], solutions[0].size()).Finalize(h160.begin());
        if (creator.KeyStore().GetCScript(h160, cScript)) {
            ret.push_back(std::vector<unsigned char>(cScript.begin(), cScript.end()));
            return true;
        }
        return false;

    default:
        return false;
    }
}

static bool SignatureN(const std::vector<valtype>& multisignatureData, const BaseSignatureCreator& creator, const CScript& scriptCode, std::vector<valtype>& ret, SigVersion signatureVersion)
{
    int nSigned = 0;
    int nRequired = multisignatureData.front()[0];
    for (unsigned int i = 1; i < multisignatureData.size()-1 && nSigned < nRequired; i++)
    {
        const valtype& pubkey = multisignatureData[i];
        CKey keyID = CPublicKeyID(pubkey).GetID();
        if (Signature1(keyID, creator, scriptCode, ret, signatureVersion))
            ++nSigned;
    }
    return nSigned==nRequired;
}

static bool Signature1(const CKey& address, const BaseSignatureCreator& creator, const CScript& scriptCode, std::vector<valtype>& ret, SigVersion signatureVersion)
{
    std::vector<unsigned char> vchSig;
    if (!creator.CreateSignature(vchSig, address, scriptCode, signatureVersion))
        return false;
    ret.push_back(vchSig);
    return true;
}

bool TxSignatureCreator::CreateSignature(std::vector<unsigned char>& vchSig, const CKey& address, const CScript& scriptCode, SigVersion sigversion) const
{
    CKey key;
    if (!keystore->GetKey(address, key))
        return false;

    if (sigversion == SIGVERSION_WITNESS_V0 && !key.IsCompressed())
        return false;

    uint256 hash = SignatureHash(scriptCode, *txTo, nIn, nHashType, amount, sigversion);
    if (!key.Sign(hash, vchSig))
        return false;
    vchSig.push_back((unsigned char)nHashType);
    return true;
}

TxSignatureCreator::TxSignatureCreator(const CKeyStore* keystoreIn, const ChkTx* txToIn, unsigned int nInIn, const CAmount& amountIn, int nHashTypeIn) : BaseSignatureCreator(keystoreIn), txTo(txToIn), nIn(nInIn), nHashType(nHashTypeIn), amount(amountIn), checker(txTo, nIn, amountIn) {}

namespace
{
struct Stacks {
	std::vector<valtype> script;
	std::vector<valtype> witness;

	Stacks() {
	}
	SignatureData Output() const {
		SignatureData result;
		result.scriptSignatures = PushAll(script);
		result.scriptWitnesses.stack = witness;
		return result;
	}

	explicit Stacks(const SignatureData& data) :
			witness(data.scriptWitnesses.stack) {
		EvalScript(script, data.scriptSignatures, SCRIPT_VERIFY_STRICTENC,
				BaseSignatureChecker(), SIGNATUREVERSION_BASE);
	}
	explicit Stacks(const std::vector<valtype>& scriptSigStack_) :
			script(scriptSigStack_), witness() {
	}
};
}

SignatureData CombineSignatures(const CScript& scriptPublicKey, const BaseSignatureChecker& checker,
                          const SignatureData& scriptSignature1, const SignatureData& scriptSignature2)
{
    txnouttype txType;
    std::vector<std::vector<unsigned char> > vSolutions;
    Solver(scriptPublicKey, txType, vSolutions);

    return CombineSignatures(scriptPublicKey, checker, txType, vSolutions, Stacks(scriptSignature1), Stacks(scriptSignature2), SIGNATUREVERSION_BASE).Output();
}

static Stacks CombineSignatures(const CScript& scriptPubKey, const BaseSignatureChecker& checker,
                                 const txnouttype txType, const std::vector<valtype>& solutions,
                                 Stacks signatures1, Stacks signaturess2, SigVersion signatureVersion)
{
    switch (txType)
    {
    case TX_UNSTANDARD:
    case TX_NULL_DATA:
        if (signatures1.script.size() >= signaturess2.script.size())
            return signatures1;
        return signaturess2;
    case TX_PUBLICKEY:
    case TX_PUBKEYHASH:
        if (signatures1.script.empty() || signatures1.script[0].empty())
            return signaturess2;
        return signatures1;
    case TX_WITNESS_V0_KEYHASH:
        if (signatures1.witness.empty() || signatures1.witness[0].empty())
            return signaturess2;
        return signatures1;
    case TX_SCRIPTHASH:
        if (signatures1.script.empty() || signatures1.script.back().empty())
            return signaturess2;
        else if (signaturess2.script.empty() || signaturess2.script.back().empty())
            return signatures1;
        else
        {
            valtype snpk = signatures1.script.back();
            CScript pubKey2(snpk.begin(), snpk.end());

            txnouttype txType2;
            std::vector<std::vector<unsigned char> > vSolutions2;
            Solver(pubKey2, txType2, vSolutions2);
            signatures1.script.pop_back();
            signaturess2.script.pop_back();
            Stacks result = CombineSignatures(pubKey2, checker, txType2, vSolutions2, signatures1, signaturess2, signatureVersion);
            result.script.push_back(snpk);
            return result;
        }
    case TX_MULTISIGNATURE:
        return Stacks(CombineMultisignatures(scriptPubKey, checker, solutions, signatures1.script, signaturess2.script, signatureVersion));
    case TX_WITNESS_V0_SCRIPTHASH:
        if (signatures1.witness.empty() || signatures1.witness.back().empty())
            return signaturess2;
        else if (signaturess2.witness.empty() || signaturess2.witness.back().empty())
            return signatures1;
        else
        {
            CScript pubKey2(signatures1.witness.back().begin(), signatures1.witness.back().end());
            txnouttype txType2;
            std::vector<valtype> vSolutions2;
            Solver(pubKey2, txType2, vSolutions2);
            signatures1.witness.pop_back();
            signatures1.script = signatures1.witness;
            signatures1.witness.clear();
            signaturess2.witness.pop_back();
            signaturess2.script = signaturess2.witness;
            signaturess2.witness.clear();
            Stacks result = CombineSignatures(pubKey2, checker, txType2, vSolutions2, signatures1, signaturess2, SIGVERSION_WITNESS_V0);
            result.witness = result.script;
            result.script.clear();
            result.witness.push_back(valtype(pubKey2.begin(), pubKey2.end()));
            return result;
        }
    default:
        return Stacks();
    }
}

namespace {
class DummySignatureChecker : public BaseSignatureChecker
{
public:
    bool CheckSignatures(const std::vector<unsigned char>& scriptSig, const std::vector<unsigned char>& vchPubKey, const CScript& scriptCode, SigVersion sigversion) const override
    {
        return true;
    }
    DummySignatureChecker() {}
};
const DummySignatureChecker dummyChecker;
}

bool DummySignatureCreator::CreateSignature(std::vector<unsigned char>& vchSig, const CKey& keyid, const CScript& scriptCode, SigVersion sigversion) const
{
    vchSig.assign(72, '\000');
    vchSig[0] = 0x01;
    vchSig[1] = 0x02;
    vchSig[2] = 69;
    vchSig[3] = 33;
    vchSig[4] = 0x30;
    vchSig[4 + 33] = 32;
    vchSig[5 + 33] = 0x02;
    vchSig[6 + 33] = 0x01;
    vchSig[6 + 33 + 32] = SIGHASH_ALL | SIGHASH_FORKID;
    return true;
}

const BaseSignatureChecker& DummySignatureCreator::Checker() const
{
    return dummyChecker;
}
                                                                                                                                                                                     