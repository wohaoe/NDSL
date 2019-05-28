//#include "my_err.h"

#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>   //read write close
#include <stdlib.h>
#include <cstdio>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <cstring>    //bzero
#include <arpa/inet.h> //inet_pton
#include <malloc.h>

class Condition{
public:
  Condition(const pthread_mutexattr_t *mutexAttr = NULL,
          const pthread_condattr_t  *condAttr = NULL);
  ~Condition();

  int signal();
  int broadcast();
  int wait();
  int timedwait(int seconds);

  int lock();
  int unlock();

private:
  pthread_mutex_t m_mutex;
  pthread_cond_t m_cond;
};
