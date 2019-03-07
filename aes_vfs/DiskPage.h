
#include "PageCipher.h"

#include "dll3.h"
#include <unistd.h>

namespace AES_VFS {

class DiskPage;
class DiskPageHashComp;
typedef DLL3::List<DiskPage,1,false,true> DiskPageList_t;
typedef DLL3::List<DiskPage,2,false,true> DiskPageLRU_t;
typedef DLL3::Hash<DiskPage,int,DiskPageHashComp,
                   3,false,true> DiskPageHash_t;

class DiskPage : public DiskPageList_t::Links,
                 public DiskPageLRU_t::Links,
                 public DiskPageHash_t::Links
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

    DiskPage(int _fd, uint32_t _pg, PageCipher * _cipher);
    ~DiskPage(void);
    void flush(void);
    bool write(void);
    bool read(void);
};

class DiskPageHashComp {
public:
    static uint32_t obj2hash(const DiskPage &item)
    { return (uint32_t) item.pageNumber; }
    static uint32_t key2hash(const uint32_t &key)
    { return (uint32_t) key; }
    static bool hashMatch(const DiskPage &item, const uint32_t &key)
    { return (item.pageNumber == key); }
};

}; // namespace AES_VFS
