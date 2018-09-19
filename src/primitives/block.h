// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Wizblcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef WIZBLCOIN_PRIMITIVES_BLOCK_H
#define WIZBLCOIN_PRIMITIVES_BLOCK_H

#include "arith_uint256.h"
#include "consensus/verify.h"
#include "primitives/transaction.h"
#include "serialize.h"
#include "uint256.h"
#include "version.h"
#include <string.h>

namespace Consensus {
    struct Params;
};

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class CBlockHeader
{
public:
    static const size_t HEADER_SIZE = 4+32+32+4+4+4;  // Excluding Equihash solution//이거 안씀. BTG 꺼임.
    // header
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nHeight;
    uint32_t nReserved[7];
    uint32_t nTime;
    //uint32_t nBits;
    //uint256 nNonce;
    std::vector<unsigned char> nSolution;  // Equihash solution.
    uint16_t iGenerateID;           // 블럭 생성 노드 ID
    uint16_t iVerificationLength;   // 블럭 검증 최소 개수
                                    // 3f+1, f=패밀리그룹에서 장애or공격 발생시 정상운영에 감당가능한 불량노드,
                                    // 초기에는 총 4대로 설정하고 차후 추가 가능하게 업데이트 예정.
                                    // 총 4대 중에 3대의 서명이 있으면 검증된것으로 본다. (패밀리그룹 노드수 / 3 * 2 + 1)
    std::vector<unsigned char> sig; // 각 노드의 블럭 해시 전자서명  //WIZBL_SIGN_BYTES*iVerificationLength

    CBlockHeader()
    {
        SetNull();
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        //bool new_format = false;//!(s.GetVersion() & SERIALIZE_BLOCK_LEGACY);
        READWRITE(this->nVersion);
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        //if (new_format) {
        //    READWRITE(nHeight);
        //    for(size_t i = 0; i < (sizeof(nReserved) / sizeof(nReserved[0])); i++) {
        //        READWRITE(nReserved[i]);
        //    }
        //}
        READWRITE(nTime);
        //READWRITE(nBits);
        //if (new_format) {
        //    READWRITE(nNonce);
        //    READWRITE(nSolution);
        //} else {
        //    uint32_t legacy_nonce = (uint32_t)nNonce.GetUint64(0);
        //    READWRITE(legacy_nonce);
        //    nNonce = ArithToUint256(arith_uint256(legacy_nonce));
        //}
        READWRITE(iGenerateID);
        READWRITE(iVerificationLength);
        sig.resize(WIZBL_SIGN_BYTES*iVerificationLength);
        if ( !(s.GetType() & SER_GETHASH) )
        {
            for (size_t i = 0; i < sig.size(); i++) {
                READWRITE(sig[i]);
            }
        }
    }

    void SetNull()
    {
        nVersion = 0;
        hashPrevBlock.SetNull();
        hashMerkleRoot.SetNull();
        nHeight = 0;
        memset(nReserved, 0, sizeof(nReserved));
        nTime = 0;
        //nBits = 0;
        //nNonce.SetNull();
        nSolution.clear();
        iGenerateID = 0;
        iVerificationLength = 0;
        sig.resize(WIZBL_SIGN_BYTES * iVerificationLength);
    }

    bool IsNull() const
    {
        //return (nBits == 0);
        return sig.size() != (WIZBL_SIGN_BYTES*iVerificationLength);
    }

    uint256 GetHash() const;
    uint256 GetHash(const Consensus::Params& params) const;

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }
};


class CBlock : public CBlockHeader
{
public:
    // network and disk
    std::vector<CTransactionRef> vtx;

    // memory only
    mutable bool fChecked;

    CBlock()
    {
        SetNull();
    }

    CBlock(const CBlockHeader &header)
    {
        SetNull();
        *((CBlockHeader*)this) = header;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(*(CBlockHeader*)this);
        READWRITE(vtx);
    }

    void SetNull()
    {
        CBlockHeader::SetNull();
        vtx.clear();
        fChecked = false;
    }

    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader block;
        block.nVersion       = nVersion;
        block.hashPrevBlock  = hashPrevBlock;
        block.hashMerkleRoot = hashMerkleRoot;
        block.nHeight        = nHeight;
        memcpy(block.nReserved, nReserved, sizeof(block.nReserved));
        block.nTime          = nTime;
        //block.nBits          = nBits;
        //block.nNonce         = nNonce;
        block.nSolution      = nSolution;
        block.iGenerateID           = iGenerateID;
        block.iVerificationLength   = iVerificationLength;
        block.sig                   = sig;
        return block;
    }

    std::string ToString() const;
};

/**
 * Custom serializer for CBlockHeader that omits the nonce and solution, for use
 * as input to Equihash.
 */
//class CEquihashInput : private CBlockHeader
//{
//public:
//    CEquihashInput(const CBlockHeader &header)
//    {
//        CBlockHeader::SetNull();
//        *((CBlockHeader*)this) = header;
//    }
//
//    ADD_SERIALIZE_METHODS;
//
//    template <typename Stream, typename Operation>
//    inline void SerializationOp(Stream& s, Operation ser_action) {
//        READWRITE(this->nVersion);
//        READWRITE(hashPrevBlock);
//        READWRITE(hashMerkleRoot);
//        READWRITE(nHeight);
//        for(size_t i = 0; i < (sizeof(nReserved) / sizeof(nReserved[0])); i++) {
//            READWRITE(nReserved[i]);
//        }
//        READWRITE(nTime);
//        READWRITE(nBits);
//    }
//};

/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
struct CBlockLocator
{
    std::vector<uint256> vHave;

    CBlockLocator() {}

    CBlockLocator(const std::vector<uint256>& vHaveIn) : vHave(vHaveIn) {}

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        int nVersion = s.GetVersion();
        if (!(s.GetType() & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vHave);
    }

    void SetNull()
    {
        vHave.clear();
    }

    bool IsNull() const
    {
        return vHave.empty();
    }
};

#endif // WIZBLCOIN_PRIMITIVES_BLOCK_H
