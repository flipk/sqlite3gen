
#include "dll3.h"
#include <string.h>
#include <unistd.h>

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
    void encrypt_page(uint64_t page_number, uint8_t * out, const uint8_t * in);
    bool decrypt_page(uint64_t page_number, uint8_t * out, const uint8_t * in);

};

class diskPage;
class diskPageHashComp;
typedef DLL3::List<diskPage,1,false,true> diskPageList_t;
typedef DLL3::List<diskPage,2,false,true> diskPageLRU_t;
typedef DLL3::Hash<diskPage,int,diskPageHashComp,
                   3,false,true> diskPageHash_t;

class diskPage : public diskPageList_t::Links,
                 public diskPageLRU_t::Links,
                 public diskPageHash_t::Links
{
    bool seek(void) {
        off_t o = pageNumber * PAGE_SIZE_DISK;
        return ::lseek(fd, o, SEEK_SET) == o;
    }
    PageCipher * cipher;
    int fd;
public:
    static const int PAGE_SIZE = PageCipher::PAGE_SIZE;
    static const int PAGE_SIZE_DISK = PageCipher::PAGE_SIZE_DISK;
    uint8_t buffer[PAGE_SIZE];
    uint32_t pageNumber;
    bool dirty;

    diskPage(int _fd, uint32_t _pg, PageCipher * _cipher);
    ~diskPage(void);
    void flush(void);
    bool write(void);
    bool read(void);
};

class diskPageHashComp {
public:
    static uint32_t obj2hash(const diskPage &item)
    { return (uint32_t) item.pageNumber; }
    static uint32_t key2hash(const uint32_t &key)
    { return (uint32_t) key; }
    static bool hashMatch(const diskPage &item, const uint32_t &key)
    { return (item.pageNumber == key); }
};

class pageCache
{
    PageCipher * cipher;
    int fd;
    int max_pages;
    WaitUtil::Lockable lock;
    diskPageList_t page_list;
    diskPageLRU_t  page_lru;
    diskPageHash_t page_hash;
public:
    static const int PAGE_SIZE = diskPage::PAGE_SIZE;
    pageCache(int fd, int max_pages, PageCipher *);
    ~pageCache(void);
    diskPage * getPage(uint32_t pgno);
    void releasePage(diskPage *);
    void flush(void);
};

class diskCache
{
    pageCache  pc;
    struct pginfo {
        diskPage *pg;
        uint32_t pgno;
        int offset_in_page;
        int offset_in_arg;
        int size_in_page;
    };
    bool getPages(std::vector<pginfo> &pgs, off_t pos, int size);
public:
    static const int PAGE_SIZE = diskPage::PAGE_SIZE;
    diskCache(int fd, int max_pages, PageCipher *_cipher);
    ~diskCache(void);
    void setKey(const std::string &password);
    int read(off_t pos, uint8_t *, int);
    int write(off_t pos, uint8_t *, int);
    void flush(void) { pc.flush(); }
};

}; // namespace AES_VFS
