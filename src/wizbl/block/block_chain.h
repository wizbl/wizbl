
#ifndef WIZBLCOIN_CHAIN_H
#define WIZBLCOIN_CHAIN_H

#include "arith_uint256.h"
#include "pow.h"
#include "primitives/block.h"
#include "uint256.h"
#include <string.h>
#include <vector>
#include "util/tiny_format.h"

static const int64_t WIZBLCOIN_MAX_FUTURE_BLOCK_TIME = 2 * 60 * 60;

static const int64_t TIMESTAMP_WINDOW = WIZBLCOIN_MAX_FUTURE_BLOCK_TIME;

class ChkBlockFileInfo
{
public:
	CInfoBlockFile() {
		SetNull();
	}

	ADD_SERIALIZE_METHODS;

	void AddBlock(unsigned int nHeightIn, uint64_t nTimeIn) {
		if (uiBlocks == 0 || uiHeightFirst > nHeightIn)
			uiHeightFirst = nHeightIn;
		if (uiBlocks == 0 || uiTimeFirst > nTimeIn)
			uiTimeFirst = nTimeIn;
		uiBlocks++;
		if (nHeightIn > uiHeightLast)
			uiHeightLast = nHeightIn;
		if (nTimeIn > uiTimeLast)
			uiTimeLast = nTimeIn;
	}

	std::string ToString() const;

	void SetNull() {
		uiBlocks = 0;
		uiSize = 0;
		uiUndoSize = 0;
		uiHeightFirst = 0;
		uiHeightLast = 0;
		uiTimeFirst = 0;
		uiTimeLast = 0;
	}

	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action) {
		READNWRITE(VARINT(uiBlocks));
		READNWRITE(VARINT(uiSize));
		READNWRITE(VARINT(uiUndoSize));
		READNWRITE(VARINT(uiHeightFirst));
		READNWRITE(VARINT(uiHeightLast));
		READNWRITE(VARINT(uiTimeFirst));
		READNWRITE(VARINT(uiTimeLast));
	}

public:
	unsigned int uiBlocks;
	unsigned int uiSize;
	unsigned int uiUndoSize;
	unsigned int uiHeightFirst;
	unsigned int uiHeightLast;
	uint64_t uiTimeFirst;
	uint64_t uiTimeLast;
};

struct CDiskBlockPos
{
	CDiskBlockPos() {
		SetNull();
	}

	CDiskBlockPos(int nFileIn, unsigned int nPosIn) {
		nFile = nFileIn;
		nPos = nPosIn;
	}

	ADD_SERIALIZE_METHODS;

	std::string ToString() const
	{
		return strprintf("ChkBlockDiskPos(nFile=%i, nPos=%i)", nFile, nPos);
	}

	friend bool operator==(const CDiskBlockPos &a, const CDiskBlockPos &b) {
		return (a.nFile == b.nFile && a.nPos == b.nPos);
	}

	friend bool operator!=(const CDiskBlockPos &a, const CDiskBlockPos &b) {
		return !(a == b);
	}

	bool IsNull() const { return (nFile == -1); }

	void SetNull() { nFile = -1; nPos = 0; }

	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action) {
		READNWRITE(VARINT(nFile));
		READNWRITE(VARINT(nPos));
	}

	int nFile;
	unsigned int nPos;
};

enum BlockStatus : uint32_t {
	BLOCK_VALID_UNKNOWN = 0,
	BLOCK_VALID_HEADER = 1,
	BLOCK_VALID_TREE = 2,
	BLOCK_VALID_TXS = 3,
	BLOCK_VALID_CHAIN = 4,
	BLOCK_VALID_SCRIPTS = 5,
	BLOCK_VALID_MASK = BLOCK_VALID_HEADER | BLOCK_VALID_TREE | BLOCK_VALID_TXS |
	BLOCK_VALID_CHAIN | BLOCK_VALID_SCRIPTS,
	BLOCK_HAVE_DATA = 8,
	BLOCK_HAVE_UNDO = 16,
	BLOCK_HAVE_MASK = BLOCK_HAVE_DATA | BLOCK_HAVE_UNDO,
	BLOCK_FAILED_VALID = 32,
	BLOCK_FAILED_CHILD = 64,
	BLOCK_FAILED_MASK = BLOCK_FAILED_VALID | BLOCK_FAILED_CHILD,
	BLOCK_OPT_WITNESS = 128,
};

class ChkBlockIndex
{
public:
	ChkBlockIndex()
	{
		SetNull();
	}

