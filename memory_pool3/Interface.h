#include"Heap.h"
#include"PrintMessage.h"
#include"cstddef"
#include"cstdlib"

extern "C"
void* malloc(size_t size)
{
    void *p = poolMalloc(size, 8);
    
    if (isTraceEnabled()) 
    {
        printMessageln("malloc", size, p);
    }

    return p;
}

extern "C"
void free(void *ptr)
{
    poolFree(ptr);
    
    if (isTraceEnabled()) 
    {
        printMessageln("free", ptr);
    }
}
