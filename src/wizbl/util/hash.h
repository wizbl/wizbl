#ifndef WIZBLCOIN_HASH_H
#define WIZBLCOIN_HASH_H

#include <vector>
#include "version.h"
#include "uint256.h"
#include "serialize.h"
#include "crypto/sha256.h"
#include "crypto/ripemd160.h"
#include "pre_vector.h"

typedef uint256 ChainCode;

uint64_t SipHashUint256Extra(uint64_t k0, uint64_t k1, const uint256& val, uint32_t extra);
uint64_t SipHashUint256(uint64_t k0, uint64_t k1, const uint256& val);

template<typename T>
uint256 SerializeHash(const T& obj, int nType = SER_GETHASH, int nVersion = PROTOCOL_VERSION)
{
	CHashWriter ss(nType, nVersion);
	ss << obj;
	return ss.GetHash();
}

unsigned int MurmurHash3(unsigned int nHashSeed, const std::vector<unsigned char>& vDataToHash);

void BIP32Hash(const ChainCode &chainCode, unsigned int nChild, unsigned char header, const unsigned char data[32], unsigned char output[64]);

template<typename T1, typename T2, typename T3>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
	const T2 p2begin, const T2 p2end,
	const T3 p3begin, const T3 p3end) {
	static const unsigned char pblank[1] = {};
	uint256 result;
	CHash256().Write(p1begin == p1end ? pblank : (const unsigned char*)&p1begin[0], (p1end - p1begin) * sizeof(p1begin[0]))
		.Write(p2begin == p2end ? pblank : (const unsigned char*)&p2begin[0], (p2end - p2begin) * sizeof(p2begin[0]))
		.Write(p3begin == p3end ? pblank : (const unsigned char*)&p3begin[0], (p3end - p3begin) * sizeof(p3begin[0]))
		.Finalize((unsigned char*)&result);
	return result;
}

template<typename T1, typename T2>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
	const T2 p2begin, const T2 p2end) {
	static const unsigned char ptrBlank[1] = {};
	uint256 hashResult;
	CHash256().Write(p1begin == p1end ? ptrBlank : (const unsigned char*)&p1begin[0], (p1end - p1begin) * sizeof(p1begin[0]))
		.Write(p2begin == p2end ? ptrBlank : (const unsigned char*)&p2begin[0], (p2end - p2begin) * sizeof(p2begin[0]))
		.Finalize((unsigned char*)&hashResult);
	return hashResult;
}

template<typename T1>
inline uint256 Hash(const T1 pbegin, const T1 pend)
{
	static const unsigned char ptrBlank[1] = {};
	uint256 hashResult;
	CHash256().Write(pbegin == pend ? ptrBlank : (const unsigned char*)&pbegin[0], (pend - pbegin) * sizeof(pbegin[0]))
		.Finalize((unsigned char*)&hashResult);
	return hashResult;
}

template<unsigned int N>
inline uint160 Hash160(const prevector<N, unsigned char>& vch)
{
	return Hash160(vch.begin(), vch.end());
}

inline uint160 Hash160(const std::vector<unsigned char>& vch)
{
	return Hash160(vch.begin(), vch.end());
}

template<typename T1>
inline uint160 Hash160(const T1 pbegin, const T1 pend)
{
	static unsigned char pblank[1] = {};
	uint160 result;
	CHash160().Write(pbegin == pend ? pblank : (const unsigned char*)&pbegin[0], (pend - pbegin) * sizeof(pbegin[0]))
		.Finalize((unsigned char*)&result);
	return result;
}

class CHash256 {
public:
	CHash256 & Reset() {
		sha.Reset();
		return *this;
	}
	CHash256& Write(const unsigned char *data, size_t len) {
		sha.Write(data, len);
		return *this;
	}
	void Finalize(unsigned char hash[OUTPUT_SIZE]) {
		unsigned char buf[CSHA256::OUTPUT_SIZE];
		sha.Finalize(buf);
		sha.Reset().Write(buf, CSHA256::OUTPUT_SIZE).Finalize(hash);
	}
private:
	CSHA256 sha;
public:
	static const size_t OUTPUT_SIZE = CSHA256::OUTPUT_SIZE;
};

class CHash160 {
public:
	CHash160 & Reset() {
		sha.Reset();
		return *this;
	}
	CHash160& Write(const unsigned char *data, size_t len) {
		sha.Write(data, len);
		return *this;
	}
	void Finalize(unsigned char hash[OUTPUT_SIZE]) {
		unsigned char buf[CSHA256::OUTPUT_SIZE];
		sha.Finalize(buf);
		CRIPEMD160().Write(buf, CSHA256::OUTPUT_SIZE).Finalize(hash);
	}
private:
	CSHA256 sha;
public:
	static const size_t OUTPUT_SIZE = CRIPEMD160::OUTPUT_SIZE;
};

class CHashWriter
{
public:
	CHashWriter(int nTypeIn, int nVersionIn) : nType(nTypeIn), nVersion(nVersionIn) {}

	template<typename T>
	CHashWriter& operator<<(const T& obj) {
		::Serialize(*this, obj);
		return (*this);
	}

	uint256 GetHash() {
		uint256 result;
		ctx.Finalize((unsigned char*)&result);
		return result;
	}

	void write(const char *pch, size_t size) {
		ctx.Write((const unsigned char*)pch, size);
	}

	int GetVersion() const { return nVersion; }

	int GetType() const { return nType; }
private:
	CHash256 ctx;

	const int nType;
	const int nVersion;
};

template<typename Source>
class CHashVerifier : public CHashWriter
{
public:
	CHashVerifier(Source* source_) : CHashWriter(source_->GetType(), source_->GetVersion()), source(source_) {}

	template<typename T>
	CHashVerifier<Source>& operator>>(T& obj)
	{
		::Unserialize(*this, obj);
		return (*this);
	}

	void ignore(size_t nSize)
	{
		char data[1024];
		while (nSize > 0) {
			size_t current = std::min<size_t>(nSize, 1024);
			read(data, current);
			nSize -= current;
		}
	}

	void read(char* pch, size_t nSize)
	{
		source->read(pch, nSize);
		this->write(pch, nSize);
	}
private:
	Source * source;
};

class CSipHasher
{
public:
	CSipHasher(uint64_t k0, uint64_t k1);
	uint64_t Finalize() const;

	CSipHasher& Write(const unsigned char* data, size_t size);

	CSipHasher& Write(uint64_t data);
private:
	uint64_t v[4];
	uint64_t tmp;
	int count;
};

#endif
                                                                                      