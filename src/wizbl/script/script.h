#ifndef WIZBLCOIN_SCRIPT_SCRIPT_H
#define WIZBLCOIN_SCRIPT_SCRIPT_H

#include "crypto/common.h"
#include "serialize.h"
#include <climits>
#include <stdint.h>
#include <limits>
#include <stdexcept>
#include <string>
#include <string.h>
#include <vector>
#include <assert.h>
#include "../util/pre_vector.h"

static const unsigned int LOCKTIME_THRESHOLD = 500000000; // Tue Nov  5 00:53:20 1985 UTC

static const int MAX_STACK_SIZE = 1000;

static const int MAX_SCRIPT_SIZE = 10000;

static const int MAX_PUBLIC_KEYS_PER_MULTISIG = 20;

static const int MAX_OPERATIONS_PER_SCRIPT = 201;

static const unsigned int MAX_SCRIPT_ELEMENT_SIZE = 520;

template <typename T>
std::vector<unsigned char> ToByteVector(const T& in)
{
    return std::vector<unsigned char>(in.begin(), in.end());
}

enum opertaionCodetype
{
    OPERATION_0 = 0x00,
    OPERATION_FALSE = OPERATION_0,
    OPERATION_PUSHDATA1 = 0x4c,
    OPERATION_PUSHDATA2 = 0x4d,
    OPERATION_PUSHDATA4 = 0x4e,
    OPERATION_1NEGATE = 0x4f,
    OPERATION_RESERVED = 0x50,
    OPERATION_1 = 0x51,
    OPERATION_TRUE=OPERATION_1,
    OPERATION_2 = 0x52,
    OPERATION_3 = 0x53,
    OPERATION_4 = 0x54,
    OPERATION_5 = 0x55,
    OPERATION_6 = 0x56,
    OPERATION_7 = 0x57,
    OPERATION_8 = 0x58,
    OPERATION_9 = 0x59,
    OPERATION_10 = 0x5a,
    OPERATION_11 = 0x5b,
    OPERATION_12 = 0x5c,
    OPERATION_13 = 0x5d,
    OPERATION_14 = 0x5e,
    OPERATION_15 = 0x5f,
    OPERATION_16 = 0x60,

    OPERATION_NOP = 0x61,
    OPERATION_VER = 0x62,
    OPERATION_IF = 0x63,
    OPERATION_NOTIF = 0x64,
    OPERATION_VERIF = 0x65,
    OPERATION_VERNOTIF = 0x66,
    OPERATION_ELSE = 0x67,
    OPERATION_ENDIF = 0x68,
    OPERATION_VERIFY = 0x69,
    OPERATION_RETURN = 0x6a,

    OPERATION_TOALTSTACK = 0x6b,
    OPERATION_FROMALTSTACK = 0x6c,
    OPERATION_2DROP = 0x6d,
    OPERATION_2DUP = 0x6e,
    OPERATION_3DUP = 0x6f,
    OPERATION_2OVER = 0x70,
    OPERATION_2ROT = 0x71,
    OPERATION_2SWAP = 0x72,
    OPERATION_IFDUP = 0x73,
    OPERATION_DEPTH = 0x74,
    OPERATION_DROP = 0x75,
    OPERATION_DUP = 0x76,
    OPERATION_NIP = 0x77,
    OPERATION_OVER = 0x78,
    OPERATION_PICK = 0x79,
    OPERATION_ROLL = 0x7a,
    OPERATION_ROT = 0x7b,
    OPERATION_SWAP = 0x7c,
    OPERATION_TUCK = 0x7d,

    OPERATION_CAT = 0x7e,
    OPERATION_SUBSTR = 0x7f,
    OPERATION_LEFT = 0x80,
    OPERATION_RIGHT = 0x81,
    OPERATION_SIZE = 0x82,

