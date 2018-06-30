extern "C" {
#include "crypto/ctaes/ctaes.c"
}

#include "crypto/common.h"
#include "aes.h"

#include <string.h>
#include <assert.h>

int CBCDecryptAES128::Decrypt(const unsigned char* data, int size, unsigned char* out) const
{
    return DecryptCBC(dec, iv, data, size, pad, out);
}

CBCDecryptAES128::~CBCDecryptAES128()
{
    memset(iv, 0, AES_BLOCKCOUNT);
}

CBCDecryptAES128::CBCDecryptAES128(const unsigned char key[AES128_SIZE], const unsigned char ivIn[AES_BLOCKCOUNT], bool padIn)
    : dec(key), pad(padIn)
{
    memcpy(iv, ivIn, AES_BLOCKCOUNT);
}

int CBCEncryptAES128::Encrypt(const unsigned char* data, int size, unsigned char* out) const
{
    return EncryptCBC(enc, iv, data, size, pad, out);
}

CBCEncryptAES128::~CBCEncryptAES128()
{
    memset(iv, 0, AES_BLOCKCOUNT);
}

CBCEncryptAES128::CBCEncryptAES128(const unsigned char key[AES128_SIZE], const unsigned char ivIn[AES_BLOCKCOUNT], bool padIn)
    : enc(key), pad(padIn)
{
    memcpy(iv, ivIn, AES_BLOCKCOUNT);
}

CBCDecryptAES256::~CBCDecryptAES256()
{
    memset(iv, 0, sizeof(iv));
}

int CBCDecryptAES256::Decrypt(const unsigned char* data, int size, unsigned char* out) const
{
    return DecryptCBC(dec, iv, data, size, pad, out);
}

CBCDecryptAES256::CBCDecryptAES256(const unsigned char key[AES256_SIZE], const unsigned char ivIn[AES_BLOCKCOUNT], bool padIn)
    : dec(key), pad(padIn)
{
    memcpy(iv, ivIn, AES_BLOCKCOUNT);
}

CBCEncryptAES256::~CBCEncryptAES256()
{
    memset(iv, 0, sizeof(iv));
}

int CBCEncryptAES256::Encrypt(const unsigned char* data, int size, unsigned char* out) const
{
    return EncryptCBC(enc, iv, data, size, pad, out);
}

CBCEncryptAES256::CBCEncryptAES256(const unsigned char key[AES256_SIZE], const unsigned char ivIn[AES_BLOCKCOUNT], bool padIn)
    : enc(key), pad(padIn)
{
    memcpy(iv, ivIn, AES_BLOCKCOUNT);
}

template <typename T>
static int EncryptCBC(const T& enc, const unsigned char iv[AES_BLOCKCOUNT], const unsigned char* data, int size, bool pad, unsigned char* out)
{
    int padsize = size % AES_BLOCKCOUNT;

    unsigned char mixed[AES_BLOCKCOUNT];
    int written = 0;

    if (!pad && padsize != 0)
          return 0;

    if (!data || !size || !out)
        return 0;

    memcpy(mixed, iv, AES_BLOCKCOUNT);

    while (written + AES_BLOCKCOUNT <= size) {
        for (int i = 0; i != AES_BLOCKCOUNT; i++)
            mixed[i] ^= *data++;
        enc.Encrypt(out + written, mixed);
        memcpy(mixed, out + written, AES_BLOCKCOUNT);
        written += AES_BLOCKCOUNT;
    }
    if (pad) {
        for (int i = 0; i != padsize; i++)
            mixed[i] ^= *data++;
        for (int i = padsize; i != AES_BLOCKCOUNT; i++)
            mixed[i] ^= AES_BLOCKCOUNT - padsize;
        enc.Encrypt(out + written, mixed);
        written += AES_BLOCKCOUNT;
    }
    return written;
}

template <typename T>
static int DecryptCBC(const T& dec, const unsigned char iv[AES_BLOCKCOUNT], const unsigned char* data, int size, bool pad, unsigned char* out)
{
    int written = 0;
    bool fail = false;
    const unsigned char* prev = iv;

    if (!data || !size || !out)
        return 0;

    if (size % AES_BLOCKCOUNT != 0)
        return 0;

    while (written != size) {
        dec.Decrypt(out, data + written);
        for (int i = 0; i != AES_BLOCKCOUNT; i++)
            *out++ ^= prev[i];
        prev = data + written;
        written += AES_BLOCKCOUNT;
    }

    if (pad) {
        unsigned char padsize = *--out;
        fail = !padsize | (padsize > AES_BLOCKCOUNT);
        padsize *= !fail;

        for (int i = AES_BLOCKCOUNT; i != 0; i--)
            fail |= ((i > AES_BLOCKCOUNT - padsize) & (*out-- != padsize));

        written -= padsize;
    }
    return written * !fail;
}

void DecryptAES256::Decrypt(unsigned char plaintext[16], const unsigned char ciphertext[16]) const
{
    AES256_decrypt(&ctx, 1, plaintext, ciphertext);
}

DecryptAES256::~DecryptAES256()
{
    memset(&ctx, 0, sizeof(ctx));
}

DecryptAES256::DecryptAES256(const unsigned char key[32])
{
    AES256_init(&ctx, key);
}

void EncryptAES256::Encrypt(unsigned char ciphertext[16], const unsigned char plaintext[16]) const
{
    AES256_encrypt(&ctx, 1, ciphertext, plaintext);
}

EncryptAES256::~EncryptAES256()
{
    memset(&ctx, 0, sizeof(ctx));
}

EncryptAES256::EncryptAES256(const unsigned char key[32])
{
    AES256_init(&ctx, key);
}

void DecryptAES128::Decrypt(unsigned char plaintext[16], const unsigned char ciphertext[16]) const
{
    AES128_decrypt(&ctx, 1, plaintext, ciphertext);
}

DecryptAES128::~DecryptAES128()
{
    memset(&ctx, 0, sizeof(ctx));
}

DecryptAES128::DecryptAES128(const unsigned char key[16])
{
    AES128_init(&ctx, key);
}

void EncryptAES128::Encrypt(unsigned char ciphertext[16], const unsigned char plaintext[16]) const
{
    AES128_encrypt(&ctx, 1, ciphertext, plaintext);
}

EncryptAES128::~EncryptAES128()
{
    memset(&ctx, 0, sizeof(ctx));
}

EncryptAES128::EncryptAES128(const unsigned char key[16])
{
    AES128_init(&ctx, key);
}

                                                      