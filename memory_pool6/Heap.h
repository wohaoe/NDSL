#ifndef HEAP_H_
#define HEAP_H_

#include<cstddef>
#include<cstdlib>
#include<unistd.h>
#include<atomic>

class Descriptor;
class Active;
class Block;

class Heap
{
private:
    std::atomic<Descriptor> partialList;
    std::atomic<Active*> active;

    const size_t sizeClass;
    const size_t usedMemory;
    const size_t allocatedMemory;
    const size_t sbSize;

public:
    Heap();
    ~Heap();
};

Block* mallocFromActive(Heap* heap);
Block* mallocFromPartial(Heap* heap);
Block* mallocForNewSB(Heap* heap);


#endif