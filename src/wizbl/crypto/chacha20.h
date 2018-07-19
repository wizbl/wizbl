#ifndef WIZBLCOIN_CRYPTO_CHACHA20_H
#define WIZBLCOIN_CRYPTO_CHACHA20_H

#include <stdlib.h>
#include <stdint.h>

class ChaCha20
{
private:
    uint32_t input[16];

public:
    ChaCha20();
    ChaCha20(const unsigned char* key, size_t keylen);
    void Output(unsigned char* output, size_t bytes);
    void Seek(uint64_t pos);
    void SetIV(uint64_t iv);
    void SetKey(const unsigned char* key, size_t keylen);
};

#endif
                                                                                                                                                