//
// Created by 樱吹雪 on 2022/5/7.
//

#include "LoopThread.h"

using namespace event;

void LoopThread::threadFunc()
{
    EventLoop loop;
    mLoop=&loop;
    loop.Loop();
}

EventLoop* LoopThread::GetLoop()
{
    return mLoop;
}

void LoopThread::Stop()
{
    mLoop->Quit();
    Thread::join();
    mStop = true;
}


