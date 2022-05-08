//
// Created by 樱吹雪 on 2022/5/4.
//

#ifndef KYROSWEBSERVER_TIMER_H
#define KYROSWEBSERVER_TIMER_H

#include "base/Timestamp.h"
#include "base/Copyable.h"
#include "Types.h"
#include <atomic>

using namespace base;

namespace event {
class Timer : public Noncopyable{
public:
    Timer(TimerCallback cb, Timestamp expired, double interval)
        : mCallback(cb),
          mExpired(expired),
          mInterval(interval),
          mRepeatable(interval > 0.0),
          mTimerId(TimerCount++)
          {}

    void run(){ mCallback();}
    Timestamp restart();
    Timestamp reset(Timestamp ts);
    inline bool isRepeat(){ return mRepeatable; }
    inline Timestamp GetExpire(){ return mExpired; }
    inline Timer_t GetTimerId(){ return mTimerId; }
    inline static Timer_t GetTimerCnt(){return TimerCount;}
private:
    TimerCallback mCallback;
    Timestamp mExpired;
    double mStartup;
    double mInterval;
    bool mRepeatable;
    Timer_t mTimerId;
    static atomic<Timer_t> TimerCount;
};

// 初始化不要放在.h文件中，不然会重复执行
// atomic<Timer_t> Timer::TimerCount(static_cast<Timer_t>(0));
}// namespace event

#endif //KYROSWEBSERVER_TIMER_H
