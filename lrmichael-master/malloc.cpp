#include <cassert>
#include <cstring>
#include <algorithm>
#include <atomic>

// for ENOMEM
#include <errno.h>

#include "lrmichael.h"
#include "size_classes.h"
#include "pages.h"
#include "pagemap.h"
#include "log.h"

void* MallocFromActive(ProcHeap* heap)
{
    ActiveDescriptor* oldActive = heap->active.load();
    ActiveDescriptor* newActive;
    uint64_t oldCredits;
    do
    {
        if(!oldActive)
            return nullptr;

        Descriptor* oldDesc;
        GetActive(oldActive, &oldDesc, &oldCredits);

        newActive = nullptr;
        if(oldCredits > 0)
        {
            newActive = MakeActive(oldDesc, oldCredits - 1);
        }
    }while(!heap->active.compare_exchange_weak(oldActive, newActive));

    Descriptor* desc = (Descriptor*)((uint64_t)oldActive & ~CREDITS_MASK);

    LOG_DEBUG("Heap %p, Desc %p", heap, desc);

    char* ptr = nullptr;
    uint64_t credits = 0;

    Anchor oldAnchor = desc->anchor.load();
    Anchor newAnchor;
    do
    {
        assert(oldAnchor.avail < desc->maxcount);

        uint64_t blockSize = desc->blockSize;
        uint64_t avail = oldAnchor.avail;
        uint64_t offset = avail * blockSize;
        ptr = (desc->superblock + offset);

        uint64_t next = *(uint64_t*)ptr;
        ptr = (desc->superblock + offset);

        uint64_t next = *(uint64_t*)ptr;

        newAnchor = oldAnchor;
        newAnchor.avail = next;
        newAnchor.tag++;

        if(oldCredits == 0)
        {
            if(oldAnchor.count == 0)
                newAnchor.state = SB_FULL;
            else
            {
                    //尽可能的保留
                credits = std::min<uint64_t>(oldAnchor.count, CREDITS_MAX);
                newAnchor.count -= credits;
            }
        }
    }while(!desc->anchor.compare_exchange_weak(oldAnchor, newAnchor));

    assert(newAnchor.avail < desc->maxcount || (oldCredits == 0));

    if(credits > 0)
        UpdateActive(heap, desc, credits);

    LOG_DEBUG("Heap %p, Desc %p, ptr %p",heap, desc, ptr);

    return (void*)ptr;
}

void UpdateActive(ProcHeap* heap, Descriptor* desc, uint64_t credits)
{
    ActiveDescriptor* oldActive = heap->active.load();
    ActiveDescriptor* newActive = MakeActive(desc, credits - 1);

    //将新的Active superblock标志
    if(heap->active.compare_exchange_strong(oldActive, newAcitve))
        return;

        // someone installed another active superblock
    // return credits to superblock, make it SB_PARTIAL
    // (because the superblock is no longer active but has available blocks)
    //如果失败，说明有其他线程设置了active superblock，则将credits加回来，将状态设为partial
    else
    {
        Anchor oldAnchor = desc->anchor.load();
        Anchor newAnchor;
        do
        {
            newAnchor = oldAnchor;
            newAnchor.count += credits;
            newAnchor.state = SB_PARTIAL;
        }
        while(!desc->anchor.compare_exchange_weak(oldAnchor, newAnchor));
    }

    HeapPushPartial(desc);
}

Descriptor* ListPopPartial(ProHeap* heap)
{
    DescriptorNode oldHead = heap->partialList.load();
    DescriptorNode newHead;
    do
    {
        if(!oldHead.desc)
            return nullptr;
        
        newHead = oldHead.desc->nexPartial.load();
        newHead.counter = oldHead.counter;
    }while(!heap->partialList.compare_exchange_weak(oldHead, newHead));

    return oldHead.desc;
}

void ListPushPartial(Descriptor* desc)
{
    ProcHeap* heap = desc->heap;

    DescriptorNode oldHead = heap->partialList.load();
    DescriptorNode newHead = { desc, oldHead.counter + 1 };
    do
    {
        newHead.desc->nextPartial.store(oldHead); 
    }
    while (!heap->partialList.compare_exchange_weak(
                oldHead, newHead));
}

void HeapPushPartial(Descriptor* desc)
{
    ListPushPartial(desc);
}

Descriptor* HeapPopPartial(ProcHeap* heap)
{
    return ListPopPartial();
}

void* MallocFromPartial(ProcHeap* heap)
{
    Descriptor* desc = HeapPopPartial(heap);
    if(!desc)
        return nullptr;

    Anchor oldAnchor = desc->anchor.load();
    Anchor newAnchor;
    uint64_t credits = 0;

    do
    {
        if(oldAnchor.state == SB_EMPTY)
        {
            DescRetire(desc);
            return MallocFromPartial(heap);
        }

        //尽可能多的保留block，取一个block给线程使用
        credits = std::min<uint64_t>(oldAnchor.count - 1, CREDITS_MAX);
        newAnchor = oldAnchor;
        newAnchor.count -= 1;
        newAnchor.count -= credits;
        newAnchor.state = (credits > 0) ? SB_ACTIVE : SB_FULL;
    }while(!desc->anchor.compare_exchange_weak(oldAnchor, newAnchor));

    ASSERT(newAnchor.count < desc->maxcount);

    char* ptr = nullptr;
    oldAnchor = desc->anchor.load();

    do
    {
        uint64_t idx = oldAnchor.avail; 
        uint64_t blockSize = desc->heap->sizeclass->blockSize;
        ptr = desc->siperblock + idx * blockSize;

        newAnchor = oldAnchor;
        newAnchor.avail = *(uint64_t*)ptr;
        newAnchor.tag++;
    }while(!desc->anchor.compare_exchange_weak(oldAnchor, newAnchor));

    assert(newAnchor.avail < desc->maxcount || newAnchor.state == SB_FULL)

    if(credits > 0)
        UpdateActive(heap, desc, credits);
    
    return (void*)ptr;
}

