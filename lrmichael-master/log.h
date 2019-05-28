
#ifndef __LOG_H
#define __LOG_H

#include <cstdio>
#include <cstdlib>

// if 1, enables assertions and other sanity checks
#define LFMALLOC_SANITY 0
// if 1, enables debug output
#define LFMALLOC_DEBUG 0

#if LFMALLOC_DEBUG
#define LOG_DEBUG(STR, ...) \
    fprintf(stdout, "%s:%d %s " STR "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__);

#else
#define LOG_DEBUG(str, ...)

#endif

#define LOG_ERR(STR, ...) \
    fprintf(stderr, "%s:%d %s " STR "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)  //##__VA_ARGS__ 宏前面加上##的作用在于，当可变参数的个数为0时，这里的##起到把前面多余的","去掉的作用,否则会编译出错

#if LFMALLOC_SANITY
#define ASSERT(x) do { if (!(x)) abort(); } while (0)
#else
#define ASSERT(x)
#endif

#endif // _LOG_H

