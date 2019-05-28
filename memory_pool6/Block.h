#ifndef BLOCK_H_

#include"Define.h"
#include<unistd.h>
#include<cstddef>
#include<cstdlib>

class Descriptor;
class SuperBlock;

class Block
{
private:
    size_t sizeOfData;
    size_t alignedSize;

    void* MagicBytes;
    Block* linkToThis;

    Descriptor* desc;
    SuperBlock* superBlock;

public:
    Block(Descriptor* _desc, SuperBlock* _superBlock, size_t _sizeOfData, size_t _alignedSize)
    {
        desc = _desc;
        superBlock = _superBlock;
        sizeOfData = _sizeOfData;
        alignedSize = _alignedSize;
        MagicBytes = MAGIC_BYTES_FOR_BLOCK;
        linkToThis = this;
    }

    ~Block()
    {
        
    }

};


#endif