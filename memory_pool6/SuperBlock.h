#ifndef SUPERBLOCK_H_
#define SUPERBLOCK_H_

#include<cstddef>
#include<cstdlib>
#include<unistd.h>

class Heap;

class SuperBlock
{
private:
    const size_t sizeClass;
    const size_t maxSizeOfDataPerBlock();
    const size_t maxCountOfBlocks;

    size_t calcMaxCountOfBlocks();

    void* magicBytes;
    size_t usedMemory;

public:
    SuperBlock(size_t _sizePerBlock, size_t _sizeClass);
    ~SuperBlock();

    Heap* heap;
};

#endif