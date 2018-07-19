#ifndef WIZBLCOIN_CRYPTO_SHA512_HMAC_H
#define WIZBLCOIN_CRYPTO_SHA512_HMAC_H

#include <stdlib.h>
#include <stdint.h>
#include "crypto/sha512.h"

class CSHA512_HMAC
{
private:
    CSHA512 outer;
    CSHA512 inner;

public:
    static const size_t OUTPUT_SIZE = 64;

    CSHA512_HMAC& Write(const unsigned char* data, size_t len)
    {
        inner.Write(data, len);
        return *this;
    }
    CSHA512_HMAC(const unsigned char* key, size_t keylen);
    void Finalize(unsigned char hash[OUTPUT_SIZE]);
};

#endif
                                                                                                                                                     