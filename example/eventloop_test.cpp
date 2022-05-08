//
// Created by 樱吹雪 on 2022/5/5.
//

#include "event/EventLoop.h"
#include "event/Channel.h"
#include "base/Logger.h"
#include "base/Timestamp.h"
#include <sys/timerfd.h>
#include <cstring>

using namespace std;
using namespace event;

int createTimerfd();
void readTimerfd(int timerfd);
void resetTimerfd(int timerfd, Timestamp expiration);

EventLoop loop;
int timerfd=createTimerfd();
Channel TimerChannel(timerfd, &loop);

void task()
{
    LOG_INFO << "EventLoop is doing task";
}

int main()
{
    TimerChannel.SetReadCallback(std::bind(readTimerfd, timerfd));

    resetTimerfd(timerfd, Timestamp::now().addTime(0.8));
    TimerChannel.EnableRead();
    loop.Loop();
    return 0;
}

int createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                   TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
    {
        LOG_ERROR << "Failed in timerfd_create";
    }
    return timerfd;
}

void readTimerfd(int timerfd)
{
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    LOG_TRACE << "readTimerfd " << howmany << " at " << Timestamp::now().toString();
    if (n != sizeof howmany)
    {
        LOG_ERROR << "readTimerfd reads " << n << " bytes instead of 8";
    }
    resetTimerfd(timerfd, Timestamp::now().addTime(0.8));
    loop.RunInLoop(task);
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.GetMicroSecondEpoch()
                           - Timestamp::now().GetMicroSecondEpoch();
    if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
            microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
            (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

void resetTimerfd(int timerfd, Timestamp expiration)
{
    // wake up Loop by timerfd_settime()
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&newValue, 0,sizeof newValue);
    memset(&oldValue, 0,sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret)
    {
        LOG_ERROR << "timerfd_settime()";
    }
}
