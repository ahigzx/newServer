#ifndef THREAD_H
#define THREAD_H
#include <base.h>
#include <pthread.h>
#include <functional>
#include <atomic.h>

class Thread :: noncopyable
{
    typedef std::function<void ()> ThreadFunc;
    public:
        explicit Thread(const ThreadFunc&, const string &name = string());
        ~Thread();
        
        void start();
        void join();
        bool started() const{
            return started_;
        } 
        pid_t tid() const{
            return tid_;
        }
        /*
        const string& name() const{
            return name_;
        }
        static const numCreadted() const{
            return numCreated_.get();
        }
        */

    private:
        bool started_;
        pid_t tid_;
        ThreadFunc func_;
        pthread_t pthreadId_;
        //stirng name;
        //static Atomicint numCreated_;
}

#endif
