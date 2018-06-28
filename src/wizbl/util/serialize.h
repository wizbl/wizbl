#ifndef WIZBLCOIN_SERIALIZE_H
#define WIZBLCOIN_SERIALIZE_H

#include <vector>
#include <utility>
#include <string>
#include <string.h>
#include <stdint.h>
#include <set>
#include <memory>
#include <map>
#include <limits>
#include <ios>
#include <assert.h>
#include <algorithm>
#include "compat/endian.h"
#include "pre_vector.h"

static const unsigned int MAX_SIZE = 0x02000000;
template<typename T>
inline T& REF(const T& val)
{
	return const_cast<T&>(val);
}
template<typename T>
inline T* NCONST_PTR(const T* val)
{
	return const_cast<T*>(val);
}
template<typename Stream> inline void ser_dataWriter8(Stream &s, uint8_t obj)
{
	s.write((char*)&obj, 1);
}
template<typename Stream> inline uint8_t dataReader8(Stream &s)
{
	uint8_t obj;
	s.read((char*)&obj, 1);
	return obj;
}
template<typename Stream> inline void ser_dataWriter16(Stream &s, uint16_t obj)
{
	obj = htole16(obj);
	s.write((char*)&obj, 2);
}
template<typename Stream> inline uint16_t ser_dataReader16(Stream &s)
{
	uint16_t obj;
	s.read((char*)&obj, 2);
	return le16toh(obj);
}
template<typename Stream> inline void ser_dataWriter32(Stream &s, uint32_t obj)
{
	obj = htole32(obj);
	s.write((char*)&obj, 4);
}
template<typename Stream> inline uint32_t ser_dataReader32(Stream &s)
{
	uint32_t obj;
	s.read((char*)&obj, 4);
	return le32toh(obj);
}
template<typename Stream> inline void ser_dataWriter64(Stream &s, uint64_t obj)
{
	obj = htole64(obj);
	s.write((char*)&obj, 8);
}
template<typename Stream> inline uint64_t ser_dataReader64(Stream &s)
{
	uint64_t obj;
	s.read((char*)&obj, 8);
	return le64toh(obj);
}
inline uint32_t ser_floatToUint32(float x)
{
	union { float x; uint32_t y; } tmp;
	tmp.x = x;
	return tmp.y;
}
inline float ser_uint32ToFloat(uint32_t y)
{
	union { float x; uint32_t y; } tmp;
	tmp.y = y;
	return tmp.x;
}
inline uint64_t ser_doubleToUint64(double x)
{
	union { double x; uint64_t y; } tmp;
	tmp.x = x;
	return tmp.y;
}
inline double ser_uint64ToDouble(uint64_t y)
{
	union { double x; uint64_t y; } tmp;
	tmp.y = y;
	return tmp.x;
}
class CSizeComputer;

enum
{
	SER_NETWORK = (1 << 0),
	SER_DISK = (1 << 1),
	SER_GETHASH = (1 << 2),
};

#define READNWRITEMANY(...)      (::SerReadWriteMany(s, ser_action, __VA_ARGS__))
#define READNWRITE(obj)      (::SerReadWrite(s, (obj), ser_action))

template<typename Stream> inline void Unserialize(Stream& s, bool& a) { char f = ser_dataReader8(s); a = f; }
template<typename Stream> inline void Unserialize(Stream& s, char& a) { a = ser_dataReader8(s); }
template<typename Stream> inline void Unserialize(Stream& s, int8_t& a) { a = ser_dataReader8(s); }
template<typename Stream> inline void Unserialize(Stream& s, uint8_t& a) { a = ser_dataReader8(s); }
template<typename Stream> inline void Unserialize(Stream& s, int16_t& a) { a = ser_dataReader16(s); }
template<typename Stream> inline void Unserialize(Stream& s, uint16_t& a) { a = ser_dataReader16(s); }
template<typename Stream> inline void Unserialize(Stream& s, int32_t& a) { a = ser_dataReader32(s); }
template<typename Stream> inline void Unserialize(Stream& s, uint32_t& a) { a = ser_dataReader32(s); }
template<typename Stream> inline void Unserialize(Stream& s, int64_t& a) { a = ser_dataReader64(s); }
template<typename Stream> inline void Unserialize(Stream& s, uint64_t& a) { a = ser_dataReader64(s); }
template<typename Stream> inline void Unserialize(Stream& s, float& a) { a = ser_uint32_to_float(ser_dataReader32(s)); }
template<typename Stream> inline void Unserialize(Stream& s, double& a) { a = ser_uint64_to_double(ser_dataReader64(s)); }

