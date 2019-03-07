
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
        pgs.push_back(pg);
    }
    std::sort(pgs.begin(), pgs.end(), pgs_sort);
    for (size_t ind = 0; ind < pgs.size(); ind++)
    {
        pgs[ind]->flush();
    }
}

}; // namespace AES_VFS
