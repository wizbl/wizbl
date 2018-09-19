// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Wizblcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef WIZBLCOIN_POLICY_FEERATE_H
#define WIZBLCOIN_POLICY_FEERATE_H

#include "amount.h"
#include "serialize.h"

//#include <vector>
#include <string>

//class CTxOut;
extern const std::string CURRENCY_UNIT;

//class CTransaction;
///**
//* 수수료
//*/
//class CFeeRate
//{
//private:
//    CAmount nFeeRate = 14;//0.14% => WBL / 10000 * nFeeRate
//public:
//    CFeeRate() : {}
//    CFeeRate(const CAmount& _nSatoshisPerK) : { /*nFeeRate = ;*/ }//빌드 호환성?
//    CFeeRate(const CFeeRate& other) { /*nFeeRate = other.nFeeRate;*/ }//빌드 호환성?
//
//    /** 금액을 입력하면 수수료 반환 */ // 이름이 다른것은 기존 함수의 자료형이 컴파일러로 걸러지지 않아서, 작업용 임시의 이름을 주었다.
//    CAmount GetFeePerAmount(const CAmount nAmount) const;
//
//    /** transaction 의 byte 당 수수료 */
//    CAmount GetFeePerK(const size_t nBytes, const CAmount nAmount) const;
//    CAmount GetFeePerK(const CTransaction* ptx) const;
//    friend bool operator<(const CFeeRate& a, const CFeeRate& b) { return a.nFeeRate < b.nFeeRate; }
//    friend bool operator>(const CFeeRate& a, const CFeeRate& b) { return a.nFeeRate > b.nFeeRate; }
//    friend bool operator==(const CFeeRate& a, const CFeeRate& b) { return a.nFeeRate == b.nFeeRate; }
//    friend bool operator<=(const CFeeRate& a, const CFeeRate& b) { return a.nFeeRate <= b.nFeeRate; }
//    friend bool operator>=(const CFeeRate& a, const CFeeRate& b) { return a.nFeeRate >= b.nFeeRate; }
//    friend bool operator!=(const CFeeRate& a, const CFeeRate& b) { return a.nFeeRate != b.nFeeRate; }
//    CFeeRate& operator+=(const CFeeRate& a) { /*nFeeRate += a.nFeeRate;*/ return *this; }
//    std::string ToString() const;
//
//    ADD_SERIALIZE_METHODS;
//
//    template <typename Stream, typename Operation>
//    inline void SerializationOp(Stream& s, Operation ser_action) {
//        READWRITE(nFeeRate);
//    }
//};

/**
 * Fee rate in satoshis per kilobyte: CAmount / kB
 */
class CFeeRate
{
private:
    CAmount nSatoshisPerK; // unit is satoshis-per-1,000-bytes
public:
    /** Fee rate of 0 satoshis per kB */
    CFeeRate() : nSatoshisPerK(0) {}
    explicit CFeeRate(const CAmount& _nSatoshisPerK) : nSatoshisPerK(_nSatoshisPerK) {}
    /** Constructor for a fee rate in satoshis per kB. The size in bytes must not exceed (2^63 - 1)*/
    CFeeRate(const CAmount& nFeePaid, size_t nBytes);
    CFeeRate(const CFeeRate& other) { nSatoshisPerK = other.nSatoshisPerK; }

    //wizbl 수수료 율 반환
    double GetWizblFeeRate();

    /**
     * Return the fee in satoshis for the given size in bytes.
     */
    //CAmount GetFee(size_t nBytes) const;
    /** 금액을 입력하면 수수료 반환 */ // 이름이 다른것은 기존 함수의 자료형이 컴파일러로 걸러지지 않아서, 작업용 임시의 이름을 주었다.
    CAmount GetFeePerAmount(const CAmount nAmount) const;

    ////트랜젝션 거래당 수수료의 합을 반환(트랜젝션 거래 합으로 구하면 각각의 수수료 합보다 클 수 있다.)
    //CAmount GetMinFee(const std::vector<CTxOut>* pvout);
    ////트랜젝션 거래 합에서 수수료 구함. (GetMinFee() 보다 클 수 있다.)
    //CAmount GetMaxFee(const std::vector<CTxOut>* pvout);
    //먼지값? 반환값 보다 작은 거래는 허용하지 않는다(수수료가 없음)
    CAmount GetDustThreshold();
    /**
     * Return the fee in satoshis for a size of 1000 bytes
     */
    //CAmount GetFeePerK() const { return GetFee(1000); }
    CAmount GetFeePerK() const { return 1; } //{ return GetFeePerAmount(1 * COIN); }//wizbl : 오류 회피용
    friend bool operator<(const CFeeRate& a, const CFeeRate& b) { return a.nSatoshisPerK < b.nSatoshisPerK; }
    friend bool operator>(const CFeeRate& a, const CFeeRate& b) { return a.nSatoshisPerK > b.nSatoshisPerK; }
    friend bool operator==(const CFeeRate& a, const CFeeRate& b) { return a.nSatoshisPerK == b.nSatoshisPerK; }
    friend bool operator<=(const CFeeRate& a, const CFeeRate& b) { return a.nSatoshisPerK <= b.nSatoshisPerK; }
    friend bool operator>=(const CFeeRate& a, const CFeeRate& b) { return a.nSatoshisPerK >= b.nSatoshisPerK; }
    friend bool operator!=(const CFeeRate& a, const CFeeRate& b) { return a.nSatoshisPerK != b.nSatoshisPerK; }
    CFeeRate& operator+=(const CFeeRate& a)
    {
        nSatoshisPerK += a.nSatoshisPerK;
        return *this;
    }
    std::string ToString() const;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(nSatoshisPerK);
    }
};

#endif //  WIZBLCOIN_POLICY_FEERATE_H
