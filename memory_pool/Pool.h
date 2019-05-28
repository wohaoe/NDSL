#ifndef POOL_H_
#define POOL_H_

#include"StandardForms.h"
#include"RawMemoryBlock.h"
#include"Chunk.h"
//#include"PoolPra.h"
#include<iostream>
#include<set>
#include<algorithm>
#include<iomanip>
#include<assert.h>
#include<exception>
#include<cstddef>
#include<cstdlib>
#include<climits>

namespace dushuang
{
    template<typename UserAllocator = standard::default_user_allocator_malloc_free>
    class Pool : public chunk::Chunk<>
    {
    public:
        typedef UserAllocator user_alloctor;
        typedef typename user_alloctor::size_type size_type;
    
    private:
        void * malloc_need_resize();
        //void * ordered_malloc_need_resize();
        //void * size();
        const size_t min_alloc_size = (size_t)math1::getLcm(sizeof(void *), sizeof(size_type));
        //const size_t min_align = (size_t)math1::getLcm(::boost::alignment_of<void *>::value, ::boost::alignment_of<size_type>::value)
        const size_t min_align = (size_t)(alignof(size_type));
    protected:
        raw::RawMemoryBlock<size_type> list;
        
        chunk::Chunk<size_type> & store()
        {
            return *this;
        }

        const size_type requestd_size;
        size_type max_size;
        size_type start_size;
        size_type next_size;

        raw::RawMemoryBlock<size_type> find_block(void * const chunkSP) const;

        static bool is_from(void *const chunkPS, char *const i, const size_type sizeof_i)
        {
            std::less_equal<void *> lt_eq;
            std::less<void *> lt;
            return (lt_eq(i, chunkPS) && lt(chunkPS, i + sizeof_i));
        }

        size_type alloc_size() const
        {
            size_type s = (std::max)(requestd_size, min_alloc_size);
            size_type rem = s % min_align;
            if(rem)
                s += min_align - rem;
            assert(s >= min_alloc_size);
            assert(s % min_align == 0);
            return s;
        }

    public:
    //第一个参数是申请chunk的大小，第二个参数首次申请chunk的个数，下次申请数量翻倍，第三个参数一个block中最多的chunk数目，第二个参数不能大于它，如果为0，就没有限制了。
        explicit Pool(const size_type nrequestd_size, const size_type nnext_size = 32, const size_type nmax_size = 0)
                : list(0, 0), requestd_size(nrequestd_size), next_size(nnext_size), start_size(nnext_size), max_size(nmax_size)
        {

        }

        bool release_memory();
        bool purge_memory();

        ~Pool();

        size_type get_next_size() const
        {
            return next_size;
        }
        void set_next_size(size_type nnext_size)
        {
            start_size = next_size = nnext_size;
        }
        
        size_type get_max_size() const
        { 
            return max_size;
        }
        void set_max_size(const size_type nmax_size)
        { 
            max_size = nmax_size;
        }
        
        size_type get_requested_size() const
        {
            return requestd_size;
        }
        
        void set_start_size(const size_type nstart_size)
        {
            start_size = nstart_size;
        }
        size_type get_start_size() const
        {
            return start_size;
        }
        
        void *malloc()
        {
            // 如果已经分配了，直接在分配的里面找，找不到，就需要调用malloc_need_resize了
            if(!store().isEmpty())
                return (store().mallocChunk)();
            return malloc_need_resize();
        }

        void free(void * const chunk1)
        {
            (store().freeChunk)(chunk1);
        }    

        void free(void * const chunks, const size_type n)
        {
            const size_type partition_size = alloc_size();
            const size_type total_req_size = n * requestd_size;
            const size_type num_chunks = total_req_size / partition_size +
                    ((total_req_size % partition_size) ? true : false);

            store().free_n(chunks, num_chunks, partition_size);
        }

        bool is_from(void * const chunk) const
        {
            return (find_block(chunk).vaild());
        }
    };
}

#endif
