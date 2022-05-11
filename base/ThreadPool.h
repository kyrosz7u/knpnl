//
// Created by 樱吹雪 on 2022/1/16.
//

#ifndef KHPNL_THREADPOOL_H
#define KHPNL_THREADPOOL_H

#include "Thread.h"
#include "Condition.h"
#include "Mutex.h"

#include <vector>
#include <queue>
#include <functional>

using namespace std;

namespace base {

class ThreadPool {
public:
    ThreadPool(int _workers);

    ThreadPool(int _workers, int _qSize);

    ~ThreadPool();

    void Start();

    void Stop();

    void AddTask(TaskFunc &&t);

    bool isRunning = false;
private:
    TaskFunc take();
    void runInThread();

    int workers;
    int qSize;
    Mutex mMutex;
    Condition isFull;
    Condition isEmpty;
    queue<TaskFunc> mTasks;
    vector<Thread *> mThreads;
};

}

#endif
