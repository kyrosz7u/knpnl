//
// Created by 樱吹雪 on 2022/5/4.
//

#include "TimerQueue.h"
#include "base/Logger.h"
#include <sys/timerfd.h>
#include <cstring>
#include <unistd.h>
#include <assert.h>

using namespace event;

TimerQueue::TimerQueue(EventLoop *loop)
{
    mTimerfd = createTimerfd();
    mLoop=loop;
}

TimerQueue::~TimerQueue()
{
    for(auto &te:mTimerSet){
        delete te.second;
    }
}

// 注意：该函数在跨线程调用时不保证新增的timer立即生效
Timer_t TimerQueue::AddTimer(TimerCallback cb, Timestamp ts, double interval)
{
    Timer *timer = new Timer(cb, ts, interval);
    if(mLoop->IsInLoopThread()){
        insert(timer);
    }else{
        mLoop->RunInLoop(std::bind(&TimerQueue::insert, this, timer));
    }
    return timer->GetTimerId();
}

void TimerQueue::Remove(Timer_t timerId)
{
    if(mTimerIdMap.find(timerId)!=mTimerIdMap.end())
    {
        if(mLoop->IsInLoopThread()){
            deleteTimer(mTimerIdMap[timerId]);
        }else{
            mLoop->RunInLoop(std::bind(&TimerQueue::deleteTimer,
                                       this, mTimerIdMap[timerId]));
        }
    }
}

void TimerQueue::ResetTimer(Timer_t timerId, Timestamp ts, bool &isReset)
{
    if(mLoop->IsInLoopThread()){
        resetTimer(timerId, ts, isReset);
    }else{
        mLoop->RunInLoop(
                std::bind(&TimerQueue::resetTimer, this, timerId, ts, isReset));
    }

}

// 定时器触发回调函数
void TimerQueue::handleRead()
{
    TimerList expiredTimer = findExpiredTimer(Timestamp::now());
    for(TimerEntry &te:expiredTimer){
        Timer *timer = te.second;
        timer->run();
        // 重置需要循环触发的定时器
        if(timer->isRepeat()){
            Timestamp ts = timer->restart();
            mTimerSet.insert(TimerEntry(ts,timer));
            mTimerIdMap[timer->GetTimerId()]=timer;
        }else{
            delete timer;
        }
    }
    assert(mTimerSet.size()==mTimerIdMap.size());
    // 最后设置好timerfd
    Timestamp ts = mTimerSet.begin()->first;
    resetTimerfd(mTimerfd, ts);
}

TimerList TimerQueue::findExpiredTimer(Timestamp when)
{
    TimerList expiredlist;

    /*  获取第一个未到期的Timer的迭代器
        lower_bound的含义是返回第一个值>=pivot的元素的iterator，
        由于pivot->second已经被设置为了无穷大，
        所以返回的是第一个时间戳大于when的定时器*/
    TimerEntry pivot(when, reinterpret_cast<Timer*>(UINTPTR_MAX));
    TimerSet::iterator end = mTimerSet.lower_bound(pivot);
    assert(end==mTimerSet.end()||when<end->first);
    // 使用back_inserter，可以避免expiredlist的大小小于mTimerSet导致报错
    std::copy(mTimerSet.begin(),end,back_inserter(expiredlist));
    mTimerSet.erase(mTimerSet.begin(),end);

    for(TimerEntry& te: expiredlist){
        mTimerIdMap.erase(te.second->GetTimerId());
    }
    assert(mTimerSet.size()==mTimerIdMap.size());

    return expiredlist;  // Release环境下编译器会进行RVO优化
}

void TimerQueue::insert(Timer *timer)
{
    Timestamp ts = timer->GetExpire();
    Timer_t timerId = timer->GetTimerId();
    assert(mTimerIdMap.find(timerId)==mTimerIdMap.end());
    mTimerSet.insert(TimerEntry(ts,timer));
    mTimerIdMap[timerId]=timer;
    // 最早响应的定时器更新，需要调整timerfd设置
    if(mTimerSet.begin()->second==timer){
        resetTimerfd(mTimerfd, ts);
    }
    assert(mTimerSet.size()==mTimerIdMap.size());
}

void TimerQueue::deleteTimer(Timer *timer)
{
    bool isHeadChange= false;
    assert(timer != nullptr);
    TimerEntry temp(timer->GetExpire(), timer);
    // 最早响应的定时器更新，需要调整timerfd设置
    if(mTimerSet.begin()->second==timer){
        isHeadChange= true;
    }
    mTimerIdMap.erase(timer->GetTimerId());
    mTimerSet.erase(temp);
    delete timer;
    if(isHeadChange){
        Timestamp ts = mTimerSet.begin()->first;
        resetTimerfd(mTimerfd, ts);
    }
    assert(mTimerSet.size()==mTimerIdMap.size());
}



void TimerQueue::resetTimer(Timer_t id, Timestamp ts, bool &isReset){
    // 没找到可能是已经到期，然后删除掉了
    // 这种情况下isReset=false
    if(mTimerIdMap.find(id)!=mTimerIdMap.end()){
        Timer *timer = mTimerIdMap[id];
        TimerEntry te(timer->GetExpire(),timer);
        mTimerSet.erase(te);
        Timestamp newts = timer->reset(ts);
        te.first = newts;
        mTimerSet.insert(te);
        assert(mTimerSet.size()==mTimerIdMap.size());
        if(mTimerSet.begin()->second==timer){
            resetTimerfd(mTimerfd, newts);
        }
        isReset= true;
    }else{
        isReset= false;
    }
}

namespace event {
int createTimerfd() {
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                   TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        LOG_ERROR << "Failed in timerfd_create";
    }
    return timerfd;
}

void readTimerfd(int timerfd) {
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    LOG_TRACE << "readTimerfd " << howmany << " at " << Timestamp::now().toString();
    if (n != sizeof howmany) {
        LOG_ERROR << "readTimerfd reads " << n << " bytes instead of 8";
    }
}

struct timespec howMuchTimeFromNow(Timestamp when) {
    int64_t microseconds = when.GetMicroSecondEpoch()
                           - Timestamp::now().GetMicroSecondEpoch();
    if (microseconds < 100) {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
            microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
            (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

void resetTimerfd(int timerfd, Timestamp expiration) {
    // wake up Loop by timerfd_settime()
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&newValue, 0, sizeof newValue);
    memset(&oldValue, 0, sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret) {
        LOG_ERROR << "timerfd_settime()";
    }
}
} //namespace event
