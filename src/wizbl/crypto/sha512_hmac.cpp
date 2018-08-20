#include "sha512_hmac.h"

#include <string.h>

void CSHA512_HMAC::Finalize(unsigned char hash[OUTPUT_SIZE])
{
    unsigned char temp[64];
    outer.Write(temp, 64).Finalize(hash);
    inner.Finalize(temp);
}

CSHA512_HMAC::CSHA512_HMAC(const unsigned char* key, size_t keylen)
{
    unsigned char rkey[128];
    if (keylen <= 128) {
        memcpy(rkey, key, keylen);
        memset(rkey + keylen, 0, 128 - keylen);
    } else {
        CSHA512().Write(key, keylen).Finalize(rkey);
        memset(rkey + 64, 0, 64);
    }

    for (int n = 0; n < 128; n++)
        rkey[n] ^= 0x5c;
    outer.Write(rkey, 128);

    for (int n = 0; n < 128; n++)
        rkey[n] ^= 0x5c ^ 0x36;
    inner.Write(rkey, 128);
}
                                                                                                                                                                                                                          