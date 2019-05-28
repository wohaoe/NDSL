#ifdef COUNTDOWNLATCH_H
#define COUNTDOWNLATCH_H

#include "mutex.h"
#include "Condition.h"

namespace Reuzel
{
class CountDownLatch
{
public:
    explicit CountDownLatch(int count);
    CountDownLatch(const CountDownLatch&) = delete;
    CountDownLatch &operator=(const CountDownLatch&) = delete;

    void wait();
    void countDown();
    int getCount() const;
private:
    int count_;
    Condition cond_;
    mutable MutexLock mutex_;
};
}

#endif