    OPERATION_INVERT = 0x83,
    OPERATION_AND = 0x84,
    OPERATION_OR = 0x85,
    OPERATION_XOR = 0x86,
    OPERATION_EQUAL = 0x87,
    OPERATION_EQUALVERIFY = 0x88,
    OPERATION_RESERVED1 = 0x89,
    OPERATION_RESERVED2 = 0x8a,

    OPERATION_1ADD = 0x8b,
    OPERATION_1SUB = 0x8c,
    OPERATION_2MUL = 0x8d,
    OPERATION_2DIV = 0x8e,
    OPERATION_NEGATE = 0x8f,
    OPERATION_ABS = 0x90,
    OPERATION_NOT = 0x91,
    OPERATION_0NOTEQUAL = 0x92,

    OPERATION_ADD = 0x93,
    OPERATION_SUB = 0x94,
    OPERATION_MUL = 0x95,
    OPERATION_DIV = 0x96,
    OPERATION_MOD = 0x97,
    OPERATION_LSHIFT = 0x98,
    OPERATION_RSHIFT = 0x99,

    OPERATION_BOOLAND = 0x9a,
    OPERATION_BOOLOR = 0x9b,
    OPERATION_NUMEQUAL = 0x9c,
    OPERATION_NUMEQUALVERIFY = 0x9d,
    OPERATION_NUMNOTEQUAL = 0x9e,
    OPERATION_LESSTHAN = 0x9f,
    OPERATION_GREATERTHAN = 0xa0,
    OPERATION_LESSTHANOREQUAL = 0xa1,
    OPERATION_GREATERTHANOREQUAL = 0xa2,
    OPERATION_MIN = 0xa3,
    OPERATION_MAX = 0xa4,

    OPERATION_WITHIN = 0xa5,

    OPERATION_RIPEMD160 = 0xa6,
    OPERATION_SHA1 = 0xa7,
    OPERATION_SHA256 = 0xa8,
    OPERATION_HASH160 = 0xa9,
    OPERATION_HASH256 = 0xaa,
    OPERATION_CODESEPARATOR = 0xab,
    OPERATION_CHECKSIG = 0xac,
    OPERATION_CHECKSIGVERIFY = 0xad,
    OPERATION_CHECKMULTISIG = 0xae,
    OPERATION_CHECKMULTISIGVERIFY = 0xaf,

    OPERATION_NOP1 = 0xb0,
    OPERATION_CHECKLOCKTIMEVERIFY = 0xb1,
    OPERATION_NOP2 = OPERATION_CHECKLOCKTIMEVERIFY,
    OPERATION_CHECKSEQUENCEVERIFY = 0xb2,
    OPERATION_NOP3 = OPERATION_CHECKSEQUENCEVERIFY,
    OPERATION_NOP4 = 0xb3,
    OPERATION_NOP5 = 0xb4,
    OPERATION_NOP6 = 0xb5,
    OPERATION_NOP7 = 0xb6,
    OPERATION_NOP8 = 0xb7,
    OPERATION_NOP9 = 0xb8,
    OPERATION_NOP10 = 0xb9,

    OPERATION_SMALLINTEGER = 0xfa,
    OPERATION_PUBKEYS = 0xfb,
    OPERATION_PUBKEYHASH = 0xfd,
    OPERATION_PUBKEY = 0xfe,

    OPERATION_INVALIDOPCODE = 0xff,
};

static const unsigned int MAX_OPERATION_CODE = OPERATION_NOP10;

const char* GetOperationName(opertaionCodetype opcode);

class scriptnum_error : public std::runtime_error
{
public:
    explicit scriptnum_error(const std::string& str) : std::runtime_error(str) {}
};

class CScriptNum
{
public:

    explicit CScriptNum(const int64_t& n)
    {
        m_value = n;
    }

    static const size_t nDefaultMaxNumSize = 4;

