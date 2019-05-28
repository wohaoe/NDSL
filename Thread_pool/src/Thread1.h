#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <functional>
#include <assert.h>
#include <exception>
#include "CurrentThread.h"

namespace Reuzel
{
class Thread
{
public:
    typedef std::function<void()> ThreadFunc;

    explicit Thread(const ThreadFunc& func);
    ~Thread();

    Thread(const Thread&) = delete;
    Thread& operaotr=(const Thread&) = delete;

    void start();
    int join();

    bool started() const
    {
        return started_;
    }

    pthead_t pthreadId() const
    {
        return pthreadId_;
    }

    pid_t threadId() const
    {
        return CurrentThread::gettid();
    }

    void runInThread();

private:
    bool started_;
    bool join_;
    pthread_t pthreadId_;
    ThreadFunc func_;
};
}

#endif