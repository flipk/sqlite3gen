
#include "page_cache.h"

namespace AES_VFS {

//////////////////////////// pageCache

pageCache :: pageCache(int _fd, int _max_pages)
    : fd(_fd), max_pages(_max_pages)
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
    pg = new diskPage(fd, pgno);
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

void
pageCache :: flush(void)
{
    WaitUtil::Lock  l(&lock);
    for (diskPage * pg = page_list.get_head();
         pg;
         pg = page_list.get_next(pg))
    {
        pg->flush();
    }
}

//////////////////////////// diskCache

diskCache :: diskCache(int fd, int max_pages)
    : pc(fd, max_pages)
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
    int ind;

    for (ind = 0; ind < pgs.size(); ind++)
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
    int ind;

    for (ind = 0; ind < pgs.size(); ind++)
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
