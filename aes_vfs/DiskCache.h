
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
    off_t file_size;
public:
    static const int PAGE_SIZE = DiskPage::PAGE_SIZE;
    static const int PAGE_SIZE_DISK = DiskPage::PAGE_SIZE_DISK;
    DiskCache(int fd, int max_pages, PageCipher *_cipher);
    ~DiskCache(void);
    void setKey(const std::string &password);
    int read(off_t pos, uint8_t *, int);
    int write(off_t pos, uint8_t *, int);
    void flush(void) { pc.flush(); }
    off_t getFileSize(void) { return file_size; }
    void truncate(off_t size) { pc.truncate(size); file_size = size; }
};

}; // namespace AES_VFS
