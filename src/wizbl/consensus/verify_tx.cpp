#include "verify_tx.h"

#include "../block_chain.h"
#include "validation.h"
#include "consensus.h"
#include "script/interpreter.h"
#include "primitives/transaction.h"
#include "coins.h"
#include "utilmoneystr.h"

unsigned int GetP2SHSignatureOpCount(const ChkTx& tx, const ChkCoinsViewCache& inputs) {
	if (tx.IsCoinBase())
		return 0;

	unsigned int nSigOps = 0;
	for (unsigned int i = 0; i < tx.vin.size(); i++) {
		const Coin& coin = inputs.AccessCoin(tx.vin[i].prevout);
		assert(!coin.IsSpent());
		const ChkTxOut &prevout = coin.out;
		if (prevout.scriptPubKey.IsPayToScriptHash())
			nSigOps += prevout.scriptPubKey.GetSigOpCount(tx.vin[i].scriptSig);
	}
	return nSigOps;
}

unsigned int GetLegacySignatureOpCount(const ChkTx& tx) {
	unsigned int nSigOps = 0;
	for (const auto& txin : tx.vin) {
		nSigOps += txin.scriptSig.GetSigOpCount(false);
	}
	for (const auto& txout : tx.vout) {
		nSigOps += txout.scriptPubKey.GetSigOpCount(false);
	}
	return nSigOps;
}

int64_t GetTxSignatureOpCost(const ChkTx& tx, const ChkCoinsViewCache& inputs,
		int flags) {
	int64_t nSigOps = GetLegacySignatureOpCount(tx) * WITNESS_SCALE_FACTOR;

	if (tx.IsCoinBase())
		return nSigOps;

	if (flags & SCRIPT_VERIFY_P2SH) {
		nSigOps += GetP2SHSignatureOpCount(tx, inputs) * WITNESS_SCALE_FACTOR;
	}

	for (unsigned int i = 0; i < tx.vin.size(); i++) {
		const Coin& coin = inputs.AccessCoin(tx.vin[i].prevout);
		assert(!coin.IsSpent());
		const ChkTxOut &prevout = coin.out;
		nSigOps += CountWitnessSigOps(tx.vin[i].scriptSig, prevout.scriptPubKey,
				&tx.vin[i].scriptWitness, flags);
	}
	return nSigOps;
}

bool Consensus::ChkTxInputs(const ChkTx& tx, ChkValidationState& state,
		const ChkCoinsViewCache& inputs, int nSpendHeight) {
	if (!inputs.HaveInputs(tx))
		return state.Invalid(false, 0, "", "Inputs unavailable");

	CAmount nValueIn = 0;
	CAmount nFees = 0;
	for (unsigned int i = 0; i < tx.vin.size(); i++) {
		const COutPoint &prevout = tx.vin[i].prevout;
		const Coin& coin = inputs.AccessCoin(prevout);
		assert(!coin.IsSpent());

		if (coin.IsCoinBase()) {
			if (nSpendHeight - coin.nHeight < COINBASE_MATURITY)
				return state.Invalid(false, REJECT_INVALID,
						"bad-txns-premature-spend-of-coinbase",
						strprintf("tried to spend coinbase at depth %d", nSpendHeight - coin.nHeight));
			}

		nValueIn += coin.out.nValue;
		if (!MoneyRange(coin.out.nValue) || !MoneyRange(nValueIn))
			return state.DoS(100, false, REJECT_INVALID,
					"bad-txns-inputvalues-outofrange");

	}

	if (nValueIn < tx.GetValueOut())
		return state.DoS(100, false, REJECT_INVALID, "bad-txns-in-belowout",
				false,
				strprintf("value in (%s) < value out (%s)", FormatMoney(nValueIn), FormatMoney(tx.GetValueOut())));

	CAmount nTxFee = nValueIn - tx.GetValueOut();
	if (nTxFee < 0)
		return state.DoS(100, false, REJECT_INVALID, "bad-txns-fee-negative");
	nFees += nTxFee;
	if (!MoneyRange(nFees))
		return state.DoS(100, false, REJECT_INVALID, "bad-txns-fee-outofrange");
	return true;
}

