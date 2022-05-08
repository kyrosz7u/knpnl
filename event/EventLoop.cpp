//
// Created by 樱吹雪 on 2022/5/3.
//

#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "TimerQueue.h"
#include "Types.h"

#include <unistd.h>
#include <sys/eventfd.h>

using namespace base;
using namespace event;

__thread EventLoop* ThreadLoop;

EventLoop::EventLoop()
{
    mTreadId= CurrentThread::tid();
    mPoller.reset(new Poller(this));
    mTimers.reset(new TimerQueue(this));

    mTimerFd=mTimers->GetTimerFd();
    mTimerCh.reset(new Channel(mTimerFd,this));
    mTimerCh->SetReadCallback(std::bind(&TimerQueue::handleRead,mTimers.get()));
    mTimerCh->EnableRead();

    mWakeupFd = createEventfd();
    mWakeupCh.reset(new Channel(mWakeupFd,this));
    mWakeupCh->SetReadCallback(std::bind(&EventLoop::wakeupRead, this));
    mWakeupCh->EnableRead();

    isWakeup = false;
    mQuit= false;
}

EventLoop::~EventLoop()
{
    mTimerCh->DisableAll();
    mWakeupCh->DisableAll();
    mTimerCh->Remove();
    mWakeupCh->Remove();
    close(mTimerFd);
    close(mWakeupFd);
}

void EventLoop::Loop(){
    while (!mQuit) {
        ChannelList activeChannel;
        mPoller->poll(&activeChannel);
        LOG_TRACE<<activeChannel.size()<<" events happened";
        isWakeup = true;
        for(Channel* ch :activeChannel){
            ch->HandleEvent();
        }
        if(!mTasks.empty())
            doTask();
    }
}

void EventLoop::DeleteChannel(Channel* channel){
    AssertInLoop();
    mPoller->Delete(channel);
}

void EventLoop::AddChannel(Channel* channel){
    AssertInLoop();
    LOG_TRACE << "AddChannel: "<<channel;
    mPoller->Add(channel);
}

void EventLoop::ModChannel(Channel* channel, uint32_t op){
    AssertInLoop();
    mPoller->Mod(channel,op);
}

bool EventLoop::HasChannel(Channel* channel){
    return mPoller->Find(channel);
}

void EventLoop::RunAt(Timestamp when, TimerCallback cb)
{
    mTimers->AddTimer(cb, when, 0);
}

void EventLoop::RunAfter(double when, TimerCallback cb)
{
    mTimers->AddTimer(cb, Timestamp::now().addTime(when), 0);
}

void EventLoop::RunEvery(double when, TimerCallback cb)
{
    mTimers->AddTimer(cb, Timestamp::now().addTime(when), when);
}

void EventLoop::RunInLoop(TaskFunc t){
    if(IsInLoopThread()){
        t();
    }else{
        QueueInLoop(std::move(t));
    }
}

void EventLoop::QueueInLoop(TaskFunc t){
    {
        MutexLock lock(mMutex);
        mTasks.push_back(std::move(t));
    }
    // 减小wakeup次数
    if(isWakeup== false){
        wakeup();
    }
}

void EventLoop::Quit()
{
    mQuit = true;
    if(isWakeup== false){
        wakeup();
    }
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(mWakeupFd, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::wakeupRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(mWakeupFd, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::doTask(){
    vector<TaskFunc> tasks;
    {
        MutexLock lock(mMutex);
        tasks.swap(mTasks);
        isWakeup = false;
    }

    for(TaskFunc t:tasks){
        t();
    }
}

pid_t EventLoop::getTid() const {
    return mTreadId;
}

namespace event {
int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_ERROR << "Failed in eventfd";
        abort();
    }
    return evtfd;
}
}// namespace event


