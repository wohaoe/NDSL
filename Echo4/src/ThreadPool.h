//
// ThreadPool.h
//
// Copyright (c) 2017 Jiawei Feng
//
/*接口类*/
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

namespace Reuzel {
    using std::string;

    class ThreadPool {
    public:
        typedef std::function<void ()> Task;    //函数指针，返回值为void，参数为空的函数原型

        explicit ThreadPool(const string &nameArg =
                            "Reuzel ThreadPool");
        ~ThreadPool();

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool &operator=(const ThreadPool&) = delete;

        void setMaxQueueSize(int maxSize)
        {
            maxQueueSize_ = maxSize;
        }
        /*
        void setThreadInitCallback(const Task &cb)
        {
            threadInitCallback_ = cb;
        }
        */

        void start(int numThreads);
        void stop();

        const string &name() const
        {
            return name_;
        }

        size_t queueSize() const;

        void addTask(const Task &task);

    private:
        bool isFull() const;
        void runInThread();
        Task takeTask();

        mutable MutexLock mutex_;
        Condition notEmpty_;
        Condition notFull_;
        string name_;
        //Task threadInitCallback_;
        std::vector<std::unique_ptr<Thread>> threads_;  //unique_ptr为智能指针，能够自动调用delete，存放着已经分配了的线程
        std::deque<Task> taskQueue_;    //双向队列，存放着线程执行函数
        size_t maxQueueSize_;
        bool running_;
    };
}

#endif
