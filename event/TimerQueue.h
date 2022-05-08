//
// Created by 樱吹雪 on 2022/5/4.
//

#ifndef KYROSWEBSERVER_TIMERQUEUE_H
#define KYROSWEBSERVER_TIMERQUEUE_H

#include "EventLoop.h"
#include "Timer.h"
#include "Types.h"
#include "base/Timestamp.h"
#include <set>
#include <map>
#include <memory>

using namespace std;
using namespace base;

namespace event {

int createTimerfd();
void readTimerfd(int timerfd);
void resetTimerfd(int timerfd, Timestamp expiration);

/*  为了提高性能，采用Timer*的数据结构管理定时器，
    弊端是需要手动管理Timer的创建和析构    */
typedef pair<Timestamp, Timer*> TimerEntry;
typedef set<TimerEntry> TimerSet;
/*  因为Timer中TimerId靠原子操作保证了唯一性，
 *  所以map的key不会有重复  */
typedef map<uint64_t, Timer*> TimerIdMap;
typedef vector<TimerEntry> TimerList;

class TimerQueue : public Noncopyable{
public:
    TimerQueue(EventLoop *loop);
    ~TimerQueue();
    // 供外部线程调用的三个函数说明
    /*  AddTimer：添加新的定时器，参数列表如下：
     *  TimerCallback cb：定时器触发时调用的回调函数
     *  Timestamp ts：定时器触发的时间
     *  double interval：触发间隔时间（单位s），
     *  =0为一次性的，>0时可以每隔interval触发一次
     *  返回值：新建的定时器id*/
    Timer_t AddTimer(TimerCallback cb, Timestamp ts, double interval);
    /*  Remove：根据id删除定时器   */
    void Remove(Timer_t timerId);
    /*  ResetTimer：重置定时器
     *  Timer_t timerId：需要操作的定时器
     *  Timestamp ts：新的时间戳
     *  bool &isReset：是否成功的标志    */
    void ResetTimer(Timer_t timerId, Timestamp ts, bool &isReset);
    int GetTimerFd(){ return mTimerfd; }
    void handleRead();
private:
    // 这些成员函数只能在mLoop中运行，所以设置为私有
    // 跨线程调用需要通过AddTimer，和Remove
    void insert(Timer *timer);
    void deleteTimer(Timer *timer);
    void resetTimer(Timer_t id, Timestamp ts, bool &isReset);
    TimerList findExpiredTimer(Timestamp when);

    int mTimerfd;
    EventLoop *mLoop;
    // 管理队列中的额定时器
    TimerSet mTimerSet;
    TimerIdMap mTimerIdMap;


};
}//namespace event

#endif //KYROSWEBSERVER_TIMERQUEUE_H
