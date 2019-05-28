#ifndef ACTIVE_H_
#define ACTIVE_H_

#include<atomic>
#include<unistd.h>
#include<cstddef>
#include<cstdlib>

class Descriptor;

class Active
{
private:
    Descriptor* desc;
    size_t credit : 6;

public:
    Active(Descriptor* _desc, size_t _credit);
    ~Active();
};

#endif