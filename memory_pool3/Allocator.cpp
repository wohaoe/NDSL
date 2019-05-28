#include"Heap.h"
#include"Block.h"
#include"SuperBlock.h"
#include"SingleBlock.h"
#include"PrintMessage.h"
#include"Lockable.h"
#include<memory.h>
#include<string.h>
#include<unistd.h>
#include<sys/mman.h>
#include<map>
#include<iostream>
#include <vector>
#include <map>

using namespace std;

void* const MAGIC_BYTES_FOR_SINGLEBLOCK = (void*) 0xBBC;
void* const MAGIC_BYTES_FOR_BLOCK = (void*) 0x239C0DE;
void* const MAGIC_BYTES_FOR_FREE_BLOCK = (void*) 0x2391C0DE;
void* const MAGIC_BYTES_FOR_SUPERBLOCK = (void*) 0xC0DE239;

const int MAX_THREAD_COUNT = 239;
const int K = 1.2;
const double BASE = 8;
const double FRACTION = 1/4;

size_t SIZE_OF_SUPERBLOCK;
int COUNT_OF_CLASSSIZE;
size_t HEAPS_COUNT;

Heap* commonHeap;
Heap* heaps;

const size_t PRECALC_MAX_SIZE = 256;
size_t* roundedUpSizes;
size_t* classesForSizes;

bool initialized;
Lockable initializationLock;

bool abortIfFalse(bool condition, const char* message) 
{
        if (!condition) 
        {
            printMessageln(message);
            std::abort();
        }
        return condition;
}

size_t roundUpSize(size_t size) 
{
    if (initialized && size < PRECALC_MAX_SIZE) 
    {
        return roundedUpSizes[size];    
    }
    double curSize = BASE;
    size_t classSize = 0;
    while (curSize < size) 
    {
        curSize += BASE;
        classSize++;
    }
    return (size_t) curSize;
}

size_t getClassSize(size_t size) 
{
    if (initialized && size < PRECALC_MAX_SIZE) 
    {
        return classesForSizes[size];
    }
    double curSize = BASE;
    size_t classSize = 0;
    while (curSize < size) 
    {
        curSize += BASE;
        classSize++;
    }
    return classSize;
}

