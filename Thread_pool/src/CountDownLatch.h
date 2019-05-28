//
// CountDownLatch.h
//
// Copyright (c) 2017 Jiawei Feng
//
/*接口类*/

#ifndef COUNTDOWNLATCH_H
#define COUNTDOWNLATCH_H

#include "Mutex.h"
#include "Condition.h"

namespace Reuzel {
    class CountDownLatch {
    public:
        explicit CountDownLatch(int count);

        CountDownLatch(const CountDownLatch&) = delete;
        CountDownLatch &operator=(const CountDownLatch&) = delete;

        void wait();

        void countDown();

        int getCount() const;
    private:
        mutable MutexLock mutex_;
        Condition cond_;
        int count_;
    };
}

#endif
