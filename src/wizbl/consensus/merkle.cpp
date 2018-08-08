#include "merkle.h"

#include "../util/str_encodings.h"
#include "hash.h"
std::vector<uint256> BlkMerkleBranch(const ChkBlock& block, uint32_t position)
{
    std::vector<uint256> leaves;
    leaves.resize(block.vtx.size());
    for (size_t s = 0; s < block.vtx.size(); s++) {
        leaves[s] = block.vtx[s]->GetHash();
    }
    return CmptMerkleBranch(leaves, position);
}

uint256 BlkWitnessMerkleRoot(const ChkBlock& block, bool* mutated)
{
    std::vector<uint256> leaves;
    leaves.resize(block.vtx.size());
    leaves[0].SetNull();
    for (size_t s = 1; s < block.vtx.size(); s++) {
        leaves[s] = block.vtx[s]->GetWitnessHash();
    }
    return CmpteMerkleRoot(leaves, mutated);
}

std::vector<uint256> CmptMerkleBranch(const std::vector<uint256>& leaves, uint32_t position) {
    std::vector<uint256> ret;
    MerkleComputation(leaves, nullptr, nullptr, position, &ret);
    return ret;
}

uint256 BlkMerkleRoot(const ChkBlock& block, bool* mutated)
{
    std::vector<uint256> leaves;
    leaves.resize(block.vtx.size());
    for (size_t s = 0; s < block.vtx.size(); s++) {
        leaves[s] = block.vtx[s]->GetHash();
    }
    return CmpteMerkleRoot(leaves, mutated);
}

uint256 CmptMerkleRootFromBranch(const uint256& leaf, const std::vector<uint256>& vMerkleBranch, uint32_t nIndex) {
    uint256 hash = leaf;
    for (std::vector<uint256>::const_iterator it = vMerkleBranch.begin(); it != vMerkleBranch.end(); ++it) {
        if (nIndex & 1) {
            hash = Hash(BEGIN(*it), END(*it), BEGIN(hash), END(hash));
        } else {
            hash = Hash(BEGIN(hash), END(hash), BEGIN(*it), END(*it));
        }
        nIndex >>= 1;
    }
    return hash;
}

uint256 CmpteMerkleRoot(const std::vector<uint256>& leaves, bool* mutated) {
    uint256 hash;
    MerkleComputation(leaves, &hash, mutated, -1, nullptr);
    return hash;
}

static void MerkleComputation(const std::vector<uint256>& leaves, uint256* proot, bool* pmutated, uint32_t branchpos, std::vector<uint256>* pbranch) {
    if (pbranch) pbranch->clear();
    if (leaves.size() == 0) {
        if (pmutated) *pmutated = false;
        if (proot) *proot = uint256();
        return;
    }
    bool mutated = false;
    uint32_t count = 0;
    uint256 inner[32];
    int matchlevel = -1;
    while (count < leaves.size()) {
        uint256 h = leaves[count];
        bool match = count == branchpos;
        count++;
        int level;
        for (level = 0; !(count & (((uint32_t)1) << level)); level++) {
            if (pbranch) {
                if (match) {
                    pbranch->push_back(inner[level]);
                } else if (matchlevel == level) {
                    pbranch->push_back(h);
                    match = true;
                }
            }
            mutated |= (inner[level] == h);
            CHash256().Write(inner[level].begin(), 32).Write(h.begin(), 32).Finalize(h.begin());
        }
        inner[level] = h;
        if (match) {
            matchlevel = level;
        }
    }
    int level = 0;
    while (!(count & (((uint32_t)1) << level))) {
        level++;
    }
    uint256 h = inner[level];
    bool match = matchlevel == level;
    while (count != (((uint32_t)1) << level)) {
        if (pbranch && match) {
            pbranch->push_back(h);
        }
        CHash256().Write(h.begin(), 32).Write(h.begin(), 32).Finalize(h.begin());
        count += (((uint32_t)1) << level);
        level++;
        while (!(count & (((uint32_t)1) << level))) {
            if (pbranch) {
                if (match) {
                    pbranch->push_back(inner[level]);
                } else if (matchlevel == level) {
                    pbranch->push_back(h);
                    match = true;
                }
            }
            CHash256().Write(inner[level].begin(), 32).Write(h.begin(), 32).Finalize(h.begin());
            level++;
        }
    }
    if (pmutated) *pmutated = mutated;
    if (proot) *proot = h;
}
                                                                                                                                                                                     