void init()
{
    if(!initialized)
    {
        initializationLock.lock();
        if(!initialized)
        {
            SIZE_OF_SUPERBLOCK = 4 * sysconf(_SC_PAGE_SIZE);    //16KB
            COUNT_OF_CLASSSIZE = getClassSize(SIZE_OF_SUPERBLOCK);
            HEAPS_COUNT = 2 * MAX_THREAD_COUNT;

            heaps = (Heap*) mmap(NULL, sizeof(Heap) * (HEAPS_COUNT + 1), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            commonHeap = heaps + HEAPS_COUNT;
            for(Heap* heap = heaps; heap < heaps + HEAPS_COUNT + 1; heap++)
            {
                new(heap) Heap();
            }

            roundedUpSizes = (size_t*) mmap(NULL, sizeof(size_t) * PRECALC_MAX_SIZE * 2, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            classesForSizes = roundedUpSizes + PRECALC_MAX_SIZE; 

            for(size_t size= 0; size < PRECALC_MAX_SIZE; size++)
            {
                roundedUpSizes[size] = roundUpSize(size);
                classesForSizes[size] = getClassSize(size);
            } 
            initialized = true;
        }   
    initializationLock.unlock();
    }
}

size_t getThreadIdHash()
{
    size_t tidHash =  std::hash<std::thread::id>()(std::this_thread::get_id());
    return tidHash;
}

bool isValidAlignment(size_t alignment) 
{
    size_t tmp = alignment;
    while (tmp > sizeof(void*)) 
    {
        if (tmp % 2 != 0) 
        {
            return false;
        }
        tmp /= 2;
    }
    return tmp % sizeof(void*) == 0;
}

void* roundUpToAlignment(void* pointer, size_t alignment) 
{
    size_t mod = (size_t) pointer % alignment;
    if (mod == 0) 
    {
        return pointer;
    } 
    else 
    {
        return (void*) ((char*) pointer + alignment - mod);
    }
}

SingleBlock::SingleBlock(size_t _sizeOfData, size_t _alignedSize)
        : sizeOfData(_sizeOfData),
        alignedSize(_alignedSize),
        magicBytes(MAGIC_BYTES_FOR_SINGLEBLOCK){
}

Block::Block(SuperBlock* _superBlock, size_t _sizeOfData, size_t _alignedSize)
        : superBlock(_superBlock),
        sizeOfData(_sizeOfData),
        alignedSize(_alignedSize),
        magicBytes(MAGIC_BYTES_FOR_BLOCK),
        linkToThis(this){
}

SuperBlock::SuperBlock(size_t _sizePerBlock, size_t _sizeClass) 
        : heap(NULL),
        usedMemory(0),
        previous(NULL),
        next(NULL),
        sizeClass(_sizeClass),
        maxSizeOfDataPerBlock(_sizePerBlock),
        maxCountOfBlocks(calcMaxCountOfBlocks()),
        offsetOfFirstIndex((size_t) (void*) ((char*) this + sizeof(SuperBlock) + sizeof(Block*) * maxCountOfBlocks)),
        nextOffsetIndex(0),
        countOfFreeBlocksInStack(0),
        magicBytes(MAGIC_BYTES_FOR_SUPERBLOCK){
}

Heap::Heap() 
{
    firstSBbySizeclass = (SuperBlock**) mmap(NULL, sizeof(SuperBlock*) * COUNT_OF_CLASSSIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    lastSBbySizeclass = (SuperBlock**) mmap(NULL, sizeof(SuperBlock*) * COUNT_OF_CLASSSIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    usedMemory = 0;
    allocatedMemory = sizeof(Heap) + 2 * sizeof(SuperBlock*) * COUNT_OF_CLASSSIZE;
    for (int i = 0; i < COUNT_OF_CLASSSIZE; i++) 
    {
        firstSBbySizeclass[i] = NULL;
        lastSBbySizeclass[i] = NULL;
    }
}

void Block::freeBlock() 
{
    abortIfFalse(magicBytes == MAGIC_BYTES_FOR_BLOCK,
            "Given pointer does not follow after a correct Block structure!(or block is free)");
    abortIfFalse(superBlock->magicBytes == MAGIC_BYTES_FOR_SUPERBLOCK,
            "Given pointer does not follow correct Superblock structure!");
    magicBytes = MAGIC_BYTES_FOR_FREE_BLOCK;
    superBlock->addFreeBlock(this);
}

void Lockable::lock() 
{
    pthread_mutex_lock(&mutex);
}

void Lockable::unlock() 
{
    pthread_mutex_unlock(&mutex);
}
    
size_t SuperBlock::calcMaxCountOfBlocks() 
{
    return (SIZE_OF_SUPERBLOCK - sizeof(SuperBlock)) /
            (sizeof(void*) + sizeof(Block) + this->maxSizeOfDataPerBlock);
}

Block* SuperBlock::allocateBlock(size_t _size, size_t _sizeOfData)
{
    abortIfFalse(_size <= maxSizeOfDataPerBlock,
            "Size is to big for allocation in this SuperBlock!");

    Block* result = NULL;
    if(nextOffsetIndex != maxCountOfBlocks)
    {
        result = (Block*)((char*) offsetOfFirstIndex + nextOffsetIndex * (sizeof(Block) + maxSizeOfDataPerBlock));
        nextOffsetIndex++;
    }
    else if(countOfFreeBlocksInStack != 0)
    {
        countOfFreeBlocksInStack--;
        result = *((Block**) ((char*)this + sizeof(SuperBlock) + countOfFreeBlocksInStack * sizeof(Block*)));
    }

    if(result != NULL)
    {
        new(result) Block(this, _sizeOfData, _size);
        usedMemory += _sizeOfData;
        heap->usedMemory += _sizeOfData;
    }
    return result;
}

void SuperBlock::attachToHeap(Heap* _heap)
{
    this->heap = _heap;
    heap->usedMemory += usedMemory;
    heap->allocatedMemory += SIZE_OF_SUPERBLOCK;

    SuperBlock* insertAfter = heap->lastSBbySizeclass[sizeClass];
    while(insertAfter != NULL && insertAfter->usedMemory < usedMemory)
    {
        insertAfter = insertAfter->previous;
    }

    if(insertAfter == NULL)
    {
        previous = NULL;
        next = heap->firstSBbySizeclass[sizeClass];
        if(heap->firstSBbySizeclass[sizeClass] != NULL)
        {
            heap->firstSBbySizeclass[sizeClass]->previous = this;
        }

        heap->firstSBbySizeclass[sizeClass] = this;

        if(heap->lastSBbySizeclass[sizeClass] == NULL)
        {
            heap->lastSBbySizeclass[sizeClass] = this;
        }
    }
    else
    {
        next = insertAfter->next;
        previous = insertAfter;
        insertAfter->next = this;
        if (next == NULL) 
        {
            heap->lastSBbySizeclass[sizeClass] = this;
        } else 
        {
            next->previous = this;
        }
    }
}

void SuperBlock::addFreeBlock(Block* _block)
{
    Block** offsetForNewFreeBlock = (Block**) ((char*) this + sizeof(SuperBlock) + countOfFreeBlocksInStack * sizeof(Block*));
    *offsetForNewFreeBlock = _block;
    countOfFreeBlocksInStack++;
    usedMemory -= _block->sizeOfData;
    heap->usedMemory -= _block->sizeOfData;
}

Block* Heap::allocateBlock(size_t _size, size_t _sizeOfData)
{
    size_t classSize = getClassSize(_size);
    SuperBlock* cur = firstSBbySizeclass[classSize];
    while(cur != NULL)
    {
        Block* result = cur->allocateBlock(_size, _sizeOfData);
        if (result != NULL) 
        {
            return result;
        }
        cur = cur->next;
    }
    return NULL;
}

void* poolMalloc(size_t size, size_t alignment)
{
    init();
    void* result;
    if (size == 0) 
    {
        result = NULL;
    } else 
    {
        size_t alignedSize = roundUpSize(size + alignment - 1);
        if (alignedSize > SIZE_OF_SUPERBLOCK / 2) {
            SingleBlock* singleBlock = (SingleBlock*) mmap(NULL, sizeof(SingleBlock) + alignedSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (singleBlock != NULL && singleBlock != MAP_FAILED) 
            {
                new (singleBlock) SingleBlock(size, alignedSize);
                result = roundUpToAlignment((char*) singleBlock + sizeof(SingleBlock), alignment);
            }
        } 
        else 
        {
            size_t heapId = getThreadIdHash() % HEAPS_COUNT;
            Heap* heap = &heaps[heapId];
            heap->lock();
            Block* allocatedBlock = heap->allocateBlock(alignedSize, size);
            heap->unlock();
            if (allocatedBlock == NULL) 
            {
                commonHeap->lock();
                allocatedBlock = commonHeap->allocateBlock(alignedSize, size);
                commonHeap->unlock();
                if (allocatedBlock == NULL) 
                {
                    SuperBlock* superBlock = (SuperBlock*) mmap(NULL, SIZE_OF_SUPERBLOCK, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                    if (superBlock != NULL && superBlock != MAP_FAILED) 
                    {
                        new (superBlock) SuperBlock(roundUpSize(alignedSize), getClassSize(alignedSize));
                        heap->lock();
                        superBlock->attachToHeap(heap);
                        allocatedBlock = superBlock->allocateBlock(alignedSize, size);
                        heap->unlock();
                    }
                } 
                else 
                {
                    SuperBlock* superBlock = allocatedBlock->superBlock;
                    superBlock->lock();
                    Heap* oldHeap = superBlock->heap;
                    if (oldHeap == commonHeap) 
                    {
                        heap->lock();
                        commonHeap->lock();
                        superBlock->deattachFromHeap();
                        superBlock->attachToHeap(heap);
                        commonHeap->unlock();
                        heap->unlock();
                    }
                    superBlock->unlock();
                }
            }
            if (allocatedBlock != NULL) 
            {
                result = roundUpToAlignment((char*) allocatedBlock + sizeof(Block), alignment);
            }
        }
    }
    return result;
}

void poolFree(void* pointer) 
{
    init();
    if (pointer == NULL) 
    {
        return;
    }

    Block* block = (Block*) ((char*) pointer - sizeof(Block));
    block = block->linkToThis;
    if (block->magicBytes == MAGIC_BYTES_FOR_BLOCK) {
        SuperBlock* superBlock = block->superBlock;
        superBlock->lock();

        Heap* heap = block->superBlock->heap;
        heap->lock();
        
        block->freeBlock();
        
        if (heap != commonHeap) {
            if (heap->usedMemory + K * SIZE_OF_SUPERBLOCK < heap->allocatedMemory
                    && heap->usedMemory < (1- FRACTION) * heap->allocatedMemory) 
            {
                commonHeap->lock();
    
                SuperBlock* minUsedBlock = NULL;
                for(int i = 0; i < COUNT_OF_CLASSSIZE; i++) 
                {
                    if (heap->lastSBbySizeclass[i] != NULL) 
                    {
                        if (minUsedBlock == NULL
                                || minUsedBlock->usedMemory > heap->lastSBbySizeclass[i]->usedMemory) 
                        {
                            minUsedBlock = heap->lastSBbySizeclass[i];
                        }
                    }
                }
                minUsedBlock->deattachFromHeap();
                minUsedBlock->attachToHeap(commonHeap);
                commonHeap->unlock();
            }
        }
        heap->unlock();
        superBlock->unlock();
    }
    else 
    {
        SingleBlock* singleBlock = (SingleBlock*) ((char*) pointer - sizeof(SingleBlock));
        singleBlock = singleBlock->linkToThis;
        abortIfFalse(singleBlock->magicBytes == MAGIC_BYTES_FOR_SINGLEBLOCK
                && singleBlock->alignedSize > SIZE_OF_SUPERBLOCK / 2,
                "Given pointer does not point either not to Block or SingleBlock!");
        singleBlock->magicBytes = MAGIC_BYTES_FOR_FREE_BLOCK;
        munmap(singleBlock, sizeof(SingleBlock) + singleBlock->alignedSize);
    }
}