void* MallocFromNewSB(ProcHeap* heap)
{
    SizeClassData const* sc = heap->sizeclass;

    Descriptor* desc = DescAlloc();
    assert(desc);

    desc->heap = heap;
    desc->blockSize = sc->blockSize;
    desc->maxcount = sc->GetBlockNum();

    desc->superblock = (char*)PageAlloc(sc->sbSize);

    uint64_t const blockSize = sc->blockSize;
    for(uint64_t idx = 1; idx < desc->maxcount - 1; ++idx)
    {
        *(uint64_t*)(desc->superblock + idx * blockSize) = (idx + 1);
    }

    uint64_t credits = std::min<uint64_t>(desc->maxcount - 1, CREDITS_MAX);
    ActiveDescriptor* newActive = MakeActive(desc, credits -1);

    Anchor anchor;
    anchor.avail = 1;
    anchor.count = (desc->maxcount - 1) - credits;
    anchor.state = SB_ACTIVE;
    anchor.tag = 0;

    desc->anchor.store(anchor);

    assert(anchor.avail < desc->maxcount);
    assert(anchor.count < desc->maxcount);

    RegisterDesc(desc);

    ActiveDescriptor* oldActive = heap->active.load();
    if(oldActive || !heap->active.compare_exchange_strong(oldActive, newActive))
    {
        UnregisterDesc(desc->heap, desc->siperblock);
        PageFree(desc->superblock, sc->sbSize);
        DescRetire(desc);
        return nullptr;
    }

    char* ptr = desc->superblock;
    LOG_DEBUG("desc:%p, ptr:%p", desc, ptr);
    return (void*)ptr;
}

Descriptor* DescAlloc()
{
    DescriptorNode oldHead = AvailDesc.load();
    while(true)
    {
        if(oldHead.desc)
        {
            DescriptorNode newHead = oldHead.desc->nextFree.load();
            newHead.counter = oldHead.counter;
            if(AvailDesc.compare_exchange_weak(oldHead, newHead))
                return oldHead.desc;
        }
        else
        {
            //分配一些页
            //得到第一个描述符，被返回调用
            char* ptr = (char*)PageAlloc(DESCRIPTOR_BLOCK_SZ);
            descriptor* ret = (Descriptor*) ptr;
            //将剩余的描述符用链表连起来，添加可用的描述符
            {
                Descriptor* first = nullptr;
                Descriptor* prev = nullptr;

                char* currptr = ptr + sizeof(Descriptor);
                currPtr = ALIGN_ADDR(currptr, CACHELINE);
                first = (Descriptor*)currPtr;
                while(currPtr + sizeof(Descriptor) < ptr + DESCRIPTOR_BLOCK_SZ)
                {
                    Descriptor* curr = (Descriptor*)currPtr;
                    if(prev)
                    {
                        prev->nextFree.store({curr, 0});
                    }

                    prev = curr;
                    currPtr = currPtr + sizeof(Descriptor);
                    currPtr = ALIGN_ADDR(currPtr, CACHELINE);
                }

                prev->nextFree.store({nullptr, 0});

                DescriptorNode oldhead = AvailDesc.load();
                Descriptor newHead;
                do
                {
                    prev->nextFree.store(oldHead);
                    newHead.desc = first;
                    newHead.counter = oldHead.counter + 1;
                }while(!AvailDesc.compare_exchange_weak(oldHead, newHead));
            }
            return ret;
        }
    }
}

void DescRetire(Descriptor* desc)
{
    DescriptorNode oldHead = AvailDesc.load();
    DescriptorNode newHead;
    do
    {
        desc->nextFree.store(oldHead);
        newHead.desc = desc;
        newHead.counter = oldHead.counter + 1;
    }while(!AvailDesc.compare_exchange_weak(oldHead, newHead));
}

void* lr_malloc(size_t size) noexcept
{
    LOG_DEBUG("size: %lu", size);

    ProHeap* heap = GetProHeap(size);
    if(UNLIKELY(!heap))
    {
        size_t pages = PAGE_CEILING(size);
        Descriptor* desc = DescAlloc();
        assert(desc);

        desc->heap = nullptr;
        desc->blockSize = pages;
        desc->maxcount  = 1;
        desc->superblock = (char*)PageAlloc(pages);

        Anchor anchor;
        anchor.avail = 0;
        anchor.count = 0;
        anchor.state = SB_FULL;
        anchor.tag = 0;

        desc->anchor.store(anchor);
        RegisterDesc(desc);

        char* ptr = desc->superblock;
        LOG_DEBUG("large, ptr = %p", ptr);
        return (void*)ptr;
    }
    while(1)
    {
        if (void* ptr = MallocFromActive(heap))
        {
            LOG_DEBUG("MallocFromActive, ptr: %p", ptr);
            return ptr;
        }

        if (void* ptr = MallocFromPartial(heap))
        {
            LOG_DEBUG("MallocFromPartial, ptr: %p", ptr);
            return ptr;
        }

        if (void* ptr = MallocFromNewSB(heap))
        {
            LOG_DEBUG("MallocFromNewSB, ptr: %p", ptr);
            return ptr;
        }
    }
}