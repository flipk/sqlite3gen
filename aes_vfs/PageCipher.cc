
#include <unistd.h>

#include "PageCipher.h"

namespace AES_VFS {

PageCipher :: PageCipher(void)
{
    mbedtls_aes_init( &aesenc_ctx );
    mbedtls_aes_init( &aesdec_ctx );
    mbedtls_md_init( &hmac_md_ctx );
}

PageCipher :: ~PageCipher(void)
{
    mbedtls_aes_free( &aesenc_ctx );
    mbedtls_aes_free( &aesdec_ctx );
    mbedtls_md_free( &hmac_md_ctx );
}

void
PageCipher :: setKey(const std::string &password)
{
    mbedtls_sha256( (const unsigned char *) password.c_str(),
                    password.length(),
                    file_key, 0/*use SHA256*/);
    mbedtls_aes_setkey_enc( &aesenc_ctx, file_key, 256 );
    mbedtls_aes_setkey_dec( &aesdec_ctx, file_key, 256 );
    mbedtls_md_setup( &hmac_md_ctx,
                      mbedtls_md_info_from_type( MBEDTLS_MD_SHA256 ),
                      /*use hmac*/ 1);
    mbedtls_md_hmac_starts( &hmac_md_ctx, file_key, 32 );
}

void
PageCipher :: make_iv(unsigned char IV_plus_sha256[32], uint32_t pgno)
{
    unsigned char page_key[32];
    memcpy(page_key, file_key, 32);
    uint32_t * pageptr = (uint32_t *) page_key;
    pageptr[0] ^=  pgno;
    pageptr[1] ^= (pgno ^ 0xFFFFFFFF);
    pageptr[2] ^= (pgno ^ 0xFFFF0000);
    pageptr[3] ^= (pgno ^ 0x0000FFFF);
    mbedtls_sha256( page_key, 32, IV_plus_sha256, 0/*use SHA256*/);
    for (int ind = 0; ind < 16; ind++)
        IV_plus_sha256[ind] ^= IV_plus_sha256[ind+16];
}

void
PageCipher :: encrypt_page(uint32_t page_number, uint8_t * out, const uint8_t * in)
{
    unsigned char IV[32];
    make_iv(IV, page_number);
    mbedtls_aes_crypt_cbc( &aesenc_ctx, MBEDTLS_AES_ENCRYPT,
                   PAGE_SIZE, IV, in, out);
    mbedtls_md_hmac_reset( &hmac_md_ctx );
    mbedtls_md_hmac_update( &hmac_md_ctx, out, PAGE_SIZE);
    mbedtls_md_hmac_finish( &hmac_md_ctx, out + PAGE_SIZE );
}

bool
PageCipher :: decrypt_page(uint32_t page_number,
                           uint8_t * out, const uint8_t * in)
{
    bool ret = true;
    unsigned char IV[32];
    make_iv(IV, page_number);
    uint8_t  hmac_buf[32];
    mbedtls_md_hmac_reset( &hmac_md_ctx );
    mbedtls_md_hmac_update( &hmac_md_ctx, in, PAGE_SIZE);
    mbedtls_md_hmac_finish( &hmac_md_ctx, hmac_buf );

    if (memcmp(hmac_buf, in + PAGE_SIZE, 32) != 0)
    {
        printf("PageCipher :: decrypt_page : HMAC FAILURE!\n");
        ret = false;
    }
    mbedtls_aes_crypt_cbc( &aesdec_ctx, MBEDTLS_AES_DECRYPT,
                   PAGE_SIZE, IV, in, out);
    return ret;
}

}; // namespace AES_VFS
