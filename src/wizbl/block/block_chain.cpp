#include "block_chain.h"
int static inline InvertLowestOne(int n) { return n & (n - 1); }
int static inline GetSkipHeight(int height) {
	if (height < 2)
		return 0;
	return (height & 1) ? InvertLowestOne(InvertLowestOne(height - 1)) + 1 : InvertLowestOne(height);
}
arith_uint256 GetBlockProof(const ChkBlockIndex& block)
{
	arith_uint256 bnTarget;
	bool fNegative;
	bool fOverflow;
	bnTarget.SetCompact(block.uiBits, &fNegative, &fOverflow);
	if (fNegative || fOverflow || bnTarget == 0)
		return 0;
	return (~bnTarget / (bnTarget + 1)) + 1;
}
int64_t GetBlockProofEquivalentTime(const ChkBlockIndex& to, const ChkBlockIndex& from, const ChkBlockIndex& tip, const Consensus::Params& params)
{
	arith_uint256 r;
	int sign = 1;
	if (to.nChainWork > from.nChainWork) {
		r = to.nChainWork - from.nChainWork;
	}
	else {
		r = from.nChainWork - to.nChainWork;
		sign = -1;
	}
	r = r * arith_uint256(params.nPowTargetSpacing) / GetBlockProof(tip);
	if (r.bits() > 63) {
		return sign * std::numeric_limits<int64_t>::max();
	}
	return sign * r.GetLow64();
}
const ChkBlockIndex* LastCommonAncestor(const ChkBlockIndex* pa, const ChkBlockIndex* pb) {
	if (pa->iHeight > pb->iHeight) {
		pa = pa->get_Ancestor(pb->iHeight);
	}
	else if (pb->iHeight > pa->iHeight) {
		pb = pb->get_Ancestor(pa->iHeight);
	}
	while (pa != pb && pa && pb) {
		pa = pa->pprev;
		pb = pb->pprev;
	}
	assert(pa == pb);
	return pa;
}
ChkBlockIndex* CChain::FindEarliestAtLeast(int64_t nTime) const
{
	std::vector<ChkBlockIndex*>::const_iterator lower = std::lower_bound(vChain.begin(), vChain.end(), nTime,
		[](ChkBlockIndex* pBlock, const int64_t& time) -> bool { return pBlock->get_BlockTimeMax() < time; });
	return (lower == vChain.end() ? nullptr : *lower);
}
const ChkBlockIndex *CChain::FindFork(const ChkBlockIndex *pindex) const {
	if (pindex == nullptr) {
		return nullptr;
	}
	if (pindex->iHeight > Height())
		pindex = pindex->get_Ancestor(Height());
	while (pindex && !Contains(pindex))
		pindex = pindex->pprev;
	return pindex;
}
ChkBlockLocator CChain::GetLocator(const ChkBlockIndex *pindex) const {
	int nStep = 1;
	std::vector<uint256> vHave;
	vHave.reserve(32);
	if (!pindex)
		pindex = Tip();
	while (pindex) {
		vHave.push_back(pindex->get_BlockHash());
		if (pindex->iHeight == 0)
			break;
		int nHeight = std::max(pindex->iHeight - nStep, 0);
		if (Contains(pindex)) {
			pindex = (*this)[nHeight];
		}
		else {
			pindex = pindex->get_Ancestor(nHeight);
		}
		if (vHave.size() > 10)
			nStep *= 2;
	}
	return ChkBlockLocator(vHave);
}
void CChain::SetTip(ChkBlockIndex *pindex) {
	if (pindex == nullptr) {
		vChain.clear();
		return;
	}
	vChain.resize(pindex->iHeight + 1);
	while (pindex && vChain[pindex->iHeight] != pindex) {
		vChain[pindex->iHeight] = pindex;
		pindex = pindex->pprev;
	}
}
void ChkBlockIndex::Skip_Build()
{
	if (pprev)
		pskip = pprev->get_Ancestor(GetSkipHeight(iHeight));
}
const ChkBlockIndex* ChkBlockIndex::get_Ancestor(int height) const
{
	return const_cast<ChkBlockIndex*>(this)->get_Ancestor(height);
}
ChkBlockIndex* ChkBlockIndex::get_Ancestor(int height)
{
	if (height > iHeight || height < 0)
		return nullptr;

	ChkBlockIndex* pindexWalk = this;
	int heightWalk = iHeight;
	while (heightWalk > height) {
		int heightSkip = GetSkipHeight(heightWalk);
		int heightSkipPrev = GetSkipHeight(heightWalk - 1);
		if (pindexWalk->pskip != nullptr &&
			(heightSkip == height ||
			(heightSkip > height && !(heightSkipPrev < heightSkip - 2 &&
				heightSkipPrev >= height)))) {
			pindexWalk = pindexWalk->pskip;
			heightWalk = heightSkip;
		}
		else {
			assert(pindexWalk->pprev);
			pindexWalk = pindexWalk->pprev;
			heightWalk--;
		}
	}
	return pindexWalk;
}

                                                