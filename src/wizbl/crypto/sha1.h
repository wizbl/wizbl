#ifndef WIZBLCOIN_CRYPTO_SHA1_H
#define WIZBLCOIN_CRYPTO_SHA1_H

#include <stdlib.h>
#include <stdint.h>

class CSHA1
{
private:
    unsigned char buf[64];
    uint64_t bytes;
    uint32_t s[5];

public:
    static const size_t OUTPUT_SIZE = 20;

    CSHA1& Reset();
    CSHA1& Write(const unsigned char* data, size_t len);
    void Finalize(unsigned char hash[OUTPUT_SIZE]);
    CSHA1();
};

#endif
                                                                                                                                                                                              