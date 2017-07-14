#ifndef SERVER_COND_H
#define SERVER_COND_H

#include <cassert>
#include <pthread.h>
#include <base.h>


class MutexLock : noncopyable
{
    public: 
        MutexLock()
        {
            pthread_mutex_init(&mutex_, NULL);
        }
        ~MutexLock()
        {
            assert(holder_ == 0); //持有者
            pthread_mutex_destory(&mutex_);
        }
        bool isLockedByThisThread()
        {
            return holder_ == CurrentThread::tid(); //id其他实现
        }

        void assertLocked()
        {
            assert(isLockedByThisThread());
        }

        void lock()
        {
            pthread_mutex_lock(&mutex);
            holder_ = CurrentThread::tid();
        }

        void unlock()
        {
            holder_ = 0;
            pthread_mutex_unlock(&mutex);
        }

        pthread_mutex_t* getPthreadMutex()
        {
            return &mutex;
        }
        
    private:
        pthread_mutex_t mutex;
        pid_t holder_;
};


class MutexLockGuard : noncopyable
{
    public:
        explicit MutexLockGuard(MutexLock& mutex) : muxtex_(mutex)
        {
            mutex_.lock();
        }
        ~MutexLockGuard()
        {
            mutex_.unlock();
        }
    private:
        MutexLock mutex_;
};
#endif
