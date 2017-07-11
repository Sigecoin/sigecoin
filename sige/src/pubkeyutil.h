// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __sig_pubkey_util_h__
#define __sig_pubkey_util_h__

#include <include/secp256k1.h>

class CPubKeyUtil {
public:
    static int ecdsa_signature_parse_der_lax(const secp256k1_context* ctx, secp256k1_ecdsa_signature* sig, const unsigned char *input, size_t inputlen);
};

#endif  /* __sig_pubkey_util_h__ */
