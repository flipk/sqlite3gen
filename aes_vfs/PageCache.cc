
#include "PageCache.h"
#include <vector>
#include <algorithm>
#include <stdio.h>

namespace AES_VFS {

PageCache :: PageCache(int _fd, int _max_pages, PageCipher * _cipher)
    : cipher(_cipher), fd(_fd), max_pages(_max_pages)
{
    // nothing
}

PageCache :: ~PageCache(void)
{
    WaitUtil::Lock  l(&lock);
    DiskPage *pg;
    while ((pg = page_list.dequeue_head()) != NULL)
    {
        if (page_lru.onthislist(pg))
            page_lru.remove(pg);
        page_hash.remove(pg);
        delete pg;
    }
}

DiskPage *
PageCache :: getPage(uint32_t pgno)
{
    WaitUtil::Lock  l(&lock);
    DiskPage * pg;
    pg = page_hash.find(pgno);
    if (pg)
    {
        if (page_lru.onthislist(pg))
            page_lru.remove(pg);
        return pg;
    }
    pg = new DiskPage(fd, pgno, cipher);
    page_list.add_tail(pg);
    page_hash.add(pg);
    return pg;
}

void
PageCache :: releasePage(DiskPage *pg)
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

static bool pgs_sort(DiskPage*a,DiskPage*b)
{
    return a->pageNumber < b->pageNumber;
}

void
PageCache :: flush(void)
{
    WaitUtil::Lock  l(&lock);
    std::vector<DiskPage*> pgs;
    for (DiskPage * pg = page_list.get_head();
         pg;
         pg = page_list.get_next(pg))
    {
        if (pg->dirty)
            pgs.push_back(pg);
    }
    std::sort(pgs.begin(), pgs.end(), pgs_sort);
    for (size_t ind = 0; ind < pgs.size(); ind++)
    {
        pgs[ind]->flush();
    }
}

void
PageCache :: truncate(off_t size)
{
    uint32_t pgno = (size + (PAGE_SIZE-1)) / PAGE_SIZE;
    DiskPage * pg, * npg;
    for (pg = page_list.get_head();
         pg;
         pg = npg)
    {
        npg = page_list.get_next(pg);

        if (pg->pageNumber > pgno)
        {
            if (page_lru.onthislist(pg))
                page_lru.remove(pg);
            page_list.remove(pg);
            page_hash.remove(pg);
            // if it's dirty, don't waste time
            // flushing it, we're going to truncate
            // prior to this page's point anyway.
            pg->dirty = false;
            delete pg;
        }
    }

    ftruncate(fd, size);
}

}; // namespace AES_VFS