    explicit CScriptNum(const std::vector<unsigned char>& vch, bool requireMinimal,
                        const size_t nMaximumSize = nDefaultMaxNumSize)
    {
        if (vch.size() > nMaximumSize) {
            throw scriptnum_error("script number overflow");
        }
        if (requireMinimal && vch.size() > 0) {
            if ((vch.back() & 0x7f) == 0) {
                if (vch.size() <= 1 || (vch[vch.size() - 2] & 0x80) == 0) {
                    throw scriptnum_error("non-minimally encoded script number");
                }
            }
        }
        m_value = set_vch(vch);
    }

    inline bool operator> (const int64_t& rhs) const    { return m_value >  rhs; }
    inline bool operator>=(const int64_t& rhs) const    { return m_value >= rhs; }
    inline bool operator< (const int64_t& rhs) const    { return m_value <  rhs; }
    inline bool operator<=(const int64_t& rhs) const    { return m_value <= rhs; }
    inline bool operator!=(const int64_t& rhs) const    { return m_value != rhs; }
    inline bool operator==(const int64_t& rhs) const    { return m_value == rhs; }
    inline bool operator> (const CScriptNum& rhs) const { return operator> (rhs.m_value); }
    inline bool operator>=(const CScriptNum& rhs) const { return operator>=(rhs.m_value); }
    inline bool operator< (const CScriptNum& rhs) const { return operator< (rhs.m_value); }
    inline bool operator<=(const CScriptNum& rhs) const { return operator<=(rhs.m_value); }
    inline bool operator!=(const CScriptNum& rhs) const { return operator!=(rhs.m_value); }
    inline bool operator==(const CScriptNum& rhs) const { return operator==(rhs.m_value); }
    inline CScriptNum operator-(   const CScriptNum& rhs) const { return operator-(rhs.m_value);   }
    inline CScriptNum operator+(   const CScriptNum& rhs) const { return operator+(rhs.m_value);   }
    inline CScriptNum operator-(   const int64_t& rhs)    const { return CScriptNum(m_value - rhs);}
    inline CScriptNum operator+(   const int64_t& rhs)    const { return CScriptNum(m_value + rhs);}
    inline CScriptNum& operator-=( const CScriptNum& rhs)       { return operator-=(rhs.m_value);  }
    inline CScriptNum& operator+=( const CScriptNum& rhs)       { return operator+=(rhs.m_value);  }
    inline CScriptNum operator&(   const CScriptNum& rhs) const { return operator&(rhs.m_value);   }
    inline CScriptNum operator&(   const int64_t& rhs)    const { return CScriptNum(m_value & rhs);}
    inline CScriptNum& operator&=( const CScriptNum& rhs)       { return operator&=(rhs.m_value);  }
    inline CScriptNum operator-()  const { return CScriptNum(-m_value); }
    inline CScriptNum& operator=( const int64_t& rhs) {
        m_value = rhs;
        return *this;
    }
    inline CScriptNum& operator+=( const int64_t& rhs){
        m_value += rhs;
        return *this;
    }
    inline CScriptNum& operator-=( const int64_t& rhs){
        m_value -= rhs;
        return *this;
    }

    inline CScriptNum& operator&=( const int64_t& rhs) {
        m_value &= rhs;
        return *this;
    }
};

typedef prevector<28, unsigned char> CScriptBase;