bool ChkTx(const ChkTx& tx, ChkValidationState &state,
		bool fCheckDuplicateInputs) {
	if (tx.vin.empty())
		return state.DoS(10, false, REJECT_INVALID, "bad-txns-vin-empty");
	if (tx.vout.empty())
		return state.DoS(10, false, REJECT_INVALID, "bad-txns-vout-empty");
	if (::GetSerialSize(tx, SER_NETWORK,
			PROTOCOL_VERSION | SERIALIZE_TX_NO_WITNESS)
			* WITNESS_SCALE_FACTOR > MAX_BLOCK_WEIGHT)
		return state.DoS(100, false, REJECT_INVALID, "bad-txns-oversize");

	CAmount nValueOut = 0;
	for (const auto& txout : tx.vout) {
		if (txout.nValue < 0)
			return state.DoS(100, false, REJECT_INVALID,
					"bad-txns-vout-negative");
		if (txout.nValue > MAX_MONEY)
			return state.DoS(100, false, REJECT_INVALID,
					"bad-txns-vout-toolarge");
		nValueOut += txout.nValue;
		if (!MoneyRange(nValueOut))
			return state.DoS(100, false, REJECT_INVALID,
					"bad-txns-txouttotal-toolarge");
	}

	if (fCheckDuplicateInputs) {
		std::set<COutPoint> vInOutPoints;
		for (const auto& txin : tx.vin) {
			if (!vInOutPoints.insert(txin.prevout).second)
				return state.DoS(100, false, REJECT_INVALID,
						"bad-txns-inputs-duplicate");
		}
	}

	if (tx.IsCoinBase()) {
		if (tx.vin[0].scriptSig.size() < 2 || tx.vin[0].scriptSig.size() > 100)
			return state.DoS(100, false, REJECT_INVALID, "bad-cb-length");
	} else {
		for (const auto& txin : tx.vin)
			if (txin.prevout.IsNull())
				return state.DoS(10, false, REJECT_INVALID,
						"bad-txns-prevout-null");
	}

	return true;
}

bool EvaluateSeqLocks(const ChkBlockIndex& block,
		std::pair<int, int64_t> lockPair) {
	assert(block.pprev);
	int64_t nBlockTime = block.pprev->get_MedianTimePast();
	if (lockPair.first >= block.iHeight || lockPair.second >= nBlockTime)
		return false;

	return true;
}

bool SeqLocks(const ChkTx &tx, int flags, std::vector<int>* prevHeights,
		const ChkBlockIndex& block) {
	return EvaluateSeqLocks(block,
			CalcSeqLocks(tx, flags, prevHeights, block));
}

std::pair<int, int64_t> CalcSeqLocks(const ChkTx &tx, int flags,
		std::vector<int>* prevHeights, const ChkBlockIndex& block) {
	assert(prevHeights->size() == tx.vin.size());
	int nMinHeight = -1;
	int64_t nMinTime = -1;

	bool fEnforceBIP68 = static_cast<uint32_t>(tx.nVersion) >= 2
			&& flags & LOCKTIME_VERIFY_SEQUENCE;

	if (!fEnforceBIP68) {
		return std::make_pair(nMinHeight, nMinTime);
	}

	for (size_t txinIndex = 0; txinIndex < tx.vin.size(); txinIndex++) {
		const ChkTxIn& txin = tx.vin[txinIndex];

		if (txin.nSeq & ChkTxIn::SEQUENCE_LOCKTIME_DISABLE_FLAG) {
			(*prevHeights)[txinIndex] = 0;
			continue;
		}

		int nCoinHeight = (*prevHeights)[txinIndex];

		if (txin.nSeq & ChkTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG) {
			int64_t nCoinTime =
					block.GetAncestor(std::max(nCoinHeight - 1, 0))->get_MedianTimePast();
			nMinTime = std::max(nMinTime,
					nCoinTime
							+ (int64_t) ((txin.nSeq
									& ChkTxIn::SEQUENCE_LOCKTIME_MASK)
									<< ChkTxIn::SEQUENCE_LOCKTIME_GRANULARITY)
							- 1);
		} else {
			nMinHeight = std::max(nMinHeight,
					nCoinHeight
							+ (int) (txin.nSeq & ChkTxIn::SEQUENCE_LOCKTIME_MASK)
							- 1);
		}
	}

	return std::make_pair(nMinHeight, nMinTime);
}

bool IsFinalTx(const ChkTx &tx, int nBlockHeight, int64_t nBlockTime) {
	if (tx.nLockTime == 0)
		return true;
	if ((int64_t) tx.nLockTime
			< ((int64_t) tx.nLockTime < LOCKTIME_THRESHOLD ?
					(int64_t) nBlockHeight : nBlockTime))
		return true;
	for (const auto& txin : tx.vin) {
		if (!(txin.nSeq == ChkTxIn::SEQUENCE_FINAL))
			return false;
	}
	return true;
}
                                                                                                                                     