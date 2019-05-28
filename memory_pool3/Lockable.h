#ifndef LOCK_H_

#include<mutex>
#include<semaphore.h>

class Lockable
{
private:
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

public:
    void lock();
    void unlock();
};

#endif