#ifndef WIZBLCOIN_MERKLE
#define WIZBLCOIN_MERKLE

#include <vector>
#include <stdint.h>

#include "primitives/block.h"
#include "primitives/transaction.h"
#include "uint256.h"

std::vector<uint256> BlkMerkleBranch(const ChkBlock& block, uint32_t position);
uint256 BlkWitnessMerkleRoot(const ChkBlock& block, bool* mutated = nullptr);

std::vector<uint256> CmptMerkleBranch(const std::vector<uint256>& leaves, uint32_t position);

uint256 BlkMerkleRoot(const ChkBlock& block, bool* mutated = nullptr);

uint256 CmptMerkleRootFromBranch(const uint256& leaf, const std::vector<uint256>& branch, uint32_t position);

uint256 CmpteMerkleRoot(const std::vector<uint256>& leaves, bool* mutated = nullptr);

#endif
                                                                                               