//
// Mutex.h
//
// Copyright (c) 2017 Jiawei Feng
//

#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>
#include <assert.h>
#include "CurrentThread.h"

#define MCHECK(ret) ({ __typeof(ret) errnum = ret;    \
                       assert(errnum == 0); (void)errnum; })

namespace Reuzel {
    class MutexLock {
    public:
        MutexLock()
          : holder_(0)
        {
            MCHECK(pthread_mutex_init(&mutex_, NULL));
        }

        ~MutexLock()
        {
            assert(holder_ == 0);
            MCHECK(pthread_mutex_destroy(&mutex_));
        }

        MutexLock(const MutexLock&) = delete;
        MutexLock &operator=(const MutexLock&) = delete;

        bool isLockedByThisThread() const
        {
            return holder_ == CurrentThread::gettid();
        }

        void assertLocked() const
        {
            assert(isLockedByThisThread());
        }

        void lock() //锁住的同时，使当前线程id赋值给holder_
        {
            MCHECK(pthread_mutex_lock(&mutex_));
            assignHolder();
        }

        void unlock()   //解锁的同时，使holder_赋值为0
        {
            unassignHolder();
            MCHECK(pthread_mutex_unlock(&mutex_));
        }

        pthread_mutex_t *getPthreadMutex()
        {
            return &mutex_;
        }

    private:
        friend class Condition;

/*
MutexLock::lock、MutexLock::unlock等方法的内部代码会小心维护MutexLock::holder_与pthread_mutex_t实例内部状态的一致性。
但当你使用pthread_cond_wait的时候，你必须把MutexLock内部的pthread_mutex_t实例暴露出来。
显然pthread_cond_wait内部会修改pthread_mutex_t实例的状态，
那么此时MutexLock::holder_与pthread_mutex_t实例内部状态的一致性就被破坏了，
所以需要在调用pthread_cond_wait的前后添加一些代码去相应的修改MutexLock::holder_，
也就是分别调用MutexLock::unassignHolder和MutexLock::assignHolder。*/

        class UnassignGuard {
        public:
            UnassignGuard(MutexLock &owner)
              : owner_(owner)
            {
                owner_.unassignHolder();
            }

            ~UnassignGuard()
            {
                owner_.assignHolder();
            }

            UnassignGuard(const UnassignGuard&) = delete;
            UnassignGuard &operator=(const UnassignGuard &) = delete;
        private:
            MutexLock &owner_;
        };

        void assignHolder()
        {
            holder_ = CurrentThread::gettid();
        }

        void unassignHolder()
        {
            holder_ = 0;
        }

        pthread_mutex_t mutex_;
        pid_t holder_;
    };

    class MutexLockGuard {
    public:
        explicit MutexLockGuard(MutexLock &mutex)
          : mutex_(mutex)
        {
            mutex_.lock();
        }

        ~MutexLockGuard()
        {
            mutex_.unlock();
        }
    private:
        MutexLock &mutex_;
    };
}

#endif
