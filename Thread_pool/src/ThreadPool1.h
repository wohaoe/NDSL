#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <vector>
#include <deque>
#include <functional>
#include <memory>
#include <string>
#include "Thread.h"
#include "Condition.h"
#include "Mutex.h"

namespace Reuzel
{
using std::string;
class ThreadPool
{
public:
    typedef std::function<void()> Task;

    explicit ThreadPool(const string &nameArg = 
                            "Reuzel Threadpool");
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    void setMaxQueueSize(int maxSize)
    {
        MaxQueueSize_ = maxSize;
    }

    void start(int numThreads);
    void stop();
    const string &name() const
    {
        return name_;
    }

    size_t queuesize() const;
    
    void addTask(const Task &task);

private:
    bool isFull() const;
    void runInThread();
    Task takeTask();

    mutable MutexLock mutex_;
    Condition notEmpty_;
    Condition notFull_;
    string name_;

    std::vector<std::unique_ptr<Thread>> threads_;
    std::deque<Task> taskQueue_;
    size_t MaxQueueSize_;
    bool running_;
};
}