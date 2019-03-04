
#include "dll3.h"
#include <string.h>
#include <unistd.h>

namespace AES_VFS {

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
        off_t o = pageNumber * PAGE_SIZE;
        return ::lseek(fd, o, SEEK_SET) == o;
    }
public:
    static const int PAGE_SIZE = 4096;
    uint8_t buffer[PAGE_SIZE];
    uint32_t pageNumber;
    bool dirty;
    int fd;

    diskPage(int _fd, uint32_t _pg) : fd(_fd), pageNumber(_pg) {
        dirty = false;
        read();
    }
    ~diskPage(void) {
        flush();
    }
    void flush(void) {
        if (dirty)
            write();
        dirty = false;
    }
    bool write(void) {
//        printf("write page %d\n", pageNumber);
        if (!seek()) return false;
        int r = ::write(fd, (void*) buffer, PAGE_SIZE);
        if (r < 0)
            return false;
        return true;
    }
    bool read(void) {
//        printf("read page %d\n", pageNumber);
        if (!seek()) return false;
        int r = ::read(fd, (void*) buffer, PAGE_SIZE);
        if (r < 0)
            return false;
        if (r != PAGE_SIZE)
            memset(buffer + r, 0, PAGE_SIZE - r);
        dirty = false;
        return true;
    }
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
    int fd;
    int max_pages;
    WaitUtil::Lockable lock;
    diskPageList_t page_list;
    diskPageLRU_t  page_lru;
    diskPageHash_t page_hash;
public:
    static const int PAGE_SIZE = diskPage::PAGE_SIZE;
    pageCache(int fd, int max_pages);
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
    diskCache(int fd, int max_pages);
    ~diskCache(void);
    int read(off_t pos, uint8_t *, int);
    int write(off_t pos, uint8_t *, int);
    void flush(void) { pc.flush(); }
};

}; // namespace AES_VFS
