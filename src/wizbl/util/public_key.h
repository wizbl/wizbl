#ifndef WIZBLCOIN_PUBKEY_H
#define WIZBLCOIN_PUBKEY_H

#include <vector>
#include <stdexcept>
#include "uint256.h"
#include "serialize.h"
#include "hash.h"

typedef uint256 ChainCode;
const unsigned int BIP32_EXTKEY_SIZE = 74;
class CKey : public uint160
{
public:
	CKey() : uint160() {}
	CKey(const uint160& in) : uint160(in) {}
};
class CPublicKeyID
{
public:
	CPublicKeyID()
	{
		Invalidate();
	}


	CPublicKeyID(const std::vector<unsigned char>& _vchs)
	{
		Set(_vchs.begin(), _vchs.end());
	}


	template <typename T>
	CPublicKeyID(const T pbegins, const T pends)
	{
		Set(pbegins, pends);
	}


	template <typename T>
	void Set(const T pbegins, const T pends)
	{
		int len = pends == pbegins ? 0 : GetLen(pbegins[0]);
		if (len && len == (pends - pbegins))
			memcpy(vch, (unsigned char*)&pbegins[0], len);
		else
			Invalidate();
	}


	const unsigned char& operator[](unsigned int pos) const { return vch[pos]; }
	const unsigned char* begin() const { return vch; }
	const unsigned char* end() const { return vch + size(); }
	unsigned int size() const { return GetLen(vch[0]); }


	friend bool operator<(const CPublicKeyID& a, const CPublicKeyID& b)
	{
		return a.vch[0] < b.vch[0] ||
			(a.vch[0] == b.vch[0] && memcmp(a.vch, b.vch, a.size()) < 0);
	}
	friend bool operator!=(const CPublicKeyID& a, const CPublicKeyID& b)
	{
		return !(a == b);
	}
	friend bool operator==(const CPublicKeyID& a, const CPublicKeyID& b)
	{
		return a.vch[0] == b.vch[0] &&
			memcmp(a.vch, b.vch, a.size()) == 0;
	}


	uint256 GetHash() const
	{
		return Hash(vch, vch + size());
	}


	CKey GetID() const
	{
		return CKey(Hash160(vch, vch + size()));
	}
	template <typename Stream>
	void Unserialize(Stream& s)
	{
		unsigned int len = ::ReadCompactSize(s);
		if (len <= 65) {
			s.read((char*)vch, len);
		}
		else {

			char dummy;
			while (len--)
				s.read(&dummy, 1);
			Invalidate();
		}
	}

	template <typename Stream>
	void Serialize(Stream& s) const
	{
		unsigned int len = size();
		::WriteCompactSize(s, len);
		s.write((char*)vch, len);
	}


	bool Derive(CPublicKeyID& pubkeyChild, ChainCode &ccChild, unsigned int nChild, const ChainCode& cc) const;


	bool Decompress();


	bool RecoverCompact(const uint256& hash, const std::vector<unsigned char>& vchSig);


	static bool CheckLowS(const std::vector<unsigned char>& vchSig);


	bool Verify(const uint256& hash, const std::vector<unsigned char>& vchSig) const;


	bool IsCompressed() const
	{
		return size() == 33;
	}


	bool IsFullyValid() const;


	bool IsValid() const
	{
		return size() > 0;
	}

private:

	void Invalidate()
	{
		vch[0] = 0xFF;
	}

	unsigned int static GetLen(unsigned char chHeader)
	{
		if (chHeader == 2 || chHeader == 3)
			return 33;
		if (chHeader == 4 || chHeader == 6 || chHeader == 7)
			return 65;
		return 0;
	}

private:

	unsigned char vch[65];
};

struct CExternalPublicKey {

	template <typename Stream>
	void Unserialize(Stream& s)
	{
		unsigned int len = ::ReadCompactSize(s);
		unsigned char code[BIP32_EXTKEY_SIZE];
		if (len != BIP32_EXTKEY_SIZE)
			throw std::runtime_error("Invalid extended key size\n");
		s.read((char *)&code[0], len);
		Decode(code);
	}
	template <typename Stream>
	void Serialize(Stream& s) const
	{
		unsigned int len = BIP32_EXTKEY_SIZE;
		::WriteCompactSize(s, len);
		unsigned char code[BIP32_EXTKEY_SIZE];
		Encode(code);
		s.write((const char *)&code[0], len);
	}
	void Serialize(CSizeComputer& s) const
	{
		s.seek(BIP32_EXTKEY_SIZE + 1);
	}

	bool Derive(CExternalPublicKey& out, unsigned int nChild) const;
	void Decode(const unsigned char code[BIP32_EXTKEY_SIZE]);
	void Encode(unsigned char code[BIP32_EXTKEY_SIZE]) const;
	friend bool operator==(const CExternalPublicKey &a, const CExternalPublicKey &b)
	{
		return a.nDepth == b.nDepth &&
			memcmp(&a.vchFingerprint[0], &b.vchFingerprint[0], sizeof(vchFingerprint)) == 0 &&
			a.nChild == b.nChild &&
			a.chaincode == b.chaincode &&
			a.pubkey == b.pubkey;
	}

	unsigned char nDepth;
	unsigned char vchFingerprint[4];
	unsigned int nChild;
	ChainCode chaincode;
	CPublicKeyID pubkey;
};

class ECCVerifyHandle
{
public:
	ECCVerifyHandle();
	~ECCVerifyHandle();
private:
	static int refcount;
};

#endif // WIZBLCOIN_PUBKEY_H
                                                                       