//
// Created by 樱吹雪 on 2022/5/8.
//

#include <atomic>
#include "event/LoopThreadPool.h"

atomic<unsigned long> i;

void task(EventLoop* loop)
{
    i++;
    assert(CurrentThread::Tid==loop->getTid());

    if(i%1000)
        LOG_INFO<<"i = "<<i;
}

int main()
{
    LoopThreadPool threadpool(4);

    sleep(1);

    while(1)
    {
        EventLoop *loop=threadpool.getNextLoop();
        loop->RunInLoop(std::bind(task, loop));
        usleep(300000);
    }

    return 0;
}

