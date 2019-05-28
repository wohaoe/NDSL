#include "condition.h"

Condition::Condition(const pthread_mutexattr_t *mutexAttr,
                     const pthread_condattr_t  *condAttr){
  pthread_mutex_init(&m_mutex, mutexAttr);
  pthread_cond_init(&m_cond, condAttr);
}

Condition::~Condition(){
  pthread_mutex_destroy(&m_mutex);
  pthread_cond_destroy(&m_cond);
}

int Condition::signal(){
  return pthread_cond_signal(&m_cond);
}

int Condition::broadcast(){
  return pthread_cond_broadcast(&m_cond);
}

int Condition::wait(){
  return pthread_cond_wait(&m_cond, &m_mutex);
}

int Condition::timedwait(int seconds){
  struct timespec abstime;
  clock_gettime(CLOCK_REALTIME, &abstime);
  abstime.tv_sec += seconds;
  return pthread_cond_timedwait(&m_cond, &m_mutex, &abstime);
}


int Condition::lock(){
  return pthread_mutex_lock(&m_mutex);
}

int Condition::unlock(){
  return pthread_mutex_unlock(&m_mutex);
}

int myfun()
{
  
}