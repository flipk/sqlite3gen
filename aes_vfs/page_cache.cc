
#include "page_cache.h"
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <sstream>

namespace AES_VFS {

//////////////////////////// PageCipher

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
PageCipher :: encrypt_page(uint64_t page_number, uint8_t * out, const uint8_t * in)
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
PageCipher :: decrypt_page(uint64_t page_number,
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


//////////////////////////// diskPage

diskPage :: diskPage(int _fd, uint32_t _pg, PageCipher * _cipher)
    : cipher(_cipher), fd(_fd), pageNumber(_pg)
{
    dirty = false;
    read();
}

diskPage :: ~diskPage(void)
{
    flush();
}

void
diskPage :: flush(void)
{
    if (dirty)
        write();
    dirty = false;
}

bool
diskPage :: write(void)
{
    if (!seek())
        return false;

    uint8_t disk_buffer[PAGE_SIZE_DISK];
    cipher->encrypt_page(pageNumber, disk_buffer, buffer);
    int r = ::write(fd, (void*) disk_buffer, PAGE_SIZE_DISK);
    if (r < 0)
        return false;
    return true;
}

bool
diskPage :: read(void)
{
    if (!seek())
        return false;

    uint8_t disk_buffer[PAGE_SIZE_DISK];
    int r = ::read(fd, (void*) disk_buffer, PAGE_SIZE_DISK);
    if (r < 0)
        return false;
    if (r != PAGE_SIZE_DISK)
        memset(buffer, 0, PAGE_SIZE);
    else
        cipher->decrypt_page(pageNumber, buffer, disk_buffer);

    dirty = false;
    return true;
}

//////////////////////////// pageCache

pageCache :: pageCache(int _fd, int _max_pages, PageCipher * _cipher)
    : cipher(_cipher), fd(_fd), max_pages(_max_pages)
{
    // nothing
}

pageCache :: ~pageCache(void)
{
    WaitUtil::Lock  l(&lock);
    diskPage *pg;
    while ((pg = page_list.dequeue_head()) != NULL)
    {
        if (page_lru.onthislist(pg))
            page_lru.remove(pg);
        page_hash.remove(pg);
        delete pg;
    }
}

diskPage *
pageCache :: getPage(uint32_t pgno)
{
    WaitUtil::Lock  l(&lock);
    diskPage * pg;
    pg = page_hash.find(pgno);
    if (pg)
    {
        if (page_lru.onthislist(pg))
            page_lru.remove(pg);
        return pg;
    }
    pg = new diskPage(fd, pgno, cipher);
    page_list.add_tail(pg);
    page_hash.add(pg);
    return pg;
}

void
pageCache :: releasePage(diskPage *pg)
{
    WaitUtil::Lock  l(&lock);
    if (!page_lru.onthislist(pg))
        page_lru.add_tail(pg);
    while (page_lru.get_cnt() > max_pages)
    {
        pg = page_lru.dequeue_head();
        page_hash.remove(pg);
        page_list.remove(pg);
        delete pg;
    }
}

static bool pgs_sort(diskPage*a,diskPage*b)
{
    return a->pageNumber < b->pageNumber;
}

void
pageCache :: flush(void)
{
    WaitUtil::Lock  l(&lock);
    std::vector<diskPage*> pgs;
    for (diskPage * pg = page_list.get_head();
         pg;
         pg = page_list.get_next(pg))
    {
        pgs.push_back(pg);
    }
    std::sort(pgs.begin(), pgs.end(), pgs_sort);
    for (size_t ind = 0; ind < pgs.size(); ind++)
    {
        pgs[ind]->flush();
    }
}

//////////////////////////// diskCache

diskCache :: diskCache(int fd, int max_pages, PageCipher *cipher)
    : pc(fd, max_pages, cipher)
{
}

diskCache :: ~diskCache(void)
{
}

bool
diskCache :: getPages(std::vector<pginfo> &pgs, off_t pos, int size)
{
    off_t end_offset;
    uint64_t start_page, end_page;
    uint32_t pgno;

    end_offset = pos + size - 1;
    start_page = pos / PAGE_SIZE;
    end_page = end_offset / PAGE_SIZE;

    int offset_in_arg = 0;
    for (pgno = start_page; pgno <= end_page; pgno++)
    {
        pginfo pi;
        pi.pg = pc.getPage(pgno);
        pi.pgno = pgno;
        pi.offset_in_page = pos % PAGE_SIZE;
        pi.offset_in_arg = offset_in_arg;
        int remaining_in_page = PAGE_SIZE - pi.offset_in_page;
        pi.size_in_page =
            (size > remaining_in_page ? remaining_in_page : size);
        pos += pi.size_in_page;
        size -= pi.size_in_page;
        offset_in_arg += pi.size_in_page;
        pgs.push_back(pi);
    }

    return true;
}

int
diskCache :: read(off_t pos, uint8_t *buf, int size)
{
    std::vector<pginfo> pgs;
    getPages(pgs, pos, size);

    for (size_t ind = 0; ind < pgs.size(); ind++)
    {
        pginfo &pi = pgs[ind];
        memcpy(buf + pi.offset_in_arg,
               pi.pg->buffer + pi.offset_in_page,
               pi.size_in_page);
        pc.releasePage(pi.pg);
    }

    return size;
}

int
diskCache :: write(off_t pos, uint8_t *buf, int size)
{
    std::vector<pginfo> pgs;
    getPages(pgs, pos, size);

    for (size_t ind = 0; ind < pgs.size(); ind++)
    {
        pginfo &pi = pgs[ind];
        memcpy(pi.pg->buffer + pi.offset_in_page,
               buf + pi.offset_in_arg,
               pi.size_in_page);
        pi.pg->dirty = true;
        pc.releasePage(pi.pg);
    }

    return size;
}

}; // namespace AES_VFS
