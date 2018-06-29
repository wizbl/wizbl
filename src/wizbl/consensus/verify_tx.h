#ifndef WIZBLCOIN_CONSENSUS_VERIFY_TX_H
#define WIZBLCOIN_CONSENSUS_VERIFY_TX_H

#include <vector>
#include <stdint.h>

class ChkTx;
class ChkCoinsViewCache;
class ChkValidationState;
class ChkBlockIndex;

bool SeqLocks(const ChkTx &tx, int flags, std::vector<int>* prevHeights,
		const ChkBlockIndex& block);

int64_t GetTxSignatureOpCost(const ChkTx& tx, const ChkCoinsViewCache& inputs, int flags);

unsigned int GetP2SHSignatureOpCount(const ChkTx& tx, const ChkCoinsViewCache& mapInputs);

bool IsFinalTx(const ChkTx &tx, int nBlockHeight, int64_t nBlockTime);

unsigned int GetLegacySignatureOpCount(const ChkTx& tx);

std::pair<int, int64_t> CalcSeqLocks(const ChkTx &tx, int flags,
		std::vector<int>* prevHeights, const ChkBlockIndex& block);

bool EvaluateSeqLocks(const ChkBlockIndex& block,
		std::pair<int, int64_t> lockPair);

bool ChkTx(const ChkTx& tx, ChkValidationState& state,
		bool fCheckDuplicateInputs = true);

namespace Consensus {
bool ChkTxInputs(const ChkTx& tx, ChkValidationState& state,
		const ChkCoinsViewCache& inputs, int nSpendHeight);
}

#endif
                                                     