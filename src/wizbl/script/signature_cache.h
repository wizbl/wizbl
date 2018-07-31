
#ifndef WIZBLCOIN_SCRIPT_SIGNATURE_CACHE_H
#define WIZBLCOIN_SCRIPT_SIGNATURE_CACHE_H

#include "script/interpreter.h"

#include <vector>

static const unsigned int DEFAULT_MAX_SIGNATURE_CACHE_SIZE = 32;
static const int64_t MAX_MAX_SIGNATURE_CACHE_SIZE = 16384;

class CPublicKeyID;

class SignatureCacheHasher
{
public:
    template <uint8_t hash_select>
    uint32_t operator()(const uint256& key) const
    {
        uint32_t u;
        std::memcpy(&u, key.begin()+4*hash_select, 4);
        static_assert(hash_select <8, "SignatureCacheHasher only has 8 hashes available.");
        return u;
    }
};

class TxSignatureCachingChecker : public TxSignatureChecker
{
public:
    TxSignatureCachingChecker(const ChkTx* txToIn, unsigned int nInIn, const CAmount& amountIn, bool storeIn, PrecomputedTxData& txdataIn) : TxSignatureChecker(txToIn, nInIn, amountIn, txdataIn), store(storeIn) {}

    bool VerifyingSignature(const std::vector<unsigned char>& vchSig, const CPublicKeyID& vchPubKey, const uint256& sighash) const override;
private:
    bool store;

};

void InitializeSignatureCache();

#endif
                                                                                                                                                                  