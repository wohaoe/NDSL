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
    threads_.reserve(numThreads);

    for(int i = 0; i < numThreads; i++)
    {
        threads_.push_back(std::unique_ptr<Thread>(new Thead([&](){this->runInThread()})));
        threads_[i]->start();
    }
}
