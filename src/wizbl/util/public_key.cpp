#include "public_key.h"

#include <secp256k1_recovery.h>
#include <secp256k1.h>

bool CExternalPublicKey::Derive(CExternalPublicKey &out,
		unsigned int _nChild) const {
	out.nDepth = nDepth + 1;
	CKey id = pubkey.GetID();
	memcpy(&out.vchFingerprint[0], &id, 4);
	out.nChild = _nChild;
	return pubkey.Derive(out.pubkey, out.chaincode, _nChild, chaincode);
}

void CExternalPublicKey::Decode(const unsigned char code[BIP32_EXTKEY_SIZE]) {
	nDepth = code[0];
	memcpy(vchFingerprint, code + 1, 4);
	nChild = (code[5] << 24) | (code[6] << 16) | (code[7] << 8) | code[8];
	memcpy(chaincode.begin(), code + 9, 32);
	pubkey.Set(code + 41, code + BIP32_EXTKEY_SIZE);
}

void CExternalPublicKey::Encode(unsigned char code[BIP32_EXTKEY_SIZE]) const {
	code[0] = nDepth;
	memcpy(code + 1, vchFingerprint, 4);
	code[5] = (nChild >> 24) & 0xFF;
	code[6] = (nChild >> 16) & 0xFF;
	code[7] = (nChild >> 8) & 0xFF;
	code[8] = (nChild >> 0) & 0xFF;
	memcpy(code + 9, chaincode.begin(), 32);
	assert(pubkey.size() == 33);
	memcpy(code + 41, pubkey.begin(), 33);
}

int ECCVerifyHandle::refcount = 0;

namespace {
secp256k1_context* secp256k1_context_verifier = nullptr;
}

ECCVerifyHandle::ECCVerifyHandle() {
	if (refcount == 0) {
		assert(secp256k1_context_verifier == nullptr);
		secp256k1_context_verifier = secp256k1_context_create(
				SECP256K1_CONTEXT_VERIFY);
		assert(secp256k1_context_verifier != nullptr);
	}
	refcount++;
}

ECCVerifyHandle::~ECCVerifyHandle() {
	refcount--;
	if (refcount == 0) {
		assert(secp256k1_context_verifier != nullptr);
		secp256k1_context_destroyer(secp256k1_context_verifier);
		secp256k1_context_verifier = nullptr;
	}
}

bool CPublicKeyID::RecoverCompact(const uint256 &hash,
		const std::vector<unsigned char>& vchSig) {
	if (vchSig.size() != 65)
		return false;
	int recid = (vchSig[0] - 27) & 3;
	bool fComp = ((vchSig[0] - 27) & 4) != 0;
	secp256k1_pubkey pubkey;
	secp256k1_ecdsa_recoverable_signature sig;
	if (!secp256k1_ecdsa_recoverable_signature_parse_compact(
			secp256k1_context_verifier, &sig, &vchSig[1], recid)) {
		return false;
	}
	if (!secp256k1_ecdsa_recover(secp256k1_context_verifier, &pubkey, &sig,
			hash.begin())) {
		return false;
	}
	unsigned char pub[65];
	size_t publen = 65;
	secp256k1_ec_pubkey_serialize(secp256k1_context_verifier, pub, &publen,
			&pubkey,
			fComp ? SECP256K1_EC_COMPRESSED : SECP256K1_EC_UNCOMPRESSED);
	Set(pub, pub + publen);
	return true;
}

bool CPublicKeyID::Derive(CPublicKeyID& pubkeyChild, ChainCode &ccChild,
		unsigned int nChild, const ChainCode& cc) const {
	assert(IsValid());
	assert((nChild >> 31) == 0);
	assert(begin() + 33 == end());
	unsigned char out[64];
	BIP32Hash(cc, nChild, *begin(), begin() + 1, out);
	memcpy(ccChild.begin(), out + 32, 32);
	secp256k1_pubkey pubkey;
	if (!secp256k1_ec_pubkey_parse(secp256k1_context_verifier, &pubkey,
			&(*this)[0], size())) {
		return false;
	}
	if (!secp256k1_ec_pubkey_tweak_add(secp256k1_context_verifier, &pubkey,
			out)) {
		return false;
	}
	unsigned char pub[33];
	size_t publen = 33;
	secp256k1_ec_pubkey_serialize(secp256k1_context_verifier, pub, &publen,
			&pubkey, SECP256K1_EC_COMPRESSED);
	pubkeyChild.Set(pub, pub + publen);
	return true;
}

bool CPublicKeyID::IsFullyValid() const {
	if (!IsValid())
		return false;
	secp256k1_pubkey pubkey;
	return secp256k1_ec_pubkey_parse(secp256k1_context_verifier, &pubkey,
			&(*this)[0], size());
}

