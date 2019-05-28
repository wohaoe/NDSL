#include "threadpool.h"

int threadpool_init(threadpool *pool, int num){
  pool->max_thread = num;
  pool->idle = 0;     //因为这里还没创建进程
  pool->counter = 0;
  pool->quit = false;
  pool->first = pool->tail = NULL;
}

void *thread_routine(void *args){
  printf("thread 0x%lx is starting...\n", pthread_self());

  threadpool *pool = (threadpool*)args;

  //等待任务到来
  while(true){
    bool timeout = false;
    pool->ready.lock();

    //进程处于等待状态中的时候说明空闲进程多了一个
    pool->idle++;

    while(pool->first == NULL && pool->quit == false){
      printf("thread 0x%lx is waiting...\n", pthread_self());

      if(0 != pool->ready.timedwait(2)){
        printf("thread 0x%lx is timeout\n", pthread_self());
        timeout = true;
        break;
      }
    }

    //无论是线程销毁还是任务到来，空闲线程数都会--
    pool->idle--;

    if(pool->first != NULL){
      task *t = pool->first;
      pool->first = pool->first->next;

      //此线程执行任务的时候放开对线程池的锁
      //因为可能要执行一段时间
      pool->ready.unlock();

      t->run(t->args);
      free(t);

      //执行完毕后要对线程池操作则需要再加上锁
      pool->ready.lock();
    }

    //收到线程销毁的消息
    if(pool->quit == true && pool->first == NULL){
      pool->counter--;

      //告诉线程池 线程已经销毁完毕
      //这时候只有线程池的线程还存活 直接调用signal就可以
      if(pool->counter == 0)
        pool->ready.signal();

      pool->ready.unlock();
      break;
    }

    //超时并且没有其他任务的时候销毁线程
    if(timeout == true && pool->first == NULL){
      pool->counter--;
      pool->ready.unlock();
      break;
    }

    pool->ready.unlock();
  }

  printf("thread 0x%lx is exiting...\n", pthread_self());
  pthread_exit(NULL);

}

void threadpool_add_task(threadpool *pool, void *(*run)(void*), void *args){
  // printf("this is threadpool_add_task\n");

  //新建一个任务
  task *new_task = (task*)malloc(sizeof(task));
  new_task->run = run;

  // printf("args = %d\n", *((int*)args));

  new_task->args = args;
  //这个free查了半天，因为传的是地址，所以不能free掉
  // free(args);

  // printf("new_task->args = %d\n", *((int *)new_task->args));

  new_task->next = NULL;

  pool->ready.lock();
  //加到缓冲池中
  if(pool->first == NULL)
    pool->first = new_task;
  else
    pool->tail->next = new_task;
  pool->tail = new_task;

  //从缓冲池中分配线程处理任务
  //有空闲进程则进行唤醒操作
  if(pool->idle > 0)
    pool->ready.signal();
  else if(pool->counter < pool->max_thread){
    //无空闲进程则创建新进程
    pthread_t pid;
    pthread_create(&pid, NULL, thread_routine, pool);
    pool->counter++;
  }
  pool->ready.unlock();
}

void thread_pool_destory(threadpool *pool){
  pool->ready.lock();
  pool->quit = true;
  if(pool->counter > 0){

    //如果还有线程在循环，则发通知销毁
    if(pool->idle > 0)
      pool->ready.broadcast();

    //等待各进程销毁
    while(pool->counter > 0)
      pool->ready.wait();
  }

  pool->ready.unlock();
}
