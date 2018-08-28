#ifndef WIZBLCOIN_CRYPTO_SHA512_H
#define WIZBLCOIN_CRYPTO_SHA512_H

#include <stdint.h>
#include <stdlib.h>

class CSHA512
{
private:
    uint64_t s[8];
    unsigned char buf[128];
    uint64_t bytes;

public:
    static const size_t OUTPUT_SIZE = 64;

    CSHA512& Reset();
    void Finalize(unsigned char hash[OUTPUT_SIZE]);
    CSHA512& Write(const unsigned char* data, size_t len);
    CSHA512();
};

#endif
                                                                                                                                                                                                                                                     