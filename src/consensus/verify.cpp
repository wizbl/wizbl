// Copyright (c) 2018- The Wizblcoin Core developers

#include "consensus/verify.h"
#include "primitives/block.h"

#include "hash.h"
#include "tinyformat.h"
#include "utilstrencodings.h"
#include "chainparams.h"
#include "consensus/params.h"
#include "crypto/common.h"
#include "../util.h"
#include "../utilstrencodings.h"

#define PublicKeyBase64Length 44
#define SecretKeyBase64Length 88

#if WIZBL_SIGN_BYTES != crypto_sign_BYTES
    error "WIZBL_SIGN_BYTES != crypto_sign_BYTES"
#endif

void CBlockHeaderSignatureValidation::resetValidationInfo()
{
    m_vPublicKey.clear();
    m_iCountOfSignaturesRequiredToPassValidation = std::numeric_limits<uint16_t>::max();
    m_iCountOfPublicKeysValid = 0;
}

uint16_t CBlockHeaderSignatureValidation::setPublicKeys(const std::string& strGenesisBlockSignaturePublicKeyBase64_44)
{
    assert(0 < strGenesisBlockSignaturePublicKeyBase64_44.size() && 0 == strGenesisBlockSignaturePublicKeyBase64_44.size() % PublicKeyBase64Length); //정상적인 공개키 입력을 확인다.
    const unsigned int uiPublicKeyCount = strGenesisBlockSignaturePublicKeyBase64_44.size() / PublicKeyBase64Length;
    assert(4 <= uiPublicKeyCount); //최소의 키 숫자 보다 적은지 확인한다.

    LogPrintfd("uiPublicKeyCount [%d]", uiPublicKeyCount);

    resetValidationInfo();
    for (unsigned int i = 0; i < uiPublicKeyCount; i++) {
        m_vPublicKey.push_back(
            DecodeBase64(strGenesisBlockSignaturePublicKeyBase64_44.substr(i * PublicKeyBase64Length, PublicKeyBase64Length).c_str()));
        LogPrintfd("m_vPublicKey.push_back [%d]", i);
    }

    for (size_t i = 0; i < m_vPublicKey.size(); i++) {
        if (crypto_sign_PUBLICKEYBYTES == m_vPublicKey[i].size()) {
            m_iCountOfPublicKeysValid++;
        }
    }
    if (4 <= m_iCountOfPublicKeysValid)
        m_iCountOfSignaturesRequiredToPassValidation = m_iCountOfPublicKeysValid / 3 * 2 + 1;

    return m_iCountOfPublicKeysValid;
}

uint16_t CBlockHeaderSignatureValidation::getCountOfSignaturesRequiredToPassValidation() const
{
    return std::max((uint16_t)3, m_iCountOfSignaturesRequiredToPassValidation);
}
uint16_t CBlockHeaderSignatureValidation::getPublicKeyCount() const
{
    return m_iCountOfPublicKeysValid;
}

bool CBlockHeaderSignatureValidation::verifyBlockHeader(const CBlockHeader* pBlockHeader) const
{
    const bool bRet = verifyBlockHeader_(pBlockHeader);
    LogPrintfd("verifyBlockHeader bRet[%d], hash[%s]", bRet, pBlockHeader->GetHash().ToString());
    return bRet;
}
bool CBlockHeaderSignatureValidation::verifyBlockHeader_(const CBlockHeader* pBlockHeader) const
{
    const uint16_t iGenerateID = pBlockHeader->iGenerateID;
    const uint16_t iVerificationLength = pBlockHeader->iVerificationLength;
    const auto& sig = pBlockHeader->sig;

    LogPrintfd("if(getCountOfSignaturesRequiredToPassValidation()[%d] != iVerificationLength[%d] || ((iVerificationLength * WIZBL_SIGN_BYTES)[%d] != sig.size()[%d])) ",
        getCountOfSignaturesRequiredToPassValidation(), iVerificationLength, iVerificationLength * WIZBL_SIGN_BYTES, sig.size());
    //유효성 검사를 통과하는 데 필요한 서명의 수량이 다른가?
    if(getCountOfSignaturesRequiredToPassValidation() != iVerificationLength
        || ((iVerificationLength * WIZBL_SIGN_BYTES) != sig.size()))
        return false;

    const uint256 hash = pBlockHeader->GetHash();

    uint16_t iPassCount = 0;
    for (size_t i = 0; i < m_vPublicKey.size() && (((iPassCount + 1) * WIZBL_SIGN_BYTES) <= sig.size()); i++)
    {
        uint16_t id = i + iGenerateID;
        if(id >= m_vPublicKey.size())
            id = id % m_vPublicKey.size();
        if (crypto_sign_PUBLICKEYBYTES == m_vPublicKey[id].size()) {
            int iOK = crypto_sign_verify_detached(sig.data() + (iPassCount * WIZBL_SIGN_BYTES), hash.begin(), hash.size(), m_vPublicKey[id].data());
            LogPrintfd("iOK[%d]", iOK);
            if (0 == iOK) iPassCount++;
        }
    }

    return (iVerificationLength == iPassCount);
}