template<typename Stream> inline void Serialize(Stream& s, bool a) { char f = a; dataWriter8(s, f); }
template<typename Stream> inline void Serialize(Stream& s, char a) { dataWriter8(s, a); }
template<typename Stream> inline void Serialize(Stream& s, int8_t a) { dataWriter8(s, a); }
template<typename Stream> inline void Serialize(Stream& s, uint8_t a) { dataWriter8(s, a); }
template<typename Stream> inline void Serialize(Stream& s, int16_t a) { ser_dataWriter16(s, a); }
template<typename Stream> inline void Serialize(Stream& s, uint16_t a) { ser_dataWriter16(s, a); }
template<typename Stream> inline void Serialize(Stream& s, int32_t a) { ser_dataWriter32(s, a); }
template<typename Stream> inline void Serialize(Stream& s, uint32_t a) { ser_dataWriter32(s, a); }
template<typename Stream> inline void Serialize(Stream& s, int64_t a) { ser_dataWriter64(s, a); }
template<typename Stream> inline void Serialize(Stream& s, uint64_t a) { ser_dataWriter64(s, a); }
template<typename Stream> inline void Serialize(Stream& s, float a) { ser_dataWriter32(s, ser_floatToUint32(a)); }
template<typename Stream> inline void Serialize(Stream& s, double a) { ser_dataWriter64(s, ser_doubleToUint64(a)); }

template<typename Stream>
uint64_t ReadCompactSize(Stream& is)
{
	uint8_t chSize = ser_dataReader8(is);
	uint64_t nSizeRet = 0;
	if (chSize < 253)
	{
		nSizeRet = chSize;
	}
	else if (chSize == 253)
	{
		nSizeRet = ser_dataReader16(is);
		if (nSizeRet < 253)
			throw std::ios_base::fail("non-canonical ReadCompactSize()");
	}
	else if (chSize == 254)
	{
		nSizeRet = ser_dataReader32(is);
		if (nSizeRet < 0x10000u)
			throw std::ios_base::fail("non-canonical ReadCompactSize()");
	}
	else
	{
		nSizeRet = ser_dataReader64(is);
		if (nSizeRet < 0x100000000ULL)
			throw std::ios_base::fail("non-canonical ReadCompactSize()");
	}
	if (nSizeRet > (uint64_t)MAX_SIZE)
		throw std::ios_base::fail("ReadCompactSize(): size too large");
	return nSizeRet;
}

inline void WriteCompactSize(CSizeComputer& os, uint64_t nSize);

template<typename Stream>
void WriteCompactSize(Stream& os, uint64_t nSize)
{
	if (nSize < 253)
	{
		dataWriter8(os, nSize);
	}
	else if (nSize <= std::numeric_limits<unsigned short>::max())
	{
		dataWriter8(os, 253);
		ser_dataWriter16(os, nSize);
	}
	else if (nSize <= std::numeric_limits<unsigned int>::max())
	{
		dataWriter8(os, 254);
		ser_dataWriter32(os, nSize);
	}
	else
	{
		dataWriter8(os, 255);
		ser_dataWriter64(os, nSize);
	}
	return;
}

template<typename Stream, typename I>
I ReadVarInt(Stream& is)
{
	I n = 0;
	while (true) {
		unsigned char chData = ser_dataReader8(is);
		if (n > (std::numeric_limits<I>::max() >> 7)) {
			throw std::ios_base::fail("ReadVarInt(): size too large");
		}
		n = (n << 7) | (chData & 0x7F);
		if (chData & 0x80) {
			if (n == std::numeric_limits<I>::max()) {
				throw std::ios_base::fail("ReadVarInt(): size too large");
			}
			n++;
		}
		else {
			return n;
		}
	}
}

template<typename I>
inline void WriteVariableInt(CSizeComputer& os, I n);

template<typename Stream, typename I>
void WriteVarInt(Stream& os, I n)
{
	unsigned char tmp[(sizeof(n) * 8 + 6) / 7];
	int len = 0;
	while (true) {
		tmp[len] = (n & 0x7F) | (len ? 0x80 : 0x00);
		if (n <= 0x7F)
			break;
		n = (n >> 7) - 1;
		len++;
	}
	do {
		dataWriter8(os, tmp[len]);
	} while (len--);
}

