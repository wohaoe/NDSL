#ifndef __DEFINE_H__
#define __DEFINE_H__

#include <cinttypes>

#define LG_CACHELINE    6
#define LG_PAGE         12
#define LG_HUGEPAGE     21

//位掩码
#define CACHELINE   ((size_t)(1U << LG_CACHELINE))  //64B
#define PAGE        ((size_t)(1U << LG_PAGE))   //4KB
#define HUGEPAGE    ((size_t)(1U << LG_HUGEPAGE))   //2MB

#define PAGE_MASK   (PAGE - 1)  

#define MIN_ALIGN sizeof(size_t)

#define ALIGN_ADDR(addr, align) \
    ( __typeof__ (addr))(((size_t)(addr) + (align - 1)) & ((~(align)) + 1)) 


#define PAGE_CEILING(s) \
    (((s) + (PAGE - 1)) & ~(PAGE - 1))                      

#define PM_NLS 12
#define PM_NHS 12
#define PM_SB (64 - PM_NLS - PM_NHS)
#define PM_KEY_SHIFT PM_NLS //除掉低位
#define PM_KEY_MASK ((1ULL << PM_SB) - 1)   //掩码

#define LIKELY(x)       __builtin_expect((x), 1)
#define UNLIKELY(x)     __builtin_expect((x), 0)

void* const MAGIC_BYTES_FOR_SINGLEBLOCK = (void*) 0xBBC;
void* const MAGIC_BYTES_FOR_BLOCK = (void*) 0x239C0DE;
void* const MAGIC_BYTES_FOR_FREE_BLOCK = (void*) 0x2391C0DE;
void* const MAGIC_BYTES_FOR_SUPERBLOCK = (void*) 0xC0DE239;

#define CREDITS_MAX (1ULL << 6) //64B
#define CREDITS_MASK ((1ULL << 6) - 1)  

#define ASSERT(x) do { if (!(x)) abort(); } while (0)
#define STATIC_ASSERT(x, m) static_assert(x, m)

#endif 