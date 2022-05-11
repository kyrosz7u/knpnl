//
// Created by 樱吹雪 on 2022/5/7.
//

#ifndef KHPNL_LOOPTHREAD_H
#define KHPNL_LOOPTHREAD_H

#include "EventLoop.h"
#include "base/Thread.h"
#include <memory>

using namespace base;
using namespace event;

namespace event {
/* 包含Loop线程的实现
 * 为了安全不允许在外部访问父类成员函数   */
class LoopThread : protected Thread {
public:
    LoopThread()
    : Thread(std::bind(&LoopThread::threadFunc, this),
            "LoopThread"),
            mLoop(nullptr),
            mStop(false)
    {
        Thread::start();
    }

    /* 在对象析构后才会尝试终止线程
     * 线程退出之前要让loop执行完任务    */
    ~LoopThread()
    {
        mLoop->Quit();
    }

    EventLoop* GetLoop();

    void Stop();

    bool isStop(){ return mStop; }

private:
    void threadFunc();

    bool mStop;
    EventLoop *mLoop;
};
}//namespace event

#endif //KHPNL_LOOPTHREAD_H
