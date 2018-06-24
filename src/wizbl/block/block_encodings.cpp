#include "block_encodings.h"

#include <unordered_map>
#include "validation.h"
#include "util.h"
#include "txmempool.h"
#include "streams.h"
#include "random.h"
#include "hash.h"
#include "consensus/validation.h"
#include "consensus/consensus.h"
#include "chainparams.h"



ReadStatus PartiallyDownloadedBlock::FillBlock(CBlock& block, const std::vector<CTransactionRef>& vtx_missing) {
	assert(!header.IsNull());
	uint256 hash = header.GetHash();
	block = header;
	block.vtx.resize(txn_available.size());

	size_t tx_missing_offset = 0;
	for (size_t i = 0; i < txn_available.size(); i++) {
		if (!txn_available[i]) {
			if (vtx_missing.size() <= tx_missing_offset)
				return READ_STATUS_INVALID;
			block.vtx[i] = vtx_missing[tx_missing_offset++];
		}
		else
			block.vtx[i] = std::move(txn_available[i]);
	}

	header.SetNull();
	txn_available.clear();

	if (vtx_missing.size() != tx_missing_offset)
		return READ_STATUS_INVALID;

	CValidationState state;
	if (!CheckBlock(block, state, Params().GetConsensus())) {
		if (state.CorruptionPossible())
			return READ_STATUS_FAILED;
		return READ_STATUS_CHECKBLOCK_FAILED;
	}

	LogPrint(BCLog::CMPCTBLOCK, "Successfully reconstructed block %s with %lu txn prefilled, %lu txn from mempool (incl at least %lu from extra pool) and %lu txn requested\n", hash.ToString(), prefilled_count, mempool_count, extra_count, vtx_missing.size());
	if (vtx_missing.size() < 5) {
		for (const auto& tx : vtx_missing) {
			LogPrint(BCLog::CMPCTBLOCK, "Reconstructed block %s required tx %s\n", hash.ToString(), tx->GetHash().ToString());
		}
	}

	return READ_STATUS_OK;
}

uint64_t CBlockHeaderAndShortTxIDs::GetShortID(const uint256& txhash) const {
	static_assert(SHORTTXIDS_LENGTH == 6, "shorttxids calculation assumes 6-byte shorttxids");
	return SipHashUint256(shorttxidk0, shorttxidk1, txhash) & 0xffffffffffffL;
}

bool PartiallyDownloadedBlock::IsTxAvailable(size_t index) const {
	assert(!header.IsNull());
	assert(index < txn_available.size());
	return txn_available[index] != nullptr;
}


