#ifndef ANCHOR_H_
#define ANCHOR_H_

#include<unistd.h>
#include<cstddef>
#include<cstdlib>

struct Anchor
{
    size_t state : 2;
    size_t avail : 25;
    size_t count : 25;
    size_t tag : 12;
};

#endif
