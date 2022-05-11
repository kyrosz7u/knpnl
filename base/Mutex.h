//
// Created by 樱吹雪 on 2022/1/13.
//

#ifndef KHPNL_MUTEX_H
#define KHPNL_MUTEX_H

#include "Copyable.h"
#include <pthread.h>

namespace base {

class Mutex : public Noncopyable{
public:
    Mutex();

    ~Mutex();

    void Lock();

    void Unlock();

private:
    friend class Condition;

    pthread_mutex_t mMutex;
    bool isLocked = false;
};

class MutexLock {
public:
    MutexLock(Mutex &mutex)
            : _Mutex(mutex) {
        _Mutex.Lock();
    }

    ~MutexLock() {
        _Mutex.Unlock();
    }

private:
    Mutex &_Mutex;
};
}//namespace base {

#endif //LINUX_SYS_MUTEX_H


