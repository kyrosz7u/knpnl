//
// Created by 樱吹雪 on 2022/5/4.
//

#include "Timer.h"

using namespace base;
using namespace event;

atomic<Timer_t> Timer::TimerCount(static_cast<Timer_t>(0));

Timestamp Timer::restart(){
    if(mInterval>0)
        mExpired = addTime(Timestamp::now(), mInterval);
    else
        mExpired = Timestamp::now();
    return mExpired;
}

Timestamp Timer::reset(Timestamp ts){
    mExpired=ts;
    return mExpired;
}

