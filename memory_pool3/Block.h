#ifndef BLOCK_H_

#include<unistd.h>
#include<cstddef>
#include<cstdlib>

struct SuperBlock;

struct Block
{
    SuperBlock* superBlock;

    size_t sizeOfData;
    size_t alignedSize;

    void* magicBytes;

    Block* linkToThis;

    Block(SuperBlock* _superBlock, size_t _sizeOfData, size_t _alignedSize);
    void freeBlock();
};

#endif