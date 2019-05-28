#include"Condition1.h"
#include<errno.h>
#include<sys/time.h>
#include<sys/types.h>

bool Reuzel::Condition::waitForSeconds(double seconds)
{
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);

    const int64_t kNanoSecondPerSecond = 1e9;
    int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondPerSecond);

    abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoseconds) / kNanoSecondPerSecond)；
    abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nanoseconds) % kNanoSecondPerSecond);

    MutexLock::UnassignGuard ug(mutex_);
    return ETIMEDOUT == pthread_cond_timedwait(&cond_, mutex_.getPthreadMutex(), &abstime);
}