#include"Chunk.h"

template<typename SizeType>
void *chunk::Chunk<SizeType>::divadeBlock(void *block, size_type blockSize, const size_type partitionSize, void *end)
{
    char *old = static_cast<char *>(block) + ((blockSize - partitionSize) / partitionSize) * partitionSize;
    nextPtrOfAddressReference(old) = end;
    
    if(old == block)
    {
        return block;
    }
    //格式化chunk，使得前4个字节指向下一个chunk
    for(char *iter = old - partitionSize; iter != block; old = iter, iter -= partitionSize)
    {
        nextPtrOfAddressReference(iter) = old;
    }
    nextPtrOfAddressReference(block) = old;
    return block;
}

//比较地址，找到ptr在当前block的前一个位置
template<typename SizeType>
void * chunk::Chunk<SizeType>::find_prev(void * const ptr)
{
    if(isEmpty() || std::greater<void *>()(firstChunkOfFreeList, ptr))
    {
        return 0;
    }

    void * iter = firstChunkOfFreeList;

    while(true)
    {
        if(nextPtrOfAddressReference(iter) == 0 || std::greater<void *>()(nextPtrOfAddressReference(iter), ptr))    //比较地址
        {
            return iter;
        }
        iter = nextPtrOfAddressReference(iter);
    }
}

//try_malloc_n会从start开始(不算start)向后申请n个partition_size大小的chunk
template<typename SizeType>
void * chunk::Chunk<SizeType>::try_malloc_n(void * & start, size_type n, const size_type partition_size)
{
    void *iter = nextPtrOfAddressReference(start);

    //start后面的块是否是连续的n块partition_size大小的内存
    while(--n != 0)
    {
        void *next = nextPtrOfAddressReference(iter);
        
        //说明下一块chunk被占用或是到了大块内存(block)的尾部
        if(next != static_cast<char *>(iter) + partition_size)
        {
            start = iter;
            return 0;
        }
        iter = next;
    }
    return iter;    //返回最后一个指针
}

template<typename SizeType>
void * chunk::Chunk<SizeType>::malloc_n(size_type n, size_type partition_size)
{
    if(n == 0)
    {
        return 0;
    }

    void *start = &firstChunkOfFreeList;
    void *iter;
    do
    {
        if(nextPtrOfAddressReference(start) == 0)
            return 0;
        iter = try_malloc_n(start, n, partition_size);
    }while(iter == 0);

    //返回查找到的内存chunk头
    void *const ret = nextPtrOfAddressReference(start);

    //单向链表移除一段节点，把该内存的前面chunk头指向该内存尾部chunk头指向的内存
    nextPtrOfAddressReference(start) = nextPtrOfAddressReference(iter);
    
    return ret;     //返回分配的第一个chunk
}


