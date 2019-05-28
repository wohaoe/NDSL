#ifndef RAWMEMORYBLOCK_H_
#define RAWMEMORYBLOCK_H_

#include"Math1.h"

namespace raw
{
    template<typename SizeType>
    class RawMemoryBlock
    {
    public:
        typedef SizeType size_type; 
    private:
        char *ptr;  //block起始指针
        size_type blockSize;
        
        //获取下一个block的size
        char *ptr_nextblock_size()
        {
            return (ptr + blockSize - sizeof(size_type));
        }

        //获得下一个block的point
        char *ptr_nextblock_point()
        {
            return (ptr_nextblock_size() - math1::getLcm(sizeof(size_type), sizeof(void *)));
        }

    public:
        RawMemoryBlock(char *const nptr, const size_type nsize) : ptr(nptr), blockSize(nsize)
        {

        }
        RawMemoryBlock() : ptr(0), blockSize(0)
        {

        }

        bool vaild() const
        {
            return (beginPtr() != 0);
        }

        //使block无效
        void invaildate()
        {
            beginPtr() = 0;
        }

        char * beginPtr() const
        {
            return ptr;
        }

        char * &beginPtr()  //为了不创建副本
        {
            return ptr;
        }

        char *endPtr() const
        {
            return ptr_nextblock_point();
        }

        size_type total_size() const
        {
            return blockSize;
        }
        
        //返回本block中的数据区大小
        size_type block_data_size() const
        {
            return (static_cast<size_type>(blockSize - sizeof(size_type) - math1
            ::getLcm(sizeof(size_type), sizeof(void *))));
        }

        size_type &nextblock_size() const
        {
            return *(static_cast<size_type *>(static_cast<void *>(ptr_nextblock_size())));
        }

        char *&nextblock_point() const
        {
            return *(static_cast<size_type *>(static_cast<void *>(ptr_nextblock_point())));
        }

        RawMemoryBlock next() const
        {
            return RawMemoryBlock<size_type>(nextblock_point(), nextblock_size());
        }

        //设置下一块
        void setNextBlock(const RawMemoryBlock &arg) const
        {
            nextblock_point() = arg.beginPtr();
            nextblock_size() = arg.total_size();
        }
    };
}

#endif