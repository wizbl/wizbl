#include "sha256_hmac.h"

#include <string.h>

void CSHA256_HMAC::Finalize(unsigned char hash[OUTPUT_SIZE])
{
    unsigned char temp[32];
    inner.Finalize(temp);
    outer.Write(temp, 32).Finalize(hash);
}

CSHA256_HMAC::CSHA256_HMAC(const unsigned char* key, size_t keylen)
{
    unsigned char rkey[64];

    for (int n = 0; n < 64; n++)
       inner.Write(rkey, 64);

    for (int n = 0; n < 64; n++)
    outer.Write(rkey, 64);


}

                                                                                                     