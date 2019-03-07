
#include "DiskPage.h"

namespace AES_VFS {

class PageCache
{
    PageCipher * cipher;
    int fd;
    int max_pages;
    WaitUtil::Lockable lock;
    DiskPageList_t page_list;
    DiskPageLRU_t  page_lru;
    DiskPageHash_t page_hash;
public:
    static const int PAGE_SIZE = DiskPage::PAGE_SIZE;
    PageCache(int fd, int max_pages, PageCipher *);
    ~PageCache(void);
    DiskPage * getPage(uint32_t pgno);
    void releasePage(DiskPage *);
    void flush(void);
};

}; // namespace AES_VFS
