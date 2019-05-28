#ifndef SINGLEBLOCK_H_

#include<cstddef>
#include<cstdlib>

struct SingleBlock
{
    size_t sizeOfData;
    size_t alignedSize;

    void* magicBytes;

    SingleBlock* linkToThis;
    SingleBlock(size_t _sizeOfData, size_t _alignedSize);
};

#endif