class CScript : public CScriptBase
{
protected:
    CScript& push_int64(int64_t n) {
        if (n == -1 || (n >= 1 && n <= 16)) { push_back(n + (OPERATION_1 - 1));}
        else if (n == 0) { push_back(OPERATION_0); }
        else { *this << CScriptNum::serialize(n); }
        return *this;
    }
public:
    CScript() { }
    CScript(const_iterator pbegin, const_iterator pend) : CScriptBase(pbegin, pend) { }
    CScript(std::vector<unsigned char>::const_iterator pbegin, std::vector<unsigned char>::const_iterator pend) : CScriptBase(pbegin, pend) { }
    CScript(const unsigned char* pbegin, const unsigned char* pend) : CScriptBase(pbegin, pend) { }

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) { READNWRITE(static_cast<CScriptBase&>(*this)); }
    CScript& operator+=(const CScript& b) {
        insert(end(), b.begin(), b.end());
        return *this;
    }
    friend CScript operator+(const CScript& a, const CScript& b) {
        CScript ret = a;
        ret += b;
        return ret;
    }

    CScript(int64_t b)        { operator<<(b); }
    explicit CScript(opertaionCodetype b)     { operator<<(b); }
    explicit CScript(const CScriptNum& b) { operator<<(b); }
    explicit CScript(const std::vector<unsigned char>& b) { operator<<(b); }
    CScript& operator<<(int64_t b) { return push_int64(b); }
    CScript& operator<<(opertaionCodetype opcode) {
        if (opcode < 0 || opcode > 0xff)
            throw std::runtime_error("CScript::operator<<(): invalid opcode");
        insert(end(), (unsigned char)opcode);
        return *this;
    }

    CScript& operator<<(const CScriptNum& b){
        *this << b.getvch();
        return *this;
    }

    CScript& operator<<(const std::vector<unsigned char>& b){return *this;}

    CScript& operator<<(const CScript& b){ return *this; }

    bool GetOperator(iterator& pc, opertaionCodetype& opcodeRet, std::vector<unsigned char>& vchRet)
    {
         const_iterator pc2 = pc;
         bool fRet = GetOperator2(pc2, opcodeRet, &vchRet);
         pc = begin() + (pc2 - begin());
         return fRet;
    }

    bool GetOperator(iterator& pc, opertaionCodetype& opcodeRet)
    {
         const_iterator pc2 = pc;
         bool fRet = GetOperator2(pc2, opcodeRet, nullptr);
         pc = begin() + (pc2 - begin());
         return fRet;
    }
    bool GetOperator(const_iterator& pc, opertaionCodetype& opcodeRet, std::vector<unsigned char>& vchRet) const { return GetOperator2(pc, opcodeRet, &vchRet); }
    bool GetOperator(const_iterator& pc, opertaionCodetype& opcodeRet) const { return GetOperator2(pc, opcodeRet, nullptr);}
    bool GetOperator2(const_iterator& pc, opertaionCodetype& opcodeRet, std::vector<unsigned char>* pvchRet) const {
        return true;
    }

    static int DecodeOPERATION_N(opertaionCodetype opcode)
    {
        if (opcode == OPERATION_0)
            return 0;
        return (int)opcode - (int)(OPERATION_1 - 1);
    }
    static opertaionCodetype EncodeOPERATION_N(int n)
    {
        if (n == 0)
            return OPERATION_0;
        return (opertaionCodetype)(OPERATION_1+n-1);
    }


    unsigned int GetSignatureOperationCount(bool fAccurate) const;

    unsigned int GetSignatureOperationCount(const CScript& scriptSig) const;

    bool IsPayToHash() const;
    bool IsPayToWitnessesScriptHash() const;
    bool IsWitnessProgram(int& version, std::vector<unsigned char>& program) const;

    bool IsPushingOnly(const_iterator pc) const;
    bool IsPushingOnly() const;

    bool HasValidOpeartions() const;

    bool IsUnspendable() const
    {
        return (size() > 0 && *begin() == OPERATION_RETURN) || (size() > MAX_SCRIPT_SIZE);
    }

    void clear()
    {
        CScriptBase::clear();
        shrink_to_fit();
    }
};

struct CScriptWitnesses
{
    std::vector<std::vector<unsigned char> > stack;

    CScriptWitnesses() { }

    bool IsNull() const { return stack.empty(); }

    void SetNull() { stack.clear(); stack.shrink_to_fit(); }

    std::string ToString() const;
};

class CReserveScript
{
public:
    CScript reserveScript;
    virtual void KeepScript() {}
    CReserveScript() {}
    virtual ~CReserveScript() {}
};

#endif
                                                                                                      