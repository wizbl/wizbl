#ifndef WIZBLCOIN_CONSENSUS_CONSENSUS_H
#define WIZBLCOIN_CONSENSUS_CONSENSUS_H

#include <stdint.h>
#include <stdlib.h>

enum {
    LOCKTIME_VERIFY_SEQUENCE = (1 << 0),
    LOCKTIME_MEDIAN_TIME_PAST = (1 << 1),
};

static const unsigned int MAX_BLOCK_WEIGHT = 4000000;
static const unsigned int MAX_BLOCK_SERIALIZED_SIZE = 4000000;
static const int COINBASE_MATURITY = 100;
static const int WITNESS_SCALE_FACTOR = 4;
static const int64_t MAX_BLOCK_SIGOPS_COST = 80000;

static const size_t MIN_SERIALIZABLE_TX_WEIGHT = WITNESS_SCALE_FACTOR * 10;
static const size_t MIN_TX_WEIGHT = WITNESS_SCALE_FACTOR * 60;

#endif
                                                                                                                                                                           