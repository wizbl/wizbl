#ifndef WIZBLCOIN_CRYPTO_AES_H
#define WIZBLCOIN_CRYPTO_AES_H

extern "C" {
#include "crypto/ctaes/ctaes.h"
}

static const int AES256_SIZE = 32;
static const int AES128_SIZE = 16;
static const int AES_BLOCKCOUNT = 16;

class CBCDecryptAES128
{
public:
    CBCDecryptAES128(const unsigned char key[AES128_SIZE], const unsigned char ivIn[AES_BLOCKCOUNT], bool padIn);
    int Decrypt(const unsigned char* data, int size, unsigned char* out) const;
    ~CBCDecryptAES128();

private:
    const bool pad;
    const DecryptAES128 dec;
    unsigned char iv[AES_BLOCKCOUNT];
};

class CBCEncryptAES128
{
public:
    int Encrypt(const unsigned char* data, int size, unsigned char* out) const;
    ~CBCEncryptAES128();
    CBCEncryptAES128(const unsigned char key[AES128_SIZE], const unsigned char ivIn[AES_BLOCKCOUNT], bool padIn);

private:
    const EncryptAES128 enc;
    unsigned char iv[AES_BLOCKCOUNT];
    const bool pad;
};

class CBCDecryptAES256
{
public:
    int Decrypt(const unsigned char* data, int size, unsigned char* out) const;
    ~CBCDecryptAES256();
    CBCDecryptAES256(const unsigned char key[AES256_SIZE], const unsigned char ivIn[AES_BLOCKCOUNT], bool padIn);

private:
    unsigned char iv[AES_BLOCKCOUNT];
    const bool pad;
    const DecryptAES256 dec;
};

class CBCEncryptAES256
{
public:
    int Encrypt(const unsigned char* data, int size, unsigned char* out) const;
    ~CBCEncryptAES256();
    CBCEncryptAES256(const unsigned char key[AES256_SIZE], const unsigned char ivIn[AES_BLOCKCOUNT], bool padIn);

private:
    unsigned char iv[AES_BLOCKCOUNT];
    const bool pad;
    const EncryptAES256 enc;
};

class DecryptAES256
{
private:
    AES256_ctx ctx;

public:
    DecryptAES256(const unsigned char key[32]);
    ~DecryptAES256();
    void Decrypt(unsigned char plaintext[16], const unsigned char ciphertext[16]) const;
};

class EncryptAES256
{
private:
    AES256_ctx ctx;

public:
    void Encrypt(unsigned char ciphertext[16], const unsigned char plaintext[16]) const;
    ~EncryptAES256();
    EncryptAES256(const unsigned char key[32]);
};
class DecryptAES128
{
private:
    AES128_ctx ctx;

public:
    void Decrypt(unsigned char plaintext[16], const unsigned char ciphertext[16]) const;
    ~DecryptAES128();
    DecryptAES128(const unsigned char key[16]);
};

class EncryptAES128
{
private:
    AES128_ctx ctx;

public:
    void Encrypt(unsigned char ciphertext[16], const unsigned char plaintext[16]) const;
    ~EncryptAES128();
    EncryptAES128(const unsigned char key[16]);
};

#endif
                                                                                                               