#include <openssl/crypto.h>
#include "cleanse.h"

void mem_cleanse(void *ptr, size_t len)
{
    OPENSSL_cleanse(ptr, len);
}
                                                                                      