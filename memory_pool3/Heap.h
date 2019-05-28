#ifndef HEAP_H_

//#include"Lockable.h"
#include<thread>
#include<functional>
#include<map>
#include<iostream>
#include<algorithm>
#include<new>
#include<pthread.h>
#include<stack>

class Lockable;
struct Block;
struct SuperBlock;

struct Heap : Lockable
{
    SuperBlock** firstSBbySizeclass;
    SuperBlock** lastSBbySizeclass;
    
    size_t usedMemory;
    size_t allocatedMemory;

    Heap();
    Block* allocateBlock(size_t _size, size_t _sizeOfData);
};

void* poolMalloc(size_t size, size_t alignment);
void poolFree(void* pointer); 

#endif