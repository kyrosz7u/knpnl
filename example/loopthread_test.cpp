//
// Created by 樱吹雪 on 2022/5/8.
//

#include "event/LoopThread.h"

using namespace event;

unsigned long i;

void task()
{
    i++;
    LOG_INFO<<"i = "<<i;
}

int main() {
    LoopThread t1;

    sleep(1);
    EventLoop *loop = t1.GetLoop();


    int cnt=10;
    while (cnt--){
        loop->RunInLoop(task);
        usleep(500000);
    }

    t1.Stop();

    while(1);

    return 0;
}



