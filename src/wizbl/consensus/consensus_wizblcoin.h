
#ifndef WIZBLCOIN_CONSENSUS_WIZBLCOIN_H
#define WIZBLCOIN_CONSENSUS_WIZBLCOIN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EXPORT_SYMBOL
  #define EXPORT_SYMBOL
#endif

#if defined(BUILD_WIZBLCOIN_INTERNAL) && defined(HAVE_CONFIG_H)
#include "config/wizblcoin-config.h"
  #if defined(_WIN32)
    #if defined(DLL_EXPORT)
      #if defined(HAVE_FUNC_ATTRIBUTE_DLLEXPORT)
        #define EXPORT_SYMBOL __declspec(dllexport)
      #else
        #define EXPORT_SYMBOL
      #endif
    #endif
  #elif defined(HAVE_FUNC_ATTRIBUTE_VISIBILITY)
    #define EXPORT_SYMBOL __attribute__ ((visibility ("default")))
  #endif
#elif defined(MSC_VER) && !defined(STATIC_LIBWIZBLCOINCONSENSUS)
  #define EXPORT_SYMBOL __declspec(dllimport)
#endif

#define WIZBLCOINCONSENSUS_API_VER 1

EXPORT_SYMBOL int wizblcoinconsensus_verify_script_with_amount(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen, int64_t amount,
                                    const unsigned char *txTo        , unsigned int txToLen,
                                    unsigned int nIn, unsigned int flags, wizblcoinconsensus_error* err);

EXPORT_SYMBOL int wizblcoinconsensus_verify_script(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen,
                                                 const unsigned char *txTo        , unsigned int txToLen,
                                                 unsigned int nIn, unsigned int flags, wizblcoinconsensus_error* err);

EXPORT_SYMBOL unsigned int wizblcoinconsensus_version();

enum
{
    wizblcoinconsensus_SCRIPT_FLAGS_VERIFY_NONE                = 0,
    wizblcoinconsensus_SCRIPT_FLAGS_VERIFY_P2SH                = (1U << 0),
    wizblcoinconsensus_SCRIPT_FLAGS_VERIFY_DERSIG              = (1U << 2),
    wizblcoinconsensus_SCRIPT_FLAGS_VERIFY_NULLDUMMY           = (1U << 4),
    wizblcoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY = (1U << 9),
    wizblcoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY = (1U << 10),
    wizblcoinconsensus_SCRIPT_FLAGS_VERIFY_WITNESS             = (1U << 11),
    wizblcoinconsensus_SCRIPT_FLAGS_VERIFY_ALL                 = wizblcoinconsensus_SCRIPT_FLAGS_VERIFY_P2SH | wizblcoinconsensus_SCRIPT_FLAGS_VERIFY_DERSIG |
                                                               wizblcoinconsensus_SCRIPT_FLAGS_VERIFY_NULLDUMMY | wizblcoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY |
                                                               wizblcoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY | wizblcoinconsensus_SCRIPT_FLAGS_VERIFY_WITNESS
};

typedef enum wizblcoinconsensus_error_t
{
    wizblcoinconsensus_ERR_OK = 0,
    wizblcoinconsensus_ERR_TX_INDEX,
    wizblcoinconsensus_ERR_TX_SIZE_MISMATCH,
    wizblcoinconsensus_ERR_TX_DESERIALIZE,
    wizblcoinconsensus_ERR_AMOUNT_REQUIRED,
    wizblcoinconsensus_ERR_INVALID_FLAGS,
} wizblcoinconsensus_error;

#ifdef __cplusplus
}
#endif

#undef EXPORT_SYMBOL

#endif
                                         