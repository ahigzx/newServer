#ifndef COND_H
#define COND_H
#include <pthread.h>
#include <base.h>
#include <mutex.h>


class Condition : noncopyable
{
    public:
        explicit Condition(MutexLock& mutex) : mutex_(mutex) 
        {
            pthread_cond_init(&cond_, NULL);
        }
        ~Condition()
        {
            pthread_cont_destory(&cond);
        }
        void wait()
        {
            pthread_cond_wait(&cond_, mutex_.getPthreadMutex());
        }
        void notify()
        {
            pthread_cond_signal(&cond_);
        }
        void notifyall()
        {
            pthread_cond_broadcast(&cond_);
        }
    private:
            MutexLock& mutex_;
            phthread_cond_t cond_;
}
#endif
