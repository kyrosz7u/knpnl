//
// Created by 樱吹雪 on 2022/5/5.
//

#include <cstdlib>
#include <atomic>
#include "event/EventLoop.h"
#include "base/Logger.h"


using namespace std;
using namespace event;

EventLoop loop;
atomic<int> i(0);

void foo()
{
    i++;
    if(i%1000==0){
        LOG_INFO<<"foo i:"<<i;
    }
}

/* 每隔100ms添加一个定时器
 * 定时器只运行一次，范围在10ms到100ms
 * 用来测试内存泄漏*/
void foo1()
{
    double t = (double)rand()/RAND_MAX;
    t=(0.9*t+0.1)/10;
    loop.RunAt(Timestamp::now().addTime(t), foo);
}

int main()
{
    for(int i=0;i<1000;i++)
    {
        loop.RunEvery(0.1,foo1);
        usleep(1000);
    }

    loop.Loop();
    return 0;
}

