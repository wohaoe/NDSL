#include "condition.h"

typedef struct Task{
  struct Task *next;
  void * (*run)(void *args);    //回调函数
  void *args;   //回调参数
}task;

typedef struct Threadpool{
  Condition ready;  //各种通知
  task *first;      //任务队列头
  task *tail;

  int max_thread; //最大进程数
  int counter;    //当前进程数
  int idle;       //空闲进程数

  bool quit;      //进程池是否退出信号

}threadpool;

int threadpool_init(threadpool *, int);
void *thread_routine(void *);
void threadpool_add_task(threadpool *pool, void *(*run)(void*), void *args);
void thread_pool_destory(threadpool *);