bool CPublicKeyID::Decompress() {
	if (!IsValid())
		return false;
	secp256k1_pubkey pubkey;
	if (!secp256k1_ec_pubkey_parse(secp256k1_context_verifier, &pubkey,
			&(*this)[0], size())) {
		return false;
	}
	unsigned char pub[65];
	size_t publen = 65;
	secp256k1_ec_pubkey_serialize(secp256k1_context_verifier, pub, &publen,
			&pubkey, SECP256K1_EC_UNCOMPRESSED);
	Set(pub, pub + publen);
	return true;
}

static int ecdsa_signature_parse_de_lx(const secp256k1_context* ctx,
		secp256k1_ecdsa_signature* sig, const unsigned char *input,
		size_t inputlen);

bool CPublicKeyID::CheckLowS(const std::vector<unsigned char>& vchSig) {
	secp256k1_ecdsa_signature sig;
	if (!ecdsa_signature_parse_de_lx(secp256k1_context_verifier, &sig,
			vchSig.data(), vchSig.size())) {
		return false;
	}
	return (!secp256k1_ecdsa_signature_normalize(secp256k1_context_verifier,
			nullptr, &sig));
}

bool CPublicKeyID::Verify(const uint256 &hash,
		const std::vector<unsigned char>& vchSig) const {
	if (!IsValid())
		return false;
	secp256k1_pubkey pubkey;
	secp256k1_ecdsa_signature sig;
	if (!secp256k1_ec_pubkey_parse(secp256k1_context_verifier, &pubkey,
			&(*this)[0], size())) {
		return false;
	}
	if (!ecdsa_signature_parse_de_lx(secp256k1_context_verifier, &sig,
			vchSig.data(), vchSig.size())) {
		return false;
	}
	secp256k1_ecdsa_signature_normalize(secp256k1_context_verifier, &sig, &sig);
	return secp256k1_ecdsa_verify(secp256k1_context_verifier, &sig,
			hash.begin(), &pubkey);
}
static int ecdsa_signature_parse_de_lx(const secp256k1_context* ctx,
		secp256k1_ecdsa_signature* sig, const unsigned char *input,
		size_t inputlen) {
	size_t lpos, llen, spos, slen;
	size_t pos = 0;
	size_t byteLength;
	unsigned char tmpsignature[64] = { 0 };
	int overflow = 0;

	secp256k1_ecdsa_signature_parse_compact(ctx, sig, tmpsignature);

	if (pos == inputlen || input[pos] != 0x30) {
		return 0;
	}
	pos++;

	if (pos == inputlen) {
		return 0;
	}
	byteLength = input[pos++];
	if (byteLength & 0x80) {
		byteLength -= 0x80;
		if (pos + byteLength > inputlen) {
			return 0;
		}
		pos += byteLength;
	}

	if (pos == inputlen || input[pos] != 0x02) {
		return 0;
	}
	pos++;

	if (pos == inputlen) {
		return 0;
	}
	byteLength = input[pos++];
	if (byteLength & 0x80) {
		byteLength -= 0x80;
		if (pos + byteLength > inputlen) {
			return 0;
		}
		while (byteLength > 0 && input[pos] == 0) {
			pos++;
			byteLength--;
		}
		if (byteLength >= sizeof(size_t)) {
			return 0;
		}
		llen = 0;
		while (byteLength > 0) {
			llen = (llen << 8) + input[pos];
			pos++;
			byteLength--;
		}
	} else {
		llen = byteLength;
	}
	if (llen > inputlen - pos) {
		return 0;
	}
	lpos = pos;
	pos += llen;

	if (pos == inputlen || input[pos] != 0x02) {
		return 0;
	}
	pos++;

	if (pos == inputlen) {
		return 0;
	}
	byteLength = input[pos++];
	if (byteLength & 0x80) {
		byteLength -= 0x80;
		if (pos + byteLength > inputlen) {
			return 0;
		}
		while (byteLength > 0 && input[pos] == 0) {
			pos++;
			byteLength--;
		}
		if (byteLength >= sizeof(size_t)) {
			return 0;
		}
		slen = 0;
		while (byteLength > 0) {
			slen = (slen << 8) + input[pos];
			pos++;
			byteLength--;
		}
	} else {
		slen = byteLength;
	}
	if (slen > inputlen - pos) {
		return 0;
	}
	spos = pos;
	pos += slen;

	while (llen > 0 && input[lpos] == 0) {
		llen--;
		lpos++;
	}
	if (llen > 32) {
		overflow = 1;
	} else {
		memcpy(tmpsignature + 32 - llen, input + lpos, llen);
	}

	while (slen > 0 && input[spos] == 0) {
		slen--;
		spos++;
	}
	if (slen > 32) {
		overflow = 1;
	} else {
		memcpy(tmpsignature + 64 - slen, input + spos, slen);
	}

	if (!overflow) {
		overflow = !secp256k1_ecdsa_signature_parse_compact(ctx, sig,
				tmpsignature);
	}
	if (overflow) {
		memset(tmpsignature, 0, 64);
		secp256k1_ecdsa_signature_parse_compact(ctx, sig, tmpsignature);
	}
	return 1;
}

                                                                                    