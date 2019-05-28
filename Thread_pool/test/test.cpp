//
// test.cpp
//
// Copyright (c) 2017 Jiawei Feng
//

#include "../src/ThreadPool.h"
#include "../src/CountDownLatch.h"
#include <stdio.h>
#include <pthread.h>
#include <string>
#include <unistd.h>

/*运行结果中，顺序不是确定的，for与for之间的执行顺序不确定，同一个for的执行顺序也不确定*/
void print()
{
    printf("current pthread = %d\n", Reuzel::CurrentThread::gettid());
}

void printString(const std::string &taskId)
{
    printf("current pthread = %d %s\n",
            Reuzel::CurrentThread::gettid(), taskId.c_str());
    usleep(50 * 1000);
}

void test(int maxSize)
{
    printf("Test ThreadPool with max queue size = %d\n", maxSize);
    Reuzel::ThreadPool pool("MainThreadPool");
    pool.setMaxQueueSize(maxSize);
    pool.start(5);  //5个线程

    printf("Adding print task\n");
    pool.addTask(print);
    pool.addTask(print);

    printf("Adding printString tast\n");
    for (int i = 0; i < 40; ++i) {
        std::string taskId = "task - ";  //字符串taskId初始化
        taskId += std::to_string(i);
        pool.addTask(std::bind(printString, taskId));//bind(函数指针， 参数值)  //原本需要无参数的函数
    }

    printf("Adding printString tast2\n");   
    for(int j = 0; j < 20; ++j)
    {
        std::string taskId2 = "task2 - ";
        taskId2 += std::to_string(j);
        pool.addTask(std::bind(printString, taskId2));
    }

    Reuzel::CountDownLatch latch(1);
    pool.addTask([&](){ latch.countDown(); });
    latch.wait();
    pool.stop();
}

int main()
{
    test(10);
}
