#ifndef WIZBLCOIN_BLOCK_ENCODINGS_H
#define WIZBLCOIN_BLOCK_ENCODINGS_H
#include "primitives/block.h"
#include <memory>
class ChkTxMemPool;
struct TxCompressor {
public:
	TxCompressor(ChkTxRef& txIn) : tx(txIn) {}
	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action) {
		READNWRITE(tx);
	}
	ADD_SERIALIZE_METHODS;
private:
	ChkTxRef & tx;
};
class BlockTxsRequest {
public:
	ADD_SERIALIZE_METHODS;
public:
	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& stream, Operation ser_action) {
		uint64_t indexes_size = (uint64_t)indexes.size();
		if (ser_action.ForRead()) {
			size_t i = 0;
			uint16_t offset = 0;
			for (size_t j = 0; j < indexes.size(); j++) {
				if (uint64_t(indexes[j]) + uint64_t(offset) > std::numeric_limits<uint16_t>::max())
					throw std::ios_base::fail("indexes overflowed 16 bits");
				indexes[j] = indexes[j] + offset;
				offset = indexes[j] + 1;
			}
		}
		else {
			for (size_t i = 0; i < indexes.size(); i++) {
				uint64_t index = indexes[i] - (i == 0 ? 0 : (indexes[i - 1] + 1));
				READNWRITE(COMPACTSIZE(index));
			}
		}
	}

public:
	uint256 blockhash;
	std::vector<uint16_t> indexes;
};

class BlockTxs {
public:
	BlockTxs() {}
	BlockTxs(const BlockTxsRequest& req) :
		blockhash(req.blockhash), txn(req.indexes.size()) {}
public:
	ADD_SERIALIZE_METHODS;
	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action) {
		uint64_t txn_size = (uint64_t)txn.size();
		if (ser_action.ForRead()) {
			size_t i = 0;
			while (txn.size() < txn_size) {
				txn.resize(std::min((uint64_t)(1000 + txn.size()), txn_size));
				for (; i < txn.size(); i++)
					READNWRITE(REF(TxCompressor(txn[i])));
			}
		}
		else {
			for (size_t i = 0; i < txn.size(); i++)
				READNWRITE(REF(TxCompressor(txn[i])));
		}
	}
public:
	uint256 blockhash;
	std::vector<ChkTxRef> txn;
};
struct PrefilledTx {
public:
	ADD_SERIALIZE_METHODS;
	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action) {
		uint64_t index = index;
		READNWRITE(COMPACTSIZE(index));
		if (index > std::numeric_limits<uint16_t>::max())
			throw std::ios_base::fail("index overflowed 16-bits");
		index = index;
		READNWRITE(REF(TxCompressor(tx)));
	}
public:
	uint16_t index;
	ChkTxRef tx;
};

typedef enum ReadStatus_t
{
	READ_STATUS_OK,
	READ_STATUS_INVALID,
	READ_STATUS_FAILED,
	READ_STATUS_CHECKBLOCK_FAILED,

} ReadStatus;

class ChkBlockHeaderAndShortTxIDs {
private:
	void FillShortTxIDSelector() const;
public:
	ChkBlockHeaderAndShortTxIDs() {}
	ChkBlockHeaderAndShortTxIDs(const ChkBlock& block, bool fUseWTXID);
	uint64_t GetShortID(const uint256& txhash) const;
	size_t BlockTxCount() const { return shorttxids.size() + prefilledtxn.size(); }
	ADD_SERIALIZE_METHODS;
	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action) {
		READNWRITE(header);
		READNWRITE(nonce);
		uint64_t shorttxids_size = (uint64_t)shorttxids.size();
		READNWRITE(COMPACTSIZE(shorttxids_size));
		if (ser_action.ForRead()) {
			size_t i = 0;
			while (shorttxids.size() < shorttxids_size) {
				shorttxids.resize(std::min((uint64_t)(1000 + shorttxids.size()), shorttxids_size));
				for (; i < shorttxids.size(); i++) {
					uint32_t lsb = 0; uint16_t msb = 0;
					READNWRITE(lsb);
					READNWRITE(msb);
					shorttxids[i] = (uint64_t(msb) << 32) | uint64_t(lsb);
					static_assert(SHORTTXIDS_LENGTH == 6, "shorttxids serialization assumes 6-byte shorttxids");
				}
			}
		}
		else {
			for (size_t i = 0; i < shorttxids.size(); i++) {
				uint32_t lsb = shorttxids[i] & 0xffffffff;
				uint16_t msb = (shorttxids[i] >> 32) & 0xffff;
				READNWRITE(lsb);
				READNWRITE(msb);
			}
		}
		READNWRITE(prefilledtxn);
		if (ser_action.ForRead())
			FillShortTxIDSelector();
	}
public:
	ChkBlockHeader header;
protected:
	std::vector<PrefilledTx> prefilledtxn;
	std::vector<uint64_t> shorttxids;
private:
	static const int SHORTTXIDS_LENGTH = 6;
	uint64_t nonce;
	mutable uint64_t shorttxidk0, shorttxidk1;
	friend class PartiallyDownloadedBlock;
};
class PartiallyDownloadedBlock {
public:
	PartiallyDownloadedBlock(ChkTxMemPool* poolIn) : pool(poolIn) {}
	ReadStatus FillBlock(ChkBlock& block, const std::vector<ChkTxRef>& vtx_missing);
	bool IsTxAvailable(size_t index) const;
	ReadStatus InitData(const ChkBlockHeaderAndShortTxIDs& cmpctblock, const std::vector<std::pair<uint256, ChkTxRef>>& extra_txn);
public:
	ChkBlockHeader header;
protected:
	std::vector<ChkTxRef> txn_available;
	size_t prefilled_count = 0, mempool_count = 0, extra_count = 0;
	ChkTxMemPool* pool;
};

#endif
                                               