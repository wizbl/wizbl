// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Wizblcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef WIZBLCOIN_CONSENSUS_CONSENSUS_H
#define WIZBLCOIN_CONSENSUS_CONSENSUS_H

#include "wizbl_package/define_wizbl.h"

#include <stdlib.h>
#include <stdint.h>


// BTG에서 뭔가 투덜투덜하고서 추가한 항목, 기존 BTC [max 4000000 / base 1000000] 정의에서 [base 1000000] 을 삭제하고 추가한값.
static const int WITNESS_SCALE_FACTOR = 4;

/** The maximum allowed size for a serialized block, in bytes (only for buffer size limits) */
static const unsigned int MAX_BLOCK_SERIALIZED_SIZE = WIZBL_MAX_BLOCK_SIZE;//4000000;
/** The maximum allowed weight for a block, see BIP 141 (network rule) */
static const unsigned int MAX_BLOCK_WEIGHT = WIZBL_MAX_BLOCK_SIZE * WITNESS_SCALE_FACTOR;//4000000;
/** The maximum allowed number of signature check operations in a block (network rule) */
static const int64_t MAX_BLOCK_SIGOPS_COST = 80000;
/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
static const int COINBASE_MATURITY = BLOCK_REWARD_MATURITY_HOLD;

static const size_t MIN_TRANSACTION_WEIGHT = WITNESS_SCALE_FACTOR * 60; // 60 is the lower bound for the size of a valid serialized CTransaction
static const size_t MIN_SERIALIZABLE_TRANSACTION_WEIGHT = WITNESS_SCALE_FACTOR * 10; // 10 is the lower bound for the size of a serialized CTransaction

/** Flags for nSequence and nLockTime locks */
enum {
    /* Interpret sequence numbers as relative lock-time constraints. */
    LOCKTIME_VERIFY_SEQUENCE = (1 << 0),

    /* Use GetMedianTimePast() instead of nTime for end point timestamp. */
    LOCKTIME_MEDIAN_TIME_PAST = (1 << 1),
};

#endif // WIZBLCOIN_CONSENSUS_CONSENSUS_H
