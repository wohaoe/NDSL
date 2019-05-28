#ifndef SUPERBLOCK_H_

//#include"Lockable.h"
#include<cstddef>
#include<cstdlib>

class Lockable;
struct Heap;
struct Block;

struct SuperBlock : Lockable
{
private:
    const size_t sizeClass;
    const size_t maxSizeOfDataPerBlock;
    const size_t maxCountOfBlocks;

    const size_t offsetOfFirstIndex;
    size_t nextOffsetIndex;
    
    size_t countOfFreeBlocksInStack;

    size_t calcMaxCountOfBlocks();

public:
    Heap* heap;
    size_t usedMemory;

    SuperBlock* previous;
    SuperBlock* next;

    void* magicBytes;
    
    SuperBlock(size_t _sizePerBlock, size_t _sizeClass);
    Block* allocateBlock(size_t _size, size_t _sizeOfData);
    void addFreeBlock(Block* _block);
    void deattachFromHeap();
    void attachToHeap(Heap* _heap);    
};

#endif