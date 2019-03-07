
#include "DiskCache.h"

namespace AES_VFS {


DiskCache :: DiskCache(int fd, int max_pages, PageCipher *cipher)
    : pc(fd, max_pages, cipher)
{
}

DiskCache :: ~DiskCache(void)
{
}

bool
DiskCache :: getPages(std::vector<pginfo> &pgs, off_t pos, int size)
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
DiskCache :: read(off_t pos, uint8_t *buf, int size)
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
DiskCache :: write(off_t pos, uint8_t *buf, int size)
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
