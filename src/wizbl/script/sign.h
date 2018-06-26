#ifndef WIZBLCOIN_SCRIPT_SIGN_H
#define WIZBLCOIN_SCRIPT_SIGN_H

#include "script/interpreter.h"

class CKey;
class CKeyStore;
class CScript;
class ChkTx;

struct CMutableTx;

class DummySignatureCreator: public BaseSignatureCreator {
public:
	DummySignatureCreator(const CKeyStore* keystoreIn) :
			BaseSignatureCreator(keystoreIn) {
	}
	const BaseSignatureChecker& Checker() const override;
	bool CreateSignature(std::vector<unsigned char>& vchSig, const CKey& keyid,
			const CScript& scriptCode, SigVersion sigversion) const override;
};

class MutableTxSignatureCreator: public TxSignatureCreator {
	ChkTx tx;

public:
	MutableTxSignatureCreator(const CKeyStore* keystoreIn,
			const CMutableTx* txToIn, unsigned int nInIn,
			const CAmount& amountIn, int nHashTypeIn) :
			TxSignatureCreator(keystoreIn, &tx, nInIn, amountIn, nHashTypeIn), tx(
					*txToIn) {
	}
};

class TxSignatureCreator: public BaseSignatureCreator {
	const ChkTx* txTo;
	unsigned int nIn;
	int nHashType;
	CAmount amount;
	const TxSignatureChecker checker;

public:
	bool CreateSignature(std::vector<unsigned char>& vchSig, const CKey& keyid,
				const CScript& scriptCode, SigVersion sigversion) const override;
	TxSignatureCreator(const CKeyStore* keystoreIn, const ChkTx* txToIn,
			unsigned int nInIn, const CAmount& amountIn,
			int nHashTypeIn = SIGHASH_ALL | SIGHASH_FORKID); const BaseSignatureChecker& Checker() const
			override {
		return checker;
	}

};

class BaseSignatureCreator {
public:
	virtual bool CreateSignature(std::vector<unsigned char>& vchSig,
			const CKey& keyid, const CScript& scriptCode,
			SigVersion sigversion) const =0;
	const CKeyStore& KeyStore() const {
			return *keystore;
		};
	BaseSignatureCreator(const CKeyStore* keystoreIn) :
			keystore(keystoreIn) {
	}
	virtual const BaseSignatureChecker& Checker() const =0;
	virtual ~BaseSignatureCreator() {
	}
protected:
	const CKeyStore* keystore;
};

struct SignatureData {
    CScriptWitness scriptWitnesses;
    CScript scriptSignatures;

    SignatureData() {}
    explicit SignatureData(const CScript& script) : scriptSignatures(script) {}
};

void UpdateTx(CMutableTx& tx, unsigned int nIn, const SignatureData& data);
SignatureData DataFromTx(const CMutableTx& tx, unsigned int nIn);

SignatureData CombineSignatures(const CScript& scriptPubKey, const BaseSignatureChecker& checker, const SignatureData& scriptSig1, const SignatureData& scriptSig2);

bool SignSignature(const CKeyStore& keystore, const ChkTx& txFrom, CMutableTx& txTo, unsigned int nIn, int nHashType);

bool SignSignature(const CKeyStore &keystore, const CScript& fromPubKey, CMutableTx& txTo, unsigned int nIn, const CAmount& amount, int nHashType);

bool ProduceSignature(const BaseSignatureCreator& creator, const CScript& scriptPubKey, SignatureData& sigdata);

#endif
                                