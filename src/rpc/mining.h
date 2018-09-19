// Copyright (c) 2017 The Wizblcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef WIZBLCOIN_RPC_MINING_H
#define WIZBLCOIN_RPC_MINING_H

#include "script/script.h"

#include <univalue.h>


/** Check bounds on a command line confirm target */
unsigned int ParseConfirmTarget(const UniValue& value);

#endif
