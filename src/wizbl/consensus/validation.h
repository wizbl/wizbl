#ifndef WIZBLCOIN_CONSENSUS_VALIDATION_H
#define WIZBLCOIN_CONSENSUS_VALIDATION_H

#include <string>
#include "consensus/params.h"
#include "consensus/consensus.h"
#include "primitives/block.h"
#include "primitives/transaction.h"
#include "version.h"

static const unsigned char REJECT_DUPLICATE = 0x12;
static const unsigned char REJECT_INVALID = 0x10;
static const unsigned char REJECT_MALFORMED = 0x01;
static const unsigned char REJECT_NONSTANDARD = 0x40;
static const unsigned char REJECT_OBSOLETE = 0x11;
static const unsigned char REJECT_CHECKPOINT = 0x43;
static const unsigned char REJECT_INSUFFICIENTFEE = 0x42;

static inline int64_t GetBlkWeight(const ChkBlock& block,
		const Consensus::Params& params) {
	int ser_flag =
			(block.nHeight < (uint32_t) params.WBLHeight) ?
					SERIALIZE_BLOCK_LEGACY : 0;
	return ((::GetSerialSize(block, SER_NETWORK,
			PROTOCOL_VERSION | SERIALIZE_TX_NO_WITNESS | ser_flag)
			* (WITNESS_SCALE_FACTOR - 1))
			+ ::GetSerialSize(block, SER_NETWORK,
					PROTOCOL_VERSION | ser_flag));
}

static inline int64_t GetTxWeight(const ChkTx& tx) {
	return ::GetSerialSize(tx, SER_NETWORK,
			PROTOCOL_VERSION | SERIALIZE_TX_NO_WITNESS)
			* (WITNESS_SCALE_FACTOR - 1)
			+ ::GetSerialSize(tx, SER_NETWORK, PROTOCOL_VERSION);
}

class ChkValidationState {
public:
	ChkValidationState() :
			mode(MODE_VALID), nDoS(0), chRejectCode(0), corruptPossible(
					false) {
	}
	unsigned int GetRejectCode() const {
		return chRejectCode;
	}
	std::string GetRejectReason() const {
		return strRejectReason;
	}
	std::string GetDebugMessage() const {
		return strDebugMessage;
	}
	bool Invalid(bool ret = false, unsigned int _chRejectCode = 0,
			const std::string &_strRejectReason = "",
			const std::string &_strDebugMessage = "") {
		return DoS(0, ret, _chRejectCode, _strRejectReason, false,
				_strDebugMessage);
	}
	void SetCorruptPossible() {
		corruptPossible = true;
	}

	bool IsValid() const {
		return mode == MODE_VALID;
	}
	bool CorruptPossible() const {
		return corruptPossible;
	}

	bool IsError() const {
		return mode == MODE_ERROR;
	}
	bool IsInvalid(int &nDoSOut) const {
		if (IsInvalid()) {
			nDoSOut = nDoS;
			return true;
		}
		return false;
	}
	bool IsInvalid() const {
		return mode == MODE_INVALID;
	}

	bool Error(const std::string& strRejectReasonIn) {
		if (mode == MODE_VALID)
			strRejectReason = strRejectReasonIn;
		mode = MODE_ERROR;
		return false;
	}

	bool DoS(int level, bool ret = false, unsigned int chRejectCodeIn = 0,
			const std::string &strRejectReasonIn = "",
			bool corruptionIn = false,
			const std::string &strDebugMessageIn = "") {
		chRejectCode = chRejectCodeIn;
		strRejectReason = strRejectReasonIn;
		corruptPossible = corruptionIn;
		strDebugMessage = strDebugMessageIn;
		if (mode == MODE_ERROR)
			return ret;
		nDoS += level;
		mode = MODE_INVALID;
		return ret;
	}

private:
	enum mode_state {
		MODE_VALID,
		MODE_INVALID,
		MODE_ERROR,
	} mode;
	int nDoS;
	std::string strRejectReason;
	unsigned int chRejectCode;
	bool corruptPossible;
	std::string strDebugMessage;
};

#endif
                                                                                                                                                                                                                      