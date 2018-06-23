#ifndef WIZBLCOIN_BASE58_H
#define WIZBLCOIN_BASE58_H

#include <vector>
#include <string>

#include "../support/allocators/zero_after_free.h"
#include "script/standard.h"
#include "script/script.h"
#include "key.h"
#include "chainparams.h"
#include "public_key.h"

inline bool DecodeBase58Check(const std::string& str, std::vector<unsigned char>& vchRet);

inline bool DecodeBase58Check(const char* psz, std::vector<unsigned char>& vchRet);

std::string EncodeBase58Check(const std::vector<unsigned char>& vchIn);

bool DecodeBase58(const std::string& str, std::vector<unsigned char>& vchRet);

bool DecodeBase58(const char* psz, std::vector<unsigned char>& vchRet);

std::string EncodeBase58(const std::vector<unsigned char>& vch);

std::string EncodeBase58(const unsigned char* pbegin, const unsigned char* pend);

class CBase58Data
{
protected:
	CBase58Data();
	void SetData(const std::vector<unsigned char> &vchVersionIn, const unsigned char *pbegin, const unsigned char *pend);
	void SetData(const std::vector<unsigned char> &vchVersionIn, const void* pdata, size_t nSize);

public:
	int CompareTo(const CBase58Data& b58) const;
	std::string ToString() const;
	bool SetString(const std::string& str);
	bool SetString(const char* psz, unsigned int nVersionBytes = 1);

	bool operator>=(const CBase58Data& b58) const { return CompareTo(b58) >= 0; }
	bool operator<=(const CBase58Data& b58) const { return CompareTo(b58) <= 0; }
	bool operator==(const CBase58Data& b58) const { return CompareTo(b58) == 0; }
	bool operator> (const CBase58Data& b58) const { return CompareTo(b58) > 0; }
	bool operator< (const CBase58Data& b58) const { return CompareTo(b58) < 0; }

protected:
	vector_uchar vchData;

	typedef std::vector<unsigned char, zero_after_free_alloc<unsigned char> > vector_uchar;

	std::vector<unsigned char> vchVersion;
};

class CWizblCoinAddress : public CBase58Data {
public:
	CWizblCoinAddress() {}
	CWizblCoinAddress(const ChkTxDestination &dest) { Set(dest); }
	CWizblCoinAddress(const ChkTxDestination &dest, const CChainParams &params) { Set(dest, params); }
	CWizblCoinAddress(const std::string& strAddress) { SetString(strAddress); }
	CWizblCoinAddress(const char* pszAddress) { SetString(pszAddress); }

	bool IsValid(const CChainParams &params) const;
	bool IsValid() const;
	bool Set(const ChkTxDestination &dest, const CChainParams &params);
	bool Set(const CScriptID &id, const CChainParams &params);
	bool Set(const CKey &id, const CChainParams &params);
	bool Set(const ChkTxDestination &dest);
	bool Set(const CScriptID &id);
	bool Set(const CKey &id);

	bool IsScript() const;
	bool GetKeyID(CKey &keyID, const CChainParams &params) const;
	bool GetKeyID(CKey &keyID) const;
	ChkTxDestination Get(const CChainParams &params) const;
	ChkTxDestination Get() const;
};

class CWizblCoinSecret : public CBase58Data
{
public:
	CWizblCoinSecret(const CKey& vchSecret) { SetKey(vchSecret); }
	CWizblCoinSecret() {}

	bool IsValid() const;
	bool SetString(const std::string& strSecret);
	bool SetString(const char* pszSecret);
	CKey GetKey();
	void SetKey(const CKey& vchSecret);
};

template<typename K, int Size, CChainParams::Base58Type Type>
class CWizblCoinExtKeyBase : public CBase58Data
{
public:
	CWizblCoinExtKeyBase() {}

	CWizblCoinExtKeyBase(const K &key) {
		SetKey(key);
	}

	CWizblCoinExtKeyBase(const std::string& strBase58c) {
		SetString(strBase58c.c_str(), Params().Base58Prefix(Type).size());
	}

	K GetKey() {
		K ret;
		if (vchData.size() == Size) {
			ret.Decode(vchData.data());
		}
		return ret;
	}

	void SetKey(const K &key) {
		unsigned char vch[Size];
		key.Encode(vch);
		SetData(Params().Base58Prefix(Type), vch, vch + Size);
	}
};

typedef CWizblCoinExtKeyBase<CExtKey, BIP32_EXTKEY_SIZE, CChainParams::EXT_SECRET_KEY> CWizblCoinExtKey;
typedef CWizblCoinExtKeyBase<CExternalPublicKey, BIP32_EXTKEY_SIZE, CChainParams::EXT_PUBLIC_KEY> CWizblCoinExtPubKey;

#endif // WIZBLCOIN_BASE58_H
                               