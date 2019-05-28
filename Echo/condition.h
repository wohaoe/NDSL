#include "my_unp.h"

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
