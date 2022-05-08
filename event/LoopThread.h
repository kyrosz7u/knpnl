//
// Created by 樱吹雪 on 2022/5/7.
//

#ifndef KYROSWEBSERVER_LOOPTHREAD_H
#define KYROSWEBSERVER_LOOPTHREAD_H

#include "EventLoop.h"
#include "base/Thread.h"
#include <memory>

using namespace base;
using namespace event;

namespace event {
// 单个Loop线程
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

#endif //KYROSWEBSERVER_LOOPTHREAD_H