	ChkBlockIndex(const ChkBlockHeader& block)
	{
		SetNull();

		iVersion = block.iVersion;
		uiHashMerkleRoot = block.uiHashMerkleRoot;
		iHeight = block.iHeight;
		memcpy(uiReserved, block.uiReserved, sizeof(uiReserved));
		uiTime = block.uiTime;
		uiBits = block.uiBits;
		uiNonce = block.uiNonce;
		vecuiSolution = block.vecuiSolution;
	}

	ChkBlockIndex* get_Ancestor(int height);
	const ChkBlockIndex* get_Ancestor(int height) const;

	void Skip_Build();

	bool Validity_Raise(enum BlockStatus nUpTo)
	{
		assert(!(nUpTo & ~BLOCK_VALID_MASK));
		if (uiStatus & BLOCK_FAILED_MASK)
			return false;
		if ((uiStatus & BLOCK_VALID_MASK) < nUpTo) {
			uiStatus = (uiStatus & ~BLOCK_VALID_MASK) | nUpTo;
			return true;
		}
		return false;
	}

	bool IsValid(enum BlockStatus nUpTo = BLOCK_VALID_TXS) const
	{
		assert(!(nUpTo & ~BLOCK_VALID_MASK));
		if (uiStatus & BLOCK_FAILED_MASK)
			return false;
		return ((uiStatus & BLOCK_VALID_MASK) >= nUpTo);
	}

	std::string ToString() const
	{
		return strprintf("ChkBlockIndex(pprev=%p, nHeight=%d, merkle=%s, hashBlock=%s)",
			pprev, iHeight,
			uiHashMerkleRoot.ToString(),
			get_BlockHash().ToString());
	}
	enum { nMedianTimeSpan = 11 };

	int64_t get_MedianTimePast() const
	{
		int64_t pmedian[nMedianTimeSpan];
		int64_t* pbegin = &pmedian[nMedianTimeSpan];
		int64_t* pend = &pmedian[nMedianTimeSpan];

		const ChkBlockIndex* pindex = this;
		for (int i = 0; i < nMedianTimeSpan && pindex; i++, pindex = pindex->pprev)
			*(--pbegin) = pindex->get_BlockTime();

		std::sort(pbegin, pend);
		return pbegin[(pend - pbegin) / 2];
	}

	int64_t get_BlockTimeMax() const
	{
		return (int64_t)uiTimeMax;
	}
	int64_t get_BlockTime() const
	{
		return (int64_t)uiTime;
	}

	uint256 get_BlockHash() const
	{
		return *phashBlock;
	}
	CDiskBlockPos get_UndoPos() const {
		CDiskBlockPos ret;
		if (uiStatus & BLOCK_HAVE_UNDO) {
			ret.nFile = iFile;
			ret.nPos = uiUndoPos;
		}
		return ret;
	}

	CDiskBlockPos get_BlockPos() const {
		CDiskBlockPos ret;
		if (uiStatus & BLOCK_HAVE_DATA) {
			ret.nFile = iFile;
			ret.nPos = uiDataPos;
		}
		return ret;
	}
	ChkBlockHeader get_BlockHeader() const
	{
		ChkBlockHeader block;
		block.iVersion = iVersion;
		if (pprev)
			block.hashPrevBlock = pprev->get_BlockHash();
		block.uiHashMerkleRoot = uiHashMerkleRoot;
		block.iHeight = iHeight;
		memcpy(block.uiReserved, uiReserved, sizeof(block.uiReserved));
		block.uiTime = uiTime;
		block.uiBits = uiBits;
		block.uiNonce = uiNonce;
		block.vecuiSolution = vecuiSolution;
		return block;
	}
	void SetNull()
	{
		phashBlock = nullptr;
		pprev = nullptr;
		pskip = nullptr;
		iHeight = 0;
		iFile = 0;
		uiDataPos = 0;
		uiUndoPos = 0;
		nChainWork = arith_uint256();
		uiTx = 0;
		uiChainTx = 0;
		uiStatus = 0;
		iSeqId = 0;
		uiTimeMax = 0;

		iVersion = 0;
		uiHashMerkleRoot = uint256();
		memset(uiReserved, 0, sizeof(uiReserved));
		uiTime = 0;
		uiBits = 0;
		uiNonce = uint256();
		vecuiSolution.clear();
	}

public:
	const uint256* phashBlock;
	ChkBlockIndex* pprev;
	ChkBlockIndex* pskip;
	int iHeight;
	int iFile;
	unsigned int uiDataPos;
	unsigned int uiUndoPos;
	arith_uint256 nChainWork;
	unsigned int uiTx;
	unsigned int uiChainTx;
	unsigned int uiStatus;
	int iVersion;
	uint256 uiHashMerkleRoot;
	uint32_t uiReserved[7];
	unsigned int uiTime;
	unsigned int uiBits;
	uint256 uiNonce;
	std::vector<unsigned char> vecuiSolution;
	int32_t iSeqId;
	unsigned int uiTimeMax;
};

