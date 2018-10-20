
#include "define_wizbl.h"

const unsigned int WIZBL_MAX_BLOCK_SIZE = 1000000 * 32;
const unsigned int WIZBL_DEFAULT_BLOCK_SIZE = 1000000 * 8;

unsigned long TRANSACTION_FEE_DENOMINATOR = DEFAULT_TRANSACTION_FEE_DENOMINATOR;
unsigned long TRANSACTION_FEE_NUMERATOR = DEFAULT_TRANSACTION_FEE_NUMERATOR;
const unsigned long& TRANSACTION_REQUEST_MINIMUM_UNIT = TRANSACTION_FEE_DENOMINATOR;

//GenesisBlock 전자서명 공개키입니다.
const char* GenesisBlockSignaturePublicKeyBase64_44_MAINNET =
    "ZLaf/3zAVvzH26BkcT5a1IYmRK/UhwlxBaKI6m518cs="
    "cvARFy4eFOuLml4q9FntlOvhST6qQeEn4QiL+o67iC0="
    "uARk8tcIhorrkeoTN/DZUdDMeYHHf8qsknbvJ1Slfx4="
    "9k9HlaMyldqT19LASuROxf3ULt6Fjl5e0VswR74oTLw=";
const char* GenesisBlockSignaturePublicKeyBase64_44_TESTNET =
    "IKaZEKdky+wCgsg05a1fiPrTgpnoKce40y6hrDSSCSY="
    "JXePNh7ikR4xvgMwBkpdzADUQH/y6sKaAH9txon3m2E="
    "t2zWaQBlGLmNt2L7/Sk1E3X56+HllPmaISNRI/uslS4="
    "36wtK4WOxplhnbGBIMwjunwMis5EBu3T53JzB7WyODA=";
const char* GenesisBlockSignaturePublicKeyBase64_44_REGTEST = "";

//GenesisBlock 서명 값입니다.
const char* GenesisBlockSignatureBase64_256_MAINNET = "z1+YYVVb8jKN31FMeNPb/WsXJv3sQRNBt/x0T6q8QsWp5x3BFTbyE7ZVc+XK0zgL+DDFRM05QbhRnMReJ0NWDFtCqqab6eXjzYSxTW7Q2LKxTXJEW75DJR0UEOgZQ3+v+xE8UZMiGFXG4GErd9h6/jX5C5CaVvJWuUI/4FzGxw3RGwWSari0Oi1c+fTlACjD7z9cuVS9+V44zBnZ4weFwzdV6eOAa8s41ddJ8CZinZILTDYX1PvpqToHMAEUiMQC";
const char* GenesisBlockSignatureBase64_256_TESTNET = "S6s0hFe1WnolfJweRhvEnynJ4ICqw+LfNSdq4nHaS1U+tngHlLLS08VT3tHeMFBxLvu16MrtsdsRI/hTbbFoBvoaM0b7pINBJaoMZCauSt7yWdIijgosavFSgZ3z+kzMe4yIo7nsxjcMB86dswIT0IjVnVTub35PPXc4ygqFNQxTtSiN8iQGzvvtEakd538dczJ4Eu7Wi3jEo3RtzM5LM0uzVbWHXLtQZDsKZIJ4bwLjfWwZ4nY0RTnKTqNCYDAL";
const char* GenesisBlockSignatureBase64_256_REGTEST = "";

//SecretKey 는 GenesisBlock 의 서명을 등록하기 위해서 사용하는 것이므로 GenesisBlockSignatureBase64 등록후 반드시 비워주세요.(분리관리)
const char* GenesisBlockSignatureSecretKey_MAINNET = "";
const char* GenesisBlockSignatureSecretKey_TESTNET = "";
const char* GenesisBlockSignatureSecretKey_REGTEST = "";

//GenesisBlock hash 값
const char* CHECK_GENESIS_BLOCK_HASH_MAINNET = "0xe63bcc812979f31dde0ff7a8e6621be6378d31829f734964652f4159ca213950";
const char* CHECK_GENESIS_BLOCK_HASH_TESTNET = "0x9db64dfe8ea4c8994359bf61cf5658910165a1a5f446ee29079e3b836181f50e";
const char* CHECK_GENESIS_BLOCK_HASH_REGTEST = "0xde911c05910b8d24874068c6979f0a972d17e003496bf03ad5e306618ca17c73";
const char* CHECK_GENESIS_BLOCK_MERKLEROOT_HASH = "0x967e07ccb26a253513e352b01aff1579e6ae50d6a5aa751b89df1fd0d039ca93";
