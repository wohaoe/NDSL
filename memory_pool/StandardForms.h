#ifndef STANDARDFORMS_H_
#define STANDARDFORMS_H_

#include<new>
#include<exception>
#include<cstddef>
#include<cstdlib>

namespace standard
{
    struct default_user_allocator_new_delete
    {
        typedef std::size_t size_type;
        static void *operator new(std::size_t size) throw(std::bad_alloc)
        {
            return ::operator new(size);
        }

        static void operator delete(void *pMemory) throw() 
        {
            ::operator delete(pMemory);
        }

        static void *operator new(std::size_t size, void *ptr) throw()
        {
            return ::operator new(size, ptr);
        }

        static void operator delete(void *pMemory, void *ptr) throw()
        {
            return ::operator delete(pMemory, ptr);
        }

        static void *operator new(std::size_t size, const std::nothrow_t &nt) throw()
        {
            return ::operator new(size, nt);
        }

        static void operator delete(void *pMemory, const std::nothrow_t &) throw()
        {
            ::operator delete(pMemory);
        }
    };

    struct default_user_allocator_malloc_free
    {
        typedef std::size_t size_type;
        
        static char * malloc(const size_type bytes)
        {
            return static_cast<char *>((std::malloc)(bytes));
        }

        static void free(char *const block)
        {
            (std::free)(block);
        }
    };
}

#endif