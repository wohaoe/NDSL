#include"Pool.h"

//找到chunk的block号，返回该block，遍历list
template<typename UserAllocator>
raw::RawMemoryBlock<typename dushuang::Pool<UserAllocator>::size_type>
dushuang::Pool<UserAllocator>::find_block(void * const chunkPS) const
{
    raw::RawMemoryBlock<size_type> iter = list;
    while(iter.valid())
    {
        if(is_from(chunkPS, iter.beginPtr(), iter.element_size()))
            return iter;
        
        iter = iter.next();
    }

    return iter;
}

//释放一点也未被占用的block
template <typename UserAllocator>
bool dushuang::Pool<UserAllocator>::release_memory()
{
    bool ret = false;
    raw::RawMemoryBlock<size_type> validMemoryPtr = list;
    raw::RawMemoryBlock<size_type> previousBlockOfvalidMemoryPtr;

    void * firstFreeChunk = this->firstChunkOfFreeList;
    void * theLastFreeChunkOfPreviousMemoryBlock = 0;

    const size_type partition_size = alloc_size();

    //查找所有已分配的内存块
    while(validMemoryPtr.valid())
    {
        if(firstFreeChunk == 0)
            break;

        bool all_chunks_free = true;

        void * saved_free = firstFreeChunk;
        for(char * i = validMemoryPtr.beginPtr(); i != validMemoryPtr.end(); i += partition_size)
        {
            if(i != firstFreeChunk)
            {
                all_chunks_free = false;
                firstFreeChunk = saved_free;
                break;
            }
            firstFreeChunk = nextPtrOfAddressReference(firstFreeChunk);
        }

        const raw::RawMemoryBlock<size_type> nextVaildBlock = validMemoryPtr.next();

        if(!all_chunks_free)
        {
            if(is_from(firstFreeChunk, validMemoryPtr.beginPtr(), validMemoryPtr.element_size()))
            {
                std::less<void *> lt;
                void * const end = validMemoryPtr.end();
                do
                {
                    theLastFreeChunkOfPreviousMemoryBlock = firstFreeChunk;
                    firstFreeChunk = nextPtrOfAddressReference(firstFreeChunk);
                }while(firstFreeChunk && lt(firstFreeChunk, end));
            }
            previousBlockOfvalidMemoryPtr = validMemoryPtr;
        }
        else    //这个block的所有chunks都是free
        {
            if(previousBlockOfvalidMemoryPtr.vaild())
                previousBlockOfvalidMemoryPtr.next(nextVaildBlock);
            else
                list = nextVaildBlock;
            
            if(theLastFreeChunkOfPreviousMemoryBlock != 0)
                nextPtrOfAddressReference(theLastFreeChunkOfPreviousMemoryBlock) = firstFreeChunk;
            else
                this->firstChunkOfFreeList = firstFreeChunk;
            
            (UserAllocator::free)(validMemoryPtr.beginPtr());
            ret = true;
        }

        //增加validMemoryPtr
        validMemoryPtr = nextVaildBlock;
    }

    next_size = start_size;
    return ret;
}

template <typename UserAllocator>
bool dushuang::Pool<UserAllocator>::purge_memory()
{
    raw::RawMemoryBlock<size_type> iter = list;

    if(!iter.vaild())
        return false;

    do
    {
        const raw::RawMemoryBlock<size_type> nextBlock = iter.next();
        
        (UserAllocator::free)(iter.beginPtr());
        iter = nextBlock;
    }while(iter.vaild());

    list.invalidate();
    this->firstChunkOfFreeList = 0;
    next_size = start_size;

    return true; 
}

template<typename UserAllocator>
void * dushuang::Pool<UserAllocator>::malloc_need_resize()
{
    size_type partition_size = alloc_size();

    //需要新添加的block的size
    size_type block_size = static_cast<size_type>(next_size * partition_size + math1::getLcm(sizeof(void *), sizeof(size_type)) + sizeof(size_type));

    //用default的new分配block
    char *ptr =(UserAllocator::malloc)(block_size);
    if(ptr == 0)
    {
        if(next_size > 4)
        {
            next_size >>= 1;
            partition_size = alloc_size();
            block_size = static_cast<size_type>(next_size * partition_size + math1::getLcm(sizeof(size_type), sizeof(void *)) + sizeof(size_type));
            ptr = (UserAllocator::malloc)(block_size);
        }
        if(ptr == 0)
            return 0;
    }

    const raw::RawMemoryBlock<size_type> node(ptr, block_size);

    //下一次分配的chunk个数是上一次的两倍
    if(!max_size)
        next_size <<= 1;
    else if(next_size * partition_size / requestd_size < max_size)
        next_size = std::min(next_size << 1, max_size * requestd_size / partition_size);
    
    //初始化
    store().add_block(node.beginPtr(), node.element_size(), partition_size);

    //添加到list
    node.next(list);
    list = node;
    
    //返回一个指向新chunk的指针
    return (store().malloc)();
}

template<typename UserAllocator>
dushuang::Pool<UserAllocator>::~Pool()
{
    try
    {
        dushuang::Pool<UserAllocator>::purge_memory();
    }
    catch(std::exception e)
    {
        std::cout<<e.what()<<std::endl;
    }
}
