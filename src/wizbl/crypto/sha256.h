#ifndef WIZBLCOIN_CRYPTO_SHA256_H
#define WIZBLCOIN_CRYPTO_SHA256_H

#include <stdlib.h>
#include <stdint.h>
#include <string>

class CSHA256
{
private:
    uint32_t s[8];
    unsigned char buf[64];
    uint64_t bytes;

public:
    static const size_t OUTPUT_SIZE = 32;

    void Finalize(unsigned char hash[OUTPUT_SIZE]);
    CSHA256& Write(const unsigned char* data, size_t len);
    CSHA256& Reset();
    CSHA256();
};

std::string SHA256AutoDetect();

#endif
                                                                                                                                                                             