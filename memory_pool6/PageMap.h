#ifndef PAGEMAP_H_
#define PAGEMAP_H_

#include"Define.h"
#include<atomic>
#include<cstddef>
#include<cstdlib>
#include<unistd.h>
#include<mmap>

class Descriptor;

struct PageInfo
{
    Descriptor *desc;
};

class PageMap
{
private:
    void init();
    bool initialized = false;
    std::atomic<PageInfo>* _pagemap = {nullptr};

    size_t addrToKey(char* ptr) const;

public:
    PageInfo getPageInfo(char* ptr);
    void setPageInfo(char *ptr, PageInfo info);
};

inline size_t PageMap::addrToKey(char* ptr) const
{
    size_t key = ((size_t)ptr >> PM_KEY_SHIFT) & PM_KEY_MASK;
    return key;
}

inline PageInfo PageMap::getPageInfo(char* ptr)
{
    if (UNLIKELY(!initialized))
        init();

    size_t key = addrToKey(ptr);
    return _pagemap[key].load();
}

inline void PageMap::setPageInfo(char* ptr, PageInfo info)
{
    if (UNLIKELY(!initialized))
        init();

    size_t key = addrToKey(ptr);
    _pagemap[key].store(info);
}


#define PM_SZ ((1ULL << PM_SB) * sizeof(PageInfo))

void* PageAlloc(size_t size)
{
    ASSERT((size & PAGE_MASK) == 0);

    void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANON | MAP_NORESERVE, -1, 0);
    if (ptr == MAP_FAILED)
        ptr = nullptr;
 
    return ptr;
}

inline void PageMap::init()
{
    ASSERT(!initialized);
    initialized = true;

    _pagemap = (std::atomic<PageInfo>*)PageAlloc(PM_SZ);
    ASSERT(_pagemap);
}

extern PageMap sPageMap;

#endif