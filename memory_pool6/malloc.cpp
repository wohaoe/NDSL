#include"Block.h"
#include"Descriptor.h"
#include"PageMap.h"
#include"Anchor.h"
#include"Active.h"
#include"Heap.h"

#include <cassert>
#include <cstring>
#include <algorithm>
#include <atomic>
#include <errno.h>


std::atomic<DescriptorNode> AvailDesc({ nullptr, 0 });

enum SuperBlockState
{
    SB_ACTIVE = 0,
    SB_FULL = 1,
    SB_PARTIAL = 2,
    SB_EMPTY = 3,
};


Block* mallocFromActive(Heap* heap)
{
    Active* oldActive = heap->active.load();
    Active* newActive = nullptr;

    do
    {
        if(!oldActive)
            return nullptr;
        
        if(oldActive->credit > 0)
        {
            newActive = new Active(oldActive->desc, oldActive->credit - 1);
        }
    }while(!heap->active.compare_exchange_weak(oldActive, newActive));

    Anchor oldAnchor = oldActive->desc.load();
    Anchor newAnchor;
    Descriptor* desc = (Descriptor*)oldActive->desc;

    Block* mallocBlock;
    size_t moreCredits = 0;

    do
    {
        ASSERT(oldAnchor.avail < desc->superBlock->maxCountOfBlocks);

        size_t blockSize = desc->superBlock->sizeClass;
        uint64_t avail = oldAnchor.avail;
        uint64_t offset = avail * blockSize;

        mallocBlock = (Block*)(desc->superBlock + offset);
        
        uint64_t next = *(uint64_t *)mallocBlock;

        newAnchor = oldAnchor;
        newAnchor.avail = next
        newAnchor.tag++;

        //最后一个可分配的block
        if(oldActive->credit == 0)
        {
            if(oldAnchor.count == 0)
            {
                newAnchor.state = SB_FULL;
            }
            else
            {
                moreCredits = std::min<uint64_t>(oldAnchor.count, CREDITS_MAX);
                newAnchor.count -= moreCredits;
            }
        }
    }while(!desc->anchor.compare_exchange_weak(oldAnchor, newAnchor));

    ASSERT(newAnchor.avail < desc->maxcount || (oldActive->credit == 0 && oldAnchor.count == 0));

    if(oldActive.credits == 0 && oldAnchor.count > 0)
        UpdateActive(heap, desc, moreCredits);
    mallocBlock->desc = desc;
    return mallocBlock;
}