#define LIMITED_STRING(obj,n) REF(LimitedString< n >(REF(obj)))
#define COMPACTSIZE(obj) REF(CCompactSize(REF(obj)))
#define VARINT(obj) REF(WrapVarInt(REF(obj)))
#define FLATDATA(obj) REF(CFlatData((char*)&(obj), (char*)&(obj) + sizeof(obj)))

class CFlatData
{
public:
	CFlatData(void* pbeginIn, void* pendIn) : pbegin((char*)pbeginIn), pend((char*)pendIn) { }
	template<typename Stream>
	void Unserialize(Stream& s)
	{
		s.read(pbegin, pend - pbegin);
	}

	template<typename Stream>
	void Serialize(Stream& s) const
	{
		s.write(pbegin, pend - pbegin);
	}

	const char* end() const { return pend; }
	char* end() { return pend; }
	const char* begin() const { return pbegin; }
	char* begin() { return pbegin; }

	template <unsigned int N, typename T, typename S, typename D>
	explicit CFlatData(prevector<N, T, S, D> &v)
	{
		pbegin = (char*)v.data();
		pend = (char*)(v.data() + v.size());
	}

	template <class T, class TAl>
	explicit CFlatData(std::vector<T, TAl> &v)
	{
		pbegin = (char*)v.data();
		pend = (char*)(v.data() + v.size());
	}
protected:
	char* pbegin;
	char* pend;
};

template<typename I>
class CVarInt
{
public:
	CVarInt(I& nIn) : n(nIn) { }

	template<typename Stream>
	void Unserialize(Stream& s) {
		n = ReadVarInt<Stream, I>(s);
	}

	template<typename Stream>
	void Serialize(Stream &s) const {
		WriteVarInt<Stream, I>(s, n);
	}
protected:
	I & n;
};

class CCompactSize
{
public:
	CCompactSize(uint64_t& nIn) : n(nIn) { }

	template<typename Stream>
	void Unserialize(Stream& s) {
		n = ReadCompactSizes<Stream>(s);
	}

	template<typename Stream>
	void Serialize(Stream &s) const {
		WriteCompactSizes<Stream>(s, n);
	}

protected:
	uint64_t & n;
};

template<size_t Limit>
class LimitedString
{
public:
	LimitedString(std::string& _string) : string(_string) {}

	template<typename Stream>
	void Serialize(Stream& s) const
	{
		WriteCompactSizes(s, string.size());
		if (!string.empty())
			s.write((char*)string.data(), string.size());
	}
	template<typename Stream>
	void Unserialize(Stream& s)
	{
		size_t size = ReadCompactSizes(s);
		if (size > Limit) {
			throw std::ios_base::fail("String length limit exceeded");
		}
		string.resize(size);
		if (size != 0)
			s.read((char*)string.data(), size);
	}

protected:
	std::string& string;
};

template<typename I>
CVarInt<I> WrapVarInt(I& n) { return CVarInt<I>(n); }

template<typename Stream, typename T> void Unserialize(Stream& os, std::unique_ptr<const T>& p);
template<typename Stream, typename T> void Serialize(Stream& os, const std::unique_ptr<const T>& p);

template<typename Stream, typename T> void Unserialize(Stream& os, std::shared_ptr<const T>& p);
template<typename Stream, typename T> void Serialize(Stream& os, const std::shared_ptr<const T>& p);

template<typename Stream, typename K, typename Pred, typename A> void Serialize(Stream& os, const std::set<K, Pred, A>& m);
template<typename Stream, typename K, typename Pred, typename A> void Unserialize(Stream& is, std::set<K, Pred, A>& m);

template<typename Stream, typename K, typename T, typename Pred, typename A> void Serialize(Stream& os, const std::map<K, T, Pred, A>& m);
template<typename Stream, typename K, typename T, typename Pred, typename A> void Unserialize(Stream& is, std::map<K, T, Pred, A>& m);

template<typename Stream, typename K, typename T> void Serialize(Stream& os, const std::pair<K, T>& item);
template<typename Stream, typename K, typename T> void Unserialize(Stream& is, std::pair<K, T>& item);

template<typename Stream, typename T, typename A> void Serialize_implement(Stream& os, const std::vector<T, A>& v, const unsigned char&);
template<typename Stream, typename T, typename A, typename V> void Serialize_implement(Stream& os, const std::vector<T, A>& v, const V&);
template<typename Stream, typename T, typename A> inline void Serialize(Stream& os, const std::vector<T, A>& v);
template<typename Stream, typename T, typename A> void Unserialize_implement(Stream& is, std::vector<T, A>& v, const unsigned char&);
template<typename Stream, typename T, typename A, typename V> void Unserialize_implement(Stream& is, std::vector<T, A>& v, const V&);
template<typename Stream, typename T, typename A> inline void Unserialize(Stream& is, std::vector<T, A>& v);


template<typename Stream, unsigned int N, typename T> void Serialize_implement(Stream& os, const prevector<N, T>& v, const unsigned char&);
template<typename Stream, unsigned int N, typename T, typename V> void Serialize_implement(Stream& os, const prevector<N, T>& v, const V&);
template<typename Stream, unsigned int N, typename T> inline void Serialize(Stream& os, const prevector<N, T>& v);
template<typename Stream, unsigned int N, typename T> void Unserialize_implement(Stream& is, prevector<N, T>& v, const unsigned char&);
template<typename Stream, unsigned int N, typename T, typename V> void Unserialize_implement(Stream& is, prevector<N, T>& v, const V&);
template<typename Stream, unsigned int N, typename T> inline void Unserialize(Stream& is, prevector<N, T>& v);

template<typename Stream, typename C> void Serialize(Stream& os, const std::basic_string<C>& str);
template<typename Stream, typename C> void Unserialize(Stream& is, std::basic_string<C>& str);


template<typename Stream, typename T>
inline void Unserialize(Stream& is, T& a)
{
	a.Unserialize(is);
}
template<typename Stream, typename T>
inline void Serialize(Stream& os, const T& a)
{
	a.Serialize(os);
}


template<typename Stream, typename T>
void Unserialize(Stream& is, std::unique_ptr<const T>& p)
{
	p.reset(new T(deserialize, is));
}
template<typename Stream, typename T> void
Serialize(Stream& os, const std::unique_ptr<const T>& p)
{
	Serialize(os, *p);
}


template<typename Stream, typename K, typename Pred, typename A>
void Unserialize(Stream& is, std::set<K, Pred, A>& m)
{
	m.clear();
	unsigned int nSize = ReadCompactSizes(is);
	typename std::set<K, Pred, A>::iterator it = m.begin();
	for (unsigned int i = 0; i < nSize; i++)
	{
		K key;
		Unserialize(is, key);
		it = m.insert(it, key);
	}
}
template<typename Stream, typename K, typename Pred, typename A>
void Serialize(Stream& os, const std::set<K, Pred, A>& m)
{
	WriteCompactSizes(os, m.size());
	for (typename std::set<K, Pred, A>::const_iterator it = m.begin(); it != m.end(); ++it)
		Serialize(os, (*it));
}

template<typename Stream, typename K, typename T>
void Unserialize(Stream& is, std::pair<K, T>& item)
{
	Unserialize(is, item.first);
	Unserialize(is, item.second);
}
template<typename Stream, typename K, typename T>
void Serialize(Stream& os, const std::pair<K, T>& item)
{
	Serialize(os, item.first);
	Serialize(os, item.second);
}

template<typename Stream, typename T, typename A, typename V>
void Serialize_implement(Stream& os, const std::vector<T, A>& v, const V&)
{
	WriteCompactSizes(os, v.size());
	for (typename std::vector<T, A>::const_iterator vi = v.begin(); vi != v.end(); ++vi)
		::Serialize(os, (*vi));
}
template<typename Stream, typename T, typename A>
void Serialize_implement(Stream& os, const std::vector<T, A>& v, const unsigned char&)
{
	WriteCompactSizes(os, v.size());
	if (!v.empty())
		os.write((char*)v.data(), v.size() * sizeof(T));
}
template<typename Stream, typename T, typename A>
inline void Serialize(Stream& os, const std::vector<T, A>& v)
{
	Serialize_impl(os, v, T());
}
template<typename Stream, unsigned int N, typename T, typename V>
void Unserialize_implement(Stream& is, prevector<N, T>& v, const V&)
{
	v.clear();
	unsigned int nSize = ReadCompactSizes(is);
	unsigned int i = 0;
	unsigned int nMid = 0;
	while (nMid < nSize)
	{
		nMid += 5000000 / sizeof(T);
		if (nMid > nSize)
			nMid = nSize;
		v.resize(nMid);
		for (; i < nMid; i++)
			Unserialize(is, v[i]);
	}
}
template<typename Stream, unsigned int N, typename T>
void Unserialize_implement(Stream& is, prevector<N, T>& v, const unsigned char&)
{
	v.clear();
	unsigned int nSize = ReadCompactSizes(is);
	unsigned int i = 0;
	while (i < nSize)
	{
		unsigned int blk = std::min(nSize - i, (unsigned int)(1 + 4999999 / sizeof(T)));
		v.resize(i + blk);
		is.read((char*)&v[i], blk * sizeof(T));
		i += blk;
	}
}

