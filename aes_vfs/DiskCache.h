
#include "PageCache.h"

namespace AES_VFS {

class DiskCache
{
    PageCache  pc;
    struct pginfo {
        DiskPage *pg;
        uint32_t pgno;
        int offset_in_page;
        int offset_in_arg;
        int size_in_page;
    };
    bool getPages(std::vector<pginfo> &pgs, off_t pos, int size);
public:
    static const int PAGE_SIZE = DiskPage::PAGE_SIZE;
    DiskCache(int fd, int max_pages, PageCipher *_cipher);
    ~DiskCache(void);
    void setKey(const std::string &password);
    int read(off_t pos, uint8_t *, int);
    int write(off_t pos, uint8_t *, int);
    void flush(void) { pc.flush(); }
};

}; // namespace AES_VFS
