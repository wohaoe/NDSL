//
// ThreadPool.cpp
//
// Copyright (c) 2017 Jiawei Feng
//

#include "ThreadPool.h"
#include <assert.h>
#include <functional>
#include <algorithm>
#include <exception>
#include <stdio.h>

using namespace Reuzel;

ThreadPool::ThreadPool(const string &nameArg)
  : mutex_(),
    notEmpty_(mutex_),
    notFull_(mutex_),
    name_(nameArg),
    maxQueueSize_(0),
    running_(false)
{
}

ThreadPool::~ThreadPool()
{
    if (running_) {
        stop();
    }
}

void ThreadPool::start(int numThreads)
{
    assert(threads_.empty());
    running_ = true;
    threads_.reserve(numThreads);   //reserver函数用来给vector预分配存储区大小，但是没有对这段内存进行初始化

/*Lambda函数可以引用在它之外声明的变量. 这些变量的集合叫做一个闭包. 闭包被定义在Lambda表达式声明中的方括号[]内. 这个机制允许这些变量被按值或按引用捕获.*/
  /* [&]       //用到的任何外部变量都隐式按引用捕获*/
    for (int i = 0; i < numThreads; ++i) {
        threads_.push_back(std::unique_ptr<Thread>(
            new Thread([&](){ this->runInThread(); } )));   //        explicit Thread(const ThreadFunc& func);
        threads_[i]->start();   //调用thread::start()
    }
    /*
    if (numThreads == 0 && threadInitCallback_) {
        threadInitCallback_();
    }
    */
}

void ThreadPool::stop()
{
    {
        MutexLockGuard lock(mutex_);    //上锁
        running_ = false;
        notEmpty_.notifyAll();  //要结束了，所以通知所有等待线程
    }
    std::for_each(threads_.begin(), threads_.end(),
        [](std::unique_ptr<Thread> &thread) { thread->join(); });   //全部join()
}

size_t ThreadPool::queueSize() const
{
    MutexLockGuard lock(mutex_);
    size_t size = taskQueue_.size();
    return size;
}

void ThreadPool::addTask(const Task &task)
{
    if (threads_.empty()) {
        task();
    }
    else {
        MutexLockGuard lock(mutex_);
        while (isFull()) {
            notFull_.wait();
        }
        assert(!isFull());  //expression为假，则调用abort()

        taskQueue_.push_back(task); //在双向队列尾部插入新的函数
        notEmpty_.notify();
    }
}

ThreadPool::Task ThreadPool::takeTask() 
{
    MutexLockGuard lock(mutex_);
    while (taskQueue_.empty() && running_) {
        notEmpty_.wait();
    }

    Task task;
    if (!taskQueue_.empty()) {
        task = taskQueue_.front();
        taskQueue_.pop_front(); //取出双向队列头部函数

        if (maxQueueSize_ > 0) {
            notFull_.notify();
        }
    }

    return task;
}

bool ThreadPool::isFull() const
{
    mutex_.assertLocked();
    return maxQueueSize_ > 0 && taskQueue_.size() >= maxQueueSize_;
}

void ThreadPool::runInThread()
{
    try {
        /*
        if (threadInitCallback_) {
            threadInitCallback_();
        }
        */
        while (running_) {
            Task task(takeTask());  //task的初始化,相当于构造函数，因为task是一个函数对象
            if (task) {
            }
                task();
        }
    }
    catch (const std::exception &e) {
        printf("exception caught in ThreadPool %s\n", name_.c_str());
        abort();
    }
}