ReadStatus PartiallyDownloadedBlock::InitData(const CBlockHeaderAndShortTxIDs& cmpctblock, const std::vector<std::pair<uint256, CTransactionRef>>& extra_txn) {
	if (cmpctblock.header.IsNull() || (cmpctblock.shorttxids.empty() && cmpctblock.prefilledtxn.empty()))
		return READ_STATUS_INVALID;
	if (cmpctblock.shorttxids.size() + cmpctblock.prefilledtxn.size() > MAX_BLOCK_WEIGHT / MIN_SERIALIZABLE_TRANSACTION_WEIGHT)
		return READ_STATUS_INVALID;

	assert(header.IsNull() && txn_available.empty());
	header = cmpctblock.header;
	txn_available.resize(cmpctblock.BlockTxCount());

	int32_t lastprefilledindex = -1;
	for (size_t i = 0; i < cmpctblock.prefilledtxn.size(); i++) {
		if (cmpctblock.prefilledtxn[i].tx->IsNull())
			return READ_STATUS_INVALID;

		lastprefilledindex += cmpctblock.prefilledtxn[i].index + 1;
		if (lastprefilledindex > std::numeric_limits<uint16_t>::max())
			return READ_STATUS_INVALID;
		if ((uint32_t)lastprefilledindex > cmpctblock.shorttxids.size() + i) {
			return READ_STATUS_INVALID;
		}
		txn_available[lastprefilledindex] = cmpctblock.prefilledtxn[i].tx;
	}
	prefilled_count = cmpctblock.prefilledtxn.size();

	std::unordered_map<uint64_t, uint16_t> shorttxids(cmpctblock.shorttxids.size());
	uint16_t index_offset = 0;
	for (size_t i = 0; i < cmpctblock.shorttxids.size(); i++) {
		while (txn_available[i + index_offset])
			index_offset++;
		shorttxids[cmpctblock.shorttxids[i]] = i + index_offset;
		if (shorttxids.bucket_size(shorttxids.bucket(cmpctblock.shorttxids[i])) > 12)
			return READ_STATUS_FAILED;
	}
	if (shorttxids.size() != cmpctblock.shorttxids.size())
		return READ_STATUS_FAILED;

	std::vector<bool> have_txn(txn_available.size());
	{
		LOCK(pool->cs);
		const std::vector<std::pair<uint256, CTxMemPool::txiter> >& vTxHashes = pool->vTxHashes;
		for (size_t i = 0; i < vTxHashes.size(); i++) {
			uint64_t shortid = cmpctblock.GetShortID(vTxHashes[i].first);
			std::unordered_map<uint64_t, uint16_t>::iterator idit = shorttxids.find(shortid);
			if (idit != shorttxids.end()) {
				if (!have_txn[idit->second]) {
					txn_available[idit->second] = vTxHashes[i].second->GetSharedTx();
					have_txn[idit->second] = true;
					mempool_count++;
				}
				else {
					if (txn_available[idit->second]) {
						txn_available[idit->second].reset();
						mempool_count--;
					}
				}
			}
			if (mempool_count == shorttxids.size())
				break;
		}
	}

	for (size_t i = 0; i < extra_txn.size(); i++) {
		uint64_t shortid = cmpctblock.GetShortID(extra_txn[i].first);
		std::unordered_map<uint64_t, uint16_t>::iterator idit = shorttxids.find(shortid);
		if (idit != shorttxids.end()) {
			if (!have_txn[idit->second]) {
				txn_available[idit->second] = extra_txn[i].second;
				have_txn[idit->second] = true;
				mempool_count++;
				extra_count++;
			}
			else {
				if (txn_available[idit->second] &&
					txn_available[idit->second]->GetWitnessHash() != extra_txn[i].second->GetWitnessHash()) {
					txn_available[idit->second].reset();
					mempool_count--;
					extra_count--;
				}
			}
		}
		if (mempool_count == shorttxids.size())
			break;
	}

	LogPrint(BCLog::CMPCTBLOCK, "Initialized PartiallyDownloadedBlock for block %s using a cmpctblock of size %lu\n", cmpctblock.header.GetHash().ToString(), GetSerialSize(cmpctblock, SER_NETWORK, PROTOCOL_VERSION));

	return READ_STATUS_OK;
}

CBlockHeaderAndShortTxIDs::CBlockHeaderAndShortTxIDs(const CBlock& block, bool fUseWTXID) :
	nonce(GetRand(std::numeric_limits<uint64_t>::max())),
	shorttxids(block.vtx.size() - 1), prefilledtxn(1), header(block) {
	FillShortTxIDSelector();
	prefilledtxn[0] = { 0, block.vtx[0] };
	for (size_t i = 1; i < block.vtx.size(); i++) {
		const CTransaction& tx = *block.vtx[i];
		shorttxids[i - 1] = GetShortID(fUseWTXID ? tx.GetWitnessHash() : tx.GetHash());
	}
}

void CBlockHeaderAndShortTxIDs::FillShortTxIDSelector() const {
	CDataStream stream(SER_NETWORK, PROTOCOL_VERSION);
	stream << header << nonce;
	CSHA256 hasher;
	hasher.Write((unsigned char*)&(*stream.begin()), stream.end() - stream.begin());
	uint256 shorttxidhash;
	hasher.Finalize(shorttxidhash.begin());
	shorttxidk0 = shorttxidhash.GetUint64(0);
	shorttxidk1 = shorttxidhash.GetUint64(1);
}
                               