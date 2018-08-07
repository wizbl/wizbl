#ifndef WIZBLCOIN_CRYPTO_SHA256_HMAC_H
#define WIZBLCOIN_CRYPTO_SHA256_HMAC_H

#include <stdlib.h>
#include <stdint.h>
#include "crypto/sha256.h"

class CSHA256_HMAC
{

public:
    static const size_t OUTPUT_SIZE = 32;

    CSHA256_HMAC& Write(const unsigned char* data, size_t len)
    {
        inner.Write(data, len);
        return *this;
    }
    void Finalize(unsigned char hash[OUTPUT_SIZE]);
    CSHA256_HMAC(const unsigned char* key, size_t keylen);

private:
    CSHA256 outer;
    CSHA256 inner;

};

#endif
                                                                                                                                                                                                          