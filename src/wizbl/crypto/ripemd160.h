#ifndef WIZBLCOIN_CRYPTO_RIPEMD160_H
#define WIZBLCOIN_CRYPTO_RIPEMD160_H

#include <stdlib.h>
#include <stdint.h>

class CRIPEMD160
{
public:
    static const size_t OUTPUT_SIZE = 20;

    CRIPEMD160();
    CRIPEMD160& Write(const unsigned char* data, size_t len);
    CRIPEMD160& Reset();
    void Finalize(unsigned char hash[OUTPUT_SIZE]);
private:
    uint32_t s[5];
    uint64_t bytes;
    unsigned char buf[64];

};

#endif
                                                                                                                  