//
// Created by 樱吹雪 on 2022/5/3.
//

#ifndef KYROSWEBSERVER_EVENT_EVENTLOOP_H
#define KYROSWEBSERVER_EVENT_EVENTLOOP_H

#include "Types.h"

#include "base/Mutex.h"
#include "base/Types.h"
#include "base/Thread.h"
#include "base/Logger.h"

#include <vector>
#include <memory>
#include <assert.h>

using namespace std;
using namespace base;

namespace event {

class Channel;
class Poller;
class TimerQueue;

int createEventfd();

class EventLoop {
public:
    EventLoop();

    ~EventLoop();

    void Loop();

    void Quit();

    pid_t getTid() const;

    // 添加任务
    void RunInLoop(TaskFunc t);

    void QueueInLoop(TaskFunc taskFunc);

    // 添加、修改和删除Channel的方法
    void ModChannel(Channel *channel, uint32_t op);

    void AddChannel(Channel *channel);

    void DeleteChannel(Channel *channel);

    bool HasChannel(Channel *channel);

    // 调用定时器执行任务
    void RunAt(Timestamp when, TimerCallback cb);

    void RunAfter(double when, TimerCallback cb);

    void RunEvery(double when, TimerCallback cb);

    inline bool IsInLoopThread(){ return mTreadId == CurrentThread::tid();}

    inline void AssertInLoop(){assert(IsInLoopThread());}

private:
    void doTask();

    void wakeupRead();

    void wakeup();

    unique_ptr<Poller> mPoller;
    unique_ptr<TimerQueue> mTimers;
    unique_ptr<Channel> mTimerCh;
    unique_ptr<Channel> mWakeupCh;
    int mTimerFd;
    int mWakeupFd;
    bool isWakeup;
    bool mQuit;     //linux下bool变量的操作默认是原子的
    pid_t mTreadId;
    Mutex mMutex;
    vector<TaskFunc> mTasks;
};
}//namespace event

#endif //KYROSWEBSERVER_EVENTLOOP_H
