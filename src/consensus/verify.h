// Copyright (c) 2018- The Wizblcoin Core developers

#ifndef WIZBLCOIN_CONSENSUS_VERIFY_H
#define WIZBLCOIN_CONSENSUS_VERIFY_H

#include "arith_uint256.h"
#include "serialize.h"
#include "uint256.h"
#include "version.h"
#include "sodium.h"
#include <string>
#include <vector>
#include <utility>
#include <map>

class CBlockHeader;

/**
* <h1>CBlockHeaderSignatureValidation</h1>
* <p>
* <b>Note:</b>
* 블럭 전자서명 검증 관련 모음입니다.
*/
class CBlockHeaderSignatureValidation
{
public:
    CBlockHeaderSignatureValidation() {
        resetValidationInfo();
    }
    /**
    * <h2>std::vector<std::vector<unsigned char>> m_vPublicKey</h2>
    * <p>
    * <b>Note:</b>
    * 블럭 전자서명 공개키 관리
    */
public:
    std::vector<std::vector<unsigned char>> m_vPublicKey;

private:
    uint16_t m_iCountOfSignaturesRequiredToPassValidation;//유효성 검사를 통과하는 데 필요한 서명 수
    uint16_t m_iCountOfPublicKeysValid;//유효한 공개 키 수

public:
    void resetValidationInfo();

    //regtest 예외 설정
private:
    bool m_bRegTest = false;
public:
    void setRegTest() { m_bRegTest = true; }
    bool isRegTest() { return m_bRegTest; }

    //전자서명 공개키 등록
public:
    uint16_t setPublicKeys(const std::string& strGenesisBlockSignaturePublicKeyBase64_44);
    
    /**
    * <h2>uint16_t getCountOfSignaturesRequiredToPassValidation()</h2>
    * <p>
    * <b>Note:</b>
    * 향후 블럭에 키 업데이트등을 추가 예정인데,
    * 패밀리 그룹의 키 or 노드에 변화가 있는경우 예상되는 어려움을 피하고자 함.
    *
    * @return 유효성 검사를 통과하는 데 필요한 서명 수를 반환합니다.
    */
public:
    uint16_t getCountOfSignaturesRequiredToPassValidation() const;//유효성 검사를 통과하는 데 필요한 서명 수 얻기

    /**
    * <h2>uint16_t getPublicKeyCount()</h2>
    * <p>
    * <b>Note:</b>
    * 향후 블럭에 키 업데이트등을 추가 예정인데,
    * 패밀리 그룹의 키 or 노드에 변화가 있는경우 예상되는 어려움을 피하고자 함.
    *
    * @return 관리 키중에 유효한 개수를 반환합니다.
    */
public:
    uint16_t getPublicKeyCount() const;

    /**
    * <h2>bool verifyBlockHeader()</h2>
    * <p>
    * <b>Note:</b>
    * 블럭헤더의 서명을 검증합니다.
    *
    * @param pBlockHeader 검증 받을 블럭의 헤더 정보입니다.
    *
    * @return 검증 성공을 bool값으로 반환합니다.
    */
public:
    bool verifyBlockHeader(const CBlockHeader* pBlockHeader) const;
private:
    bool verifyBlockHeader_(const CBlockHeader* pBlockHeader) const;
};


#endif // WIZBLCOIN_CONSENSUS_VERIFY_H
