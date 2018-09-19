// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Wizblcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "feerate.h"

#include "tinyformat.h"

#include "../primitives/transaction.h"
#include "../util.h"
#include "../utilmoneystr.h"

const std::string CURRENCY_UNIT = "WBL";

//CFeeRate::CFeeRate(const CAmount& nFeePaid, CAmount nAmount)
//{
//    //wizbl : 사용안함.
//    //assert(nBytes_ <= uint64_t(std::numeric_limits<int64_t>::max()));
//    //int64_t nSize = int64_t(nBytes_);
//    //
//    //if (nSize > 0)
//    //    nSatoshisPerK = nFeePaid * 1000 / nSize;
//    //else
//    //    nSatoshisPerK = 0;
//}

//CAmount CFeeRate::GetFeePerAmount(const CAmount nAmount) const
//{
//    const CAmount nSatoshis = (0 > nAmount) ? nAmount * -1 : nAmount;
//    assert(nSatoshis <= CAmount(std::numeric_limits<CAmount>::max()));
//    const CAmount nOverValue = (std::numeric_limits<CAmount>::max() / nFeeRate);
//
//    CAmount nFee = 0;
//    //CAmount 범위 오버 할수 있어서.(기존 로직에 실수값 사용을 안해서 정수 연산으로 구현)
//    if (nOverValue < nSatoshis)
//    {
//        nFee = ((nSatoshis / 100) * nFeeRate) / 100;
//    }
//    else
//    {
//        nFee = nSatoshis * nFeeRate / 10000;
//    }
//
//    if (nFee == 0 && nSatoshis != 0) {
//        nFee = CAmount(1);
//    }
//
//    return nFee;
//}
//
//CAmount CFeeRate::GetFeePerK(const size_t nBytes, const CAmount nAmount) const
//{
//    const CAmount nFee = GetFeePerAmount(nAmount);
//    const CAmount nOverValue = (std::numeric_limits<CAmount>::max() / nBytes);
//    if (nOverValue < nFee)
//    {
//        return nFee / 1000 * nBytes;
//    }
//    return nFee * nBytes / 1000;
//}
//
//CAmount CFeeRate::GetFeePerK(const CTransaction* ptx) const
//{
//    if(nullptr == ptx)
//        return 0;
//    return GetFeePerK(ptx->GetTotalSize(), ptx->GetValueOut());
//}
//
//std::string CFeeRate::ToString() const
//{
//    return strprintf("%d.%08d %s/kB", nSatoshisPerK / COIN, nSatoshisPerK % COIN, CURRENCY_UNIT);
//}


CFeeRate::CFeeRate(const CAmount& nFeePaid, size_t nBytes_)
{
    assert(nBytes_ <= uint64_t(std::numeric_limits<int64_t>::max()));
    int64_t nSize = int64_t(nBytes_);

    if (nSize > 0)
        nSatoshisPerK = nFeePaid * 1000 / nSize;
    else
        nSatoshisPerK = 0;
}

//CAmount CFeeRate::GetFee(size_t nBytes_) const
//{
//	if(WBL_TRANSACTION_FEE_FREE)
//		return 0;
//
//    assert(nBytes_ <= uint64_t(std::numeric_limits<int64_t>::max()));
//    int64_t nSize = int64_t(nBytes_);
//
//    CAmount nFee = nSatoshisPerK * nSize / 1000;
//
//    if (nFee == 0 && nSize != 0) {
//        if (nSatoshisPerK > 0)
//            nFee = CAmount(1);
//        if (nSatoshisPerK < 0)
//            nFee = CAmount(-1);
//    }
//
//    return nFee;
//}

double CFeeRate::GetWizblFeeRate()
{
    static const double fFeeRate = (double)DEFAULT_TRANSACTION_FEE_NUMERATOR / DEFAULT_TRANSACTION_FEE_DENOMINATOR;
    return fFeeRate;
}

CAmount CFeeRate::GetFeePerAmount(const CAmount nAmount) const
{
    static const CAmount ntfDenominator = DEFAULT_TRANSACTION_FEE_DENOMINATOR;
    static const CAmount ntfNumerator = DEFAULT_TRANSACTION_FEE_NUMERATOR;

    //static const CAmount nOverflowCheck(std::numeric_limits<CAmount>::max() / ntfDenominator);
    const CAmount nSatoshis = (0 > nAmount) ? -nAmount : nAmount;
    assert(nSatoshis <= CAmount(std::numeric_limits<CAmount>::max()));

    CAmount nFee = 0;
    // 온전한 수수료 0.13% 적용 관련 수정.
    nFee = (nSatoshis / ntfDenominator) * ntfNumerator;
    //if (nOverflowCheck < nSatoshis)
    //    nFee = (nSatoshis / ntfDenominator) * ntfNumerator;
    //else
    //    nFee = (nSatoshis * ntfNumerator) / ntfDenominator;
    
    if (nSatoshisPerK < 0)
        nFee *= -1;

    if (0 == nFee && 0 != nSatoshis) {
        if (nSatoshisPerK > 0)
            nFee = CAmount(1);
        if (nSatoshisPerK < 0)
            nFee = CAmount(-1);
    }

    //LogPrintfd("nAmount[%s] => nFee[%s]", FormatMoney(nAmount), FormatMoney(nFee));

    return nFee;
}

//CAmount CFeeRate::GetMinFee(const std::vector<CTxOut>* pvout)
//{
//    LogPrintfd("IN");
//    if (nullptr == pvout)
//        return 0;
//    CAmount nFeeOut = 0;
//    for (const CTxOut& tx_out : *pvout) {
//        nFeeOut += GetFeePerAmount(tx_out.nValue);
//        if (!MoneyRange(tx_out.nValue) || !MoneyRange(nFeeOut))
//            throw std::runtime_error(std::string(__func__) + ": value out of range");
//    }
//    return nFeeOut;
//}
//CAmount CFeeRate::GetMaxFee(const std::vector<CTxOut>* pvout)
//{
//    LogPrintfd("IN");
//    if (nullptr == pvout)
//        return 0;
//    CAmount nValueOut = 0;
//    for (const CTxOut& tx_out : *pvout) {
//        nValueOut += tx_out.nValue;
//        if (!MoneyRange(tx_out.nValue) || !MoneyRange(nValueOut))
//            throw std::runtime_error(std::string(__func__) + ": value out of range");
//    }
//    return GetFeePerAmount(nValueOut);
//}

CAmount CFeeRate::GetDustThreshold()
{
    //static const double dDustThreshold = 1 / GetWizblFeeRate();
    //static CAmount nDustThreshold = (1 > ((CAmount)dDustThreshold * GetWizblFeeRate())) ? dDustThreshold + 1 : dDustThreshold;
    //return nDustThreshold;

    // 온전한 수수료 0.13% 적용 관련 수정.
    return DEFAULT_TRANSACTION_REQUEST_MINIMUM_UNIT;
}

std::string CFeeRate::ToString() const
{
    return strprintf("%d.%08d %s/kB", nSatoshisPerK / COIN, nSatoshisPerK % COIN, CURRENCY_UNIT);
}
