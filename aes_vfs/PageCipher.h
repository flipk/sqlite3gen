
#include <string.h>
#include <inttypes.h>
#include <string>
#include <stdio.h>

#include <mbedtls/aes.h>
#include <mbedtls/sha256.h>
#include <mbedtls/md.h>

namespace AES_VFS {

class PageCipher
{
    unsigned char file_key[32];
    mbedtls_aes_context  aesenc_ctx;
    mbedtls_aes_context  aesdec_ctx;
    mbedtls_md_context_t hmac_md_ctx;
    void make_iv(unsigned char IV_plus_sha256[32], uint32_t pgno);
public:
    static const int PAGE_SIZE = 4096;
    static const int PAGE_SIZE_DISK = (4096 + 32); // HMAC overhead
    PageCipher(void);
    ~PageCipher(void);
    void setKey(const std::string &password);
    void encrypt_page(uint32_t page_number, uint8_t * out, const uint8_t * in);
    bool decrypt_page(uint32_t page_number, uint8_t * out, const uint8_t * in);

};

}; // namespace AES_VFS