arith_uint256 GetBlockProof(const ChkBlockIndex& block);
int64_t GetBlockProofEquivalentTime(const ChkBlockIndex& to, const ChkBlockIndex& from, const ChkBlockIndex& tip, const Consensus::Params&);
const ChkBlockIndex* LastCommonAncestor(const ChkBlockIndex* pa, const ChkBlockIndex* pb);
class CDiskBlockIndex : public ChkBlockIndex
{
public:
	ADD_SERIALIZE_METHODS;

public:
	CDiskBlockIndex() {
		uiHashPrev = uint256();
	}

	explicit CDiskBlockIndex(const ChkBlockIndex* pindex) : ChkBlockIndex(*pindex) {
		uiHashPrev = (pprev ? pprev->get_BlockHash() : uint256());
	}

public:
	std::string ToString() const
	{
		std::string str = "CDiskBlockIndex(";
		str += ChkBlockIndex::ToString();
		str += strprintf("\n                hashBlock=%s, hashPrev=%s)",
			get_BlockHash().ToString(),
			uiHashPrev.ToString());
		return str;
	}
	uint256 get_BlockHash() const
	{
		ChkBlockHeader block;
		block.iVersion = iVersion;
		block.hashPrevBlock = uiHashPrev;
		block.uiHashMerkleRoot = uiHashMerkleRoot;
		block.iHeight = iHeight;
		memcpy(block.uiReserved, uiReserved, sizeof(block.uiReserved));
		block.uiTime = uiTime;
		block.uiBits = uiBits;
		block.uiNonce = uiNonce;
		block.vecuiSolution = vecuiSolution;
		return block.GetHash();
	}

	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action) {
		int _nVersion = s.GetVersion();
		if (!(s.GetType() & SER_GETHASH))
			READNWRITE(VARINT(_nVersion));

		READNWRITE(VARINT(iHeight));
		READNWRITE(VARINT(uiStatus));
		READNWRITE(VARINT(uiTx));
		if (uiStatus & (BLOCK_HAVE_DATA | BLOCK_HAVE_UNDO))
			READNWRITE(VARINT(iFile));
		if (uiStatus & BLOCK_HAVE_DATA)
			READNWRITE(VARINT(uiDataPos));
		if (uiStatus & BLOCK_HAVE_UNDO)
			READNWRITE(VARINT(uiUndoPos));

		READNWRITE(this->iVersion);
		READNWRITE(uiHashPrev);
		READNWRITE(uiHashMerkleRoot);
		for (size_t i = 0; i < (sizeof(uiReserved) / sizeof(uiReserved[0])); i++) {
			READNWRITE(uiReserved[i]);
		}
		READNWRITE(uiTime);
		READNWRITE(uiBits);
		READNWRITE(uiNonce);
		READNWRITE(vecuiSolution);
	}

public:
	uint256 uiHashPrev;

};

class CChain
{
private:
	std::vector<ChkBlockIndex*> vChain;

public:

	ChkBlockIndex* FindEarliestAtLeast(int64_t nTime) const;
	const ChkBlockIndex *FindFork(const ChkBlockIndex *pindex) const;
	ChkBlockLocator GetLocator(const ChkBlockIndex *pindex = nullptr) const;
	void SetTip(ChkBlockIndex *pindex);
	int Height() const {
		return vChain.size() - 1;
	}
	ChkBlockIndex *Next(const ChkBlockIndex *pindex) const {
		if (Contains(pindex))
			return (*this)[pindex->iHeight + 1];
		else
			return nullptr;
	}
	bool Contains(const ChkBlockIndex *pindex) const {
		return (*this)[pindex->iHeight] == pindex;
	}
	friend bool operator==(const CChain &a, const CChain &b) {
		return a.vChain.size() == b.vChain.size() &&
			a.vChain[a.vChain.size() - 1] == b.vChain[b.vChain.size() - 1];
	}
	ChkBlockIndex *operator[](int nHeight) const {
		if (nHeight < 0 || nHeight >= (int)vChain.size())
			return nullptr;
		return vChain[nHeight];
	}
	ChkBlockIndex *Tip() const {
		return vChain.size() > 0 ? vChain[vChain.size() - 1] : nullptr;
	}
	ChkBlockIndex * Genesis() const {
		return vChain.size() > 0 ? vChain[0] : nullptr;
	}
};

#endif
                                