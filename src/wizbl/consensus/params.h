#ifndef WIZBLCOIN_CONSENSUS_PARAMS_H
#define WIZBLCOIN_CONSENSUS_PARAMS_H

#include "uint256.h"
#include <string>
#include <map>

namespace Consensus {

struct Params {
	uint256 hashGenesisBlock;
	int nSubsidyHalvingInterval;
	int bip34Height;
	uint256 bip34Hash;
	int bip65Height;
	int bip66Height;
	int WBLHeight;
	int WBLZawyLWMAHeight;
	int WBLEquihashForkHeight;
	int64_t WBLMaxFutureBlkTime;
	int WBLPremineWindow;
	bool WBLPremineForceWhtlist;
	uint32_t nRuleChangeActivateThreshold;
	uint32_t nMinerConfirmateWindow;
	bip9Deployment vDeployments[MAX_VERSION_BITS_DEPLOYMENTS];
	uint256 powLimit;
	uint256 powLimitLegacy;
	uint256 powLimitStart;

	const uint256& PowLimit(bool postfork) const {
		return postfork ? powLimit : powLimitLegacy;
	}
	bool fPowAllowMinDifficultyBlocks;
	bool fPowNoRetargeting;
	int64_t nPowTargetSpacing;
	int64_t nPowTargetTimespanLegacy;
	int64_t DifficultyAdjustmentInterval() const {
		return nPowTargetTimespanLegacy / nPowTargetSpacing;
	}
	uint256 nMinimumChainWork;
	uint256 defaultAssumeValid;

	int64_t nDigishieldAvgWindow;
	int64_t nDigishieldMaxAdjtDown;
	int64_t nDigishieldMaxAdjUp;
	int64_t DigishieldAveragingWindowTimespan() const {
		return nDigishieldAvgWindow * nPowTargetSpacing;
	}
	int64_t DigishieldMinActualTime() const {
		return (DigishieldAveragingWindowTimespan()
				* (100 - nDigishieldMaxAdjUp)) / 100;
	}
	int64_t DigishieldMaxActualTimespan() const {
		return (DigishieldAveragingWindowTimespan()
				* (100 + nDigishieldMaxAdjtDown)) / 100;
	}

	int64_t nZawyLwmaAveragingWindow;  // N
	int64_t nZawyLwmaAjustedWeight;  // k = (N+1)/2 * 0.9989^(500/N) * T

};

struct bip9Deployment {
	int bit;
	int64_t nStartTime;
	int64_t nTimeout;
};

enum DeploymentPos {
	DEPLOYMENT_TESTDUMMY, DEPLOYMENT_CSV,
	DEPLOYMENT_SEGWIT,
	MAX_VERSION_BITS_DEPLOYMENTS
};

}

#endif
                                                                                                       