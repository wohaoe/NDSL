#ifndef CHUNK_H_
#define CHUNK_H_

#include<functional>
#include<cstddef>

namespace chunk
{
    template<typename SizeType = std::size_t>
    class Chunk
    {
    public: 
        typedef SizeType size_type;
    
    private:
        Chunk(const Chunk &);
        void operator = (const Chunk &);
        static void *try_malloc_n(void * & start, size_type n, size_type partition_size);

    protected:
        void *firstChunkOfFreeList;
        
        void *find_prev(void *ptr);
        
        /*一块内存的指针是ptr,这块内存的前几个字节储存了下一块内存的指针地址
            即ptr可以看作是指针的指针
          返回这地址的引用
          */
        static void * & nextPtrOfAddressReference(void * const ptr)
        {
            return *(static_cast<void **>(ptr));
        }

    public:
        //初始化空闲链表被空 
        Chunk() : firstChunkOfFreeList(0)
        {

        }

        static void * divadeBlock(void *block, size_type blockSize, const size_type partitionSize, void *end = 0);

        //添加一个block时，会将这个block分解为chunk,添加到链表头部。
        void add_block(void * const block1, const size_type blockSize1, const size_type partitionSize1)
        {
            firstChunkOfFreeList = devadeBlock(block1, blockSize1, partitionSize1, firstChunkOfFreeList);
        }

        bool isEmpty() const
        {
            return (firstChunkOfFreeList == 0);
        }

        void * mallocChunk()
        {
            void * const ret  = firstChunkOfFreeList;
            firstChunkOfFreeList = nextPtrOfAddressReference(firstChunkOfFreeList);
            return ret;
        }

        //释放单个chunk  
        void * freeChunk(void * const chunk1)
        {
            nextPtrOfAddressReference(chunk1) = firstChunkOfFreeList;
            firstChunkOfFreeList = chunk1;
        }

        //分配n个chunk
        void *malloc_n(size_type n, size_type partition_size);

        //释放n个chunk
        void free_n(void * const chunks, const size_type n, const size_type partition_size)
        {
            if(n != 0)
            {
                add_block(chunks, n * partition_size, partition_size);
            }
        }

    }; 
}

#endif