template<typename Stream, unsigned int N, typename T>
inline void Unserialize(Stream& is, prevector<N, T>& v)
{
	Unserialize_impl(is, v, T());
}
template<typename Stream, unsigned int N, typename T, typename V>
void Serialize_implement(Stream& os, const prevector<N, T>& v, const V&)
{
	WriteCompactSizes(os, v.size());
	for (typename prevector<N, T>::const_iterator vi = v.begin(); vi != v.end(); ++vi)
		::Serialize(os, (*vi));
}
template<typename Stream, unsigned int N, typename T>
void Serialize_implement(Stream& os, const prevector<N, T>& v, const unsigned char&)
{
	WriteCompactSizes(os, v.size());
	if (!v.empty())
		os.write((char*)v.data(), v.size() * sizeof(T));
}
template<typename Stream, unsigned int N, typename T>
inline void Serialize(Stream& os, const prevector<N, T>& v)
{
	Serialize_impl(os, v, T());
}

template<typename Stream, typename T, typename A, typename V>
void Unserialize_implement(Stream& is, std::vector<T, A>& v, const V&)
{
	v.clear();
	unsigned int nSize = ReadCompactSizes(is);
	unsigned int i = 0;
	unsigned int nMid = 0;
	while (nMid < nSize)
	{
		nMid += 50000000 / sizeof(T);
		if (nMid > nSize)
			nMid = nSize;
		v.resize(nMid);
		for (; i < nMid; i++)
			Unserialize(is, v[i]);
	}
}
template<typename Stream, typename T, typename A>
void Unserialize_implement(Stream& is, std::vector<T, A>& v, const unsigned char&)
{
	v.clear();
	unsigned int nSize = ReadCompactSizes(is);
	unsigned int i = 0;
	while (i < nSize)
	{
		unsigned int blk = std::min(nSize - i, (unsigned int)(1 + 4999999 / sizeof(T)));
		v.resize(i + blk);
		is.read((char*)&v[i], blk * sizeof(T));
		i += blk;
	}
}
template<typename Stream, typename T, typename A>
inline void Unserialize(Stream& is, std::vector<T, A>& v)
{
	Unserialize_impl(is, v, T());
}
template<typename Stream, typename T>
void Unserialize(Stream& is, std::shared_ptr<const T>& p)
{
	p = std::make_shared<const T>(deserialize, is);
}
template<typename Stream, typename T> void
Serialize(Stream& os, const std::shared_ptr<const T>& p)
{
	Serialize(os, *p);
}
template<typename Stream, typename C>
void Unserialize(Stream& is, std::basic_string<C>& str)
{
	unsigned int nSize = ReadCompactSizes(is);
	str.resize(nSize);
	if (nSize != 0)
		is.read((char*)str.data(), nSize * sizeof(C));
}
template<typename Stream, typename C>
void Serialize(Stream& os, const std::basic_string<C>& str)
{
	WriteCompactSizes(os, str.size());
	if (!str.empty())
		os.write((char*)str.data(), str.size() * sizeof(C));
}
template<typename Stream, typename K, typename T, typename Pred, typename A>
void Unserialize(Stream& is, std::map<K, T, Pred, A>& m)
{
	m.clear();
	unsigned int nSize = ReadCompactSizes(is);
	typename std::map<K, T, Pred, A>::iterator mi = m.begin();
	for (unsigned int i = 0; i < nSize; i++)
	{
		std::pair<K, T> item;
		Unserialize(is, item);
		mi = m.insert(mi, item);
	}
}
template<typename Stream, typename K, typename T, typename Pred, typename A>
void Serialize(Stream& os, const std::map<K, T, Pred, A>& m)
{
	WriteCompactSizes(os, m.size());
	for (typename std::map<K, T, Pred, A>::const_iterator mi = m.begin(); mi != m.end(); ++mi)
		Serialize(os, (*mi));
}
struct CSerActionUnserialize
{
	constexpr bool ForRead() const { return true; }
};
template<typename Stream, typename T>
inline void SerReadWrite(Stream& s, T& obj, CSerActionUnserialize ser_action)
{
	::Unserialize(s, obj);
}
struct CSerActionSerialize
{
	constexpr bool ForRead() const { return false; }
};

