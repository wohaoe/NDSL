#include <cerrno>

#include "Allocator.h"

using namespace dushuang;

namespace
{
    __thread bool inside_malloc = false;

    struct recuirsion_guard
    {
        recuirsion_guard()
        {
            if (inside_malloc)
            {
                printMessage("recursive call\n");
                std::abort();
            }
            
            inside_malloc = true;
        }

        ~recuirsion_guard()
        {
            inside_malloc = false;
        }

    private:
        recuirsion_guard(recuirsion_guard const&);
        recuirsion_guard& operator=(recuirsion_guard const&);
    };
}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;//delete

extern "C"
void* malloc(size_t size)
{
    recuirsion_guard rg;
    
    void *p = poolMalloc(size);
    
    if (isTraceEnabled()) {
        printMessageln("malloc", size, p);
    }

    return p;
}


extern "C"
void free(void *ptr)
{
    recuirsion_guard rg;

    poolFree(ptr);
    
    if (isTraceEnabled()) {
        printMessageln("free", ptr);
    }
}

/*
extern "C"
int posix_memalign(void** memptr, size_t alignment, size_t size)
{
    recuirsion_guard rg;

    int result = hoardPosixMemalign(memptr, alignment, size);
    
    if (isTraceEnabled()) {
        printMessageln("posix_memalign", alignment, size, *memptr);
    }
    return result;
}


extern "C"
void *memalign(size_t boundary, size_t size)
{
    recuirsion_guard rg;

    printMessage("deprecated function memalign is not supported\n");
    std::abort();
}*/
