#include "str_echo.h"
#include"ServerInit.h"

static void *doit(void *);

int main(int argc, char const *argv[]) {
  int listenfd = initserver(SERVER);
  int *connfd;
  // pthread_t tid;

  //初始化线程池
  threadpool pool;
  threadpool_init(&pool, 3);

  for(;;){
    connfd = (int*)malloc(sizeof(int));
    *connfd = Accept(listenfd, NULL, NULL);
    // printf("connfd = %d\n", (*connfd));
    // pthread_create(&tid, NULL, &doit, connfd)
    threadpool_add_task(&pool, doit, connfd);
  }
  return 0;
}

static void *doit(void *arg){
  // printf("this is doit\n");

  int connfd = *( (int*) arg);
  // free(arg);

  // printf("doit connfd = %d\n", connfd);

  pthread_detach(pthread_self());
  str_echo(connfd);
  close(connfd);
}
