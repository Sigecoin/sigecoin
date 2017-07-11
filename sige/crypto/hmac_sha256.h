// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.//

#ifndef __crypto_hmac_sha256_h__
#define __crypto_hmac_sha256_h__

#include "crypto/sha256.h"

#include <stdint.h>
#include <stdlib.h>

/** A hasher class for HMAC-SHA-512. */
class CHMAC_SHA256
{
private:
    CSHA256 outer;
    CSHA256 inner;

public:
    static const size_t OUTPUT_SIZE = 32;

    CHMAC_SHA256(const unsigned char* key, size_t keylen);
    CHMAC_SHA256& Write(const unsigned char* data, size_t len)
    {
        inner.Write(data, len);
        return *this;
    }
    void Finalize(unsigned char hash[OUTPUT_SIZE]);
};

#endif  /* __crypto_hmac_sha256_h__ */
