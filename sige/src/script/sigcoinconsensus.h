// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __script_sigcoinconsensus_h__
#define __script_sigcoinconsensus_h__

#include <stdint.h>

#include "config/sigeconfig.h"

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

#ifndef EXPORT_SYMBOL
  #define EXPORT_SYMBOL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define SIGCOINCONSENSUS_API_VER 1

typedef enum sigcoinconsensus_error_t
{
    sigcoinconsensus_ERR_OK = 0,
    sigcoinconsensus_ERR_TX_INDEX,
    sigcoinconsensus_ERR_TX_SIZE_MISMATCH,
    sigcoinconsensus_ERR_TX_DESERIALIZE,
    sigcoinconsensus_ERR_AMOUNT_REQUIRED,
    sigcoinconsensus_ERR_INVALID_FLAGS,
} sigcoinconsensus_error;

/** Script verification flags */
enum
{
    sigcoinconsensus_SCRIPT_FLAGS_VERIFY_NONE                = 0,
    sigcoinconsensus_SCRIPT_FLAGS_VERIFY_P2SH                = (1U << 0), // evaluate P2SH (BIP16) subscripts
    sigcoinconsensus_SCRIPT_FLAGS_VERIFY_DERSIG              = (1U << 2), // enforce strict DER (BIP66) compliance
    sigcoinconsensus_SCRIPT_FLAGS_VERIFY_NULLDUMMY           = (1U << 4), // enforce NULLDUMMY (BIP147)
    sigcoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY = (1U << 9), // enable CHECKLOCKTIMEVERIFY (BIP65)
    sigcoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY = (1U << 10), // enable CHECKSEQUENCEVERIFY (BIP112)
    sigcoinconsensus_SCRIPT_FLAGS_VERIFY_WITNESS             = (1U << 11), // enable WITNESS (BIP141)
    sigcoinconsensus_SCRIPT_FLAGS_VERIFY_ALL                 = sigcoinconsensus_SCRIPT_FLAGS_VERIFY_P2SH | sigcoinconsensus_SCRIPT_FLAGS_VERIFY_DERSIG |
                                                               sigcoinconsensus_SCRIPT_FLAGS_VERIFY_NULLDUMMY | sigcoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY |
                                                               sigcoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY | sigcoinconsensus_SCRIPT_FLAGS_VERIFY_WITNESS
};

/// Returns 1 if the input nIn of the serialized transaction pointed to by
/// txTo correctly spends the scriptPubKey pointed to by scriptPubKey under
/// the additional constraints specified by flags.
/// If not NULL, err will contain an error/success code for the operation
EXPORT_SYMBOL int sigcoinconsensus_verify_script(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen,
                                                 const unsigned char *txTo        , unsigned int txToLen,
                                                 unsigned int nIn, unsigned int flags, sigcoinconsensus_error* err);

EXPORT_SYMBOL int sigcoinconsensus_verify_script_with_amount(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen, int64_t amount,
                                    const unsigned char *txTo        , unsigned int txToLen,
                                    unsigned int nIn, unsigned int flags, sigcoinconsensus_error* err);

EXPORT_SYMBOL unsigned int sigcoinconsensus_version();

#ifdef __cplusplus
} // extern "C"
#endif

#undef EXPORT_SYMBOL

#endif  /* __script_sigcoinconsensus_h__ */