template<typename Stream, typename T>
inline void SerReadWrite(Stream& s, const T& obj, CSerActionSerialize ser_action)
{
	::Serialize(s, obj);
}
#define ADD_SERIALIZE_METHODS                                         \
    template<typename Stream>                                         \
    void Serialize(Stream& s) const {                                 \
        NCONST_PTR(this)->SerializationOp(s, CSerActionSerialize());  \
    }                                                                 \
    template<typename Stream>                                         \
    void Unserialize(Stream& s) {                                     \
        SerializationOp(s, CSerActionUnserialize());                  \
    }
class CSizeComputer
{
public:
	CSizeComputer(int nTypeIn, int nVersionIn) : nSize(0), nType(nTypeIn), nVersion(nVersionIn) {}

	int GetType() const { return nType; }
	int GetVersion() const { return nVersion; }

	size_t size() const {
		return nSize;
	}

	template<typename T>
	CSizeComputer& operator<<(const T& obj)
	{
		::Serialize(*this, obj);
		return (*this);
	}
	void seek(size_t _nSize)
	{
		this->nSize += _nSize;
	}

	void write(const char *psz, size_t _nSize)
	{
		this->nSize += _nSize;
	}
protected:
	size_t nSize;

	const int nType;
	const int nVersion;
};

template <typename T>
size_t GetSerialSize(const T& t, int nType, int nVersion = 0)
{
	return (CSizeComputer(nType, nVersion) << t).size();
}

template <typename S, typename T>
size_t GetSerialSize(const S& s, const T& t)
{
	return (CSizeComputer(s.GetType(), s.GetVersion()) << t).size();
}

template<typename I>
inline unsigned int GetSizeOfVarInt(I n)
{
	int nRet = 0;
	while (true) {
		nRet++;
		if (n <= 0x7F)
			break;
		n = (n >> 7) - 1;
	}
	return nRet;
}

template<typename I>
inline void WriteVariableInt(CSizeComputer &s, I n)
{
	s.seek(GetSizeOfVarInt<I>(n));
}

inline unsigned int GetSizeOfCompactSize(uint64_t nSize)
{
	if (nSize < 253)             return sizeof(unsigned char);
	else if (nSize <= std::numeric_limits<unsigned short>::max()) return sizeof(unsigned char) + sizeof(unsigned short);
	else if (nSize <= std::numeric_limits<unsigned int>::max())  return sizeof(unsigned char) + sizeof(unsigned int);
	else                         return sizeof(unsigned char) + sizeof(uint64_t);
}

inline void WriteCompactSize(CSizeComputer &s, uint64_t nSize)
{
	s.seek(GetSizeOfCompactSize(nSize));
}

template<typename Stream>
inline void UnserializeMany(Stream& s)
{
}

template<typename Stream, typename Arg>
inline void UnserializeMany(Stream& s, Arg& arg)
{
	::Unserialize(s, arg);
}

template<typename Stream, typename Arg, typename... Args>
inline void UnserializeMany(Stream& s, Arg& arg, Args&... args)
{
	::Unserialize(s, arg);
	::UnserializeMany(s, args...);
}

template<typename Stream>
void SerializeMany(Stream& s)
{
}

template<typename Stream, typename Arg>
void SerializeMany(Stream& s, Arg&& arg)
{
	::Serialize(s, std::forward<Arg>(arg));
}

template<typename Stream, typename Arg, typename... Args>
void SerializeMany(Stream& s, Arg&& arg, Args&&... args)
{
	::Serialize(s, std::forward<Arg>(arg));
	::SerializeMany(s, std::forward<Args>(args)...);
}

template<typename Stream, typename... Args>
inline void SerReadWriteMany(Stream& s, CSerActionUnserialize ser_action, Args&... args)
{
	::UnserializeMany(s, args...);
}

template<typename Stream, typename... Args>
inline void SerReadWriteMany(Stream& s, CSerActionSerialize ser_action, Args&&... args)
{
	::SerializeMany(s, std::forward<Args>(args)...);
}

#endif
                                                   