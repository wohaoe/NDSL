#ifndef DESCRIPTOR_H_
#define DESCRIPTOR_H_

#include"Define.h"
#include"PageMap.h"
#include<atomic>
#include<unistd.h>
#include<cstddef>
#include<cstdlib>

struct Anchor;
class SuperBlock;
class Heap;

class Descriptor
{
private:
    std::atomic<Anchor> anchor;
    std::atomic<Descriptor> nextFree;
    std::atomic<Descriptor> nextPartial;

    SuperBlock* superBlock;
    Heap* heap;
    size_t sbSize;
    

public:
    Descriptor(SuperBlock* _superBlock)
    {
        superBlock = _superBlock;
    }

    ~Descriptor()
    {

    }

    void updatePageMap(Heap *_heap, SuperBlock* _superBlock, Descriptor* _desc)
    {
        PageInfo info;
        info.desc = _desc;

        if (!_heap)
        {
            sPageMap.setPageInfo((char*)_superBlock, info);
            return;
        }

        for(int i = 0; i < sbSize; i += PAGE)
        {
            sPageMap.setPageInfo((char*)_superBlock + idx, info);
        }
    }

    void registerDesc()
    {
        updatePageMap(heap, superBlock, this);
    }

    void unRegisterDesc()
    {
        updatePageMap(heap, superBlock, nullptr);
    }

    Descriptor* getDescriptorForPtr(void* ptr)
    {
        PageInfo info = sPageMap.GetPageInfo((char*)ptr);
        return info.desc;
    }
};



#endif