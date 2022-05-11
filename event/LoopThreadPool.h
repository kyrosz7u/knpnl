//
// Created by 樱吹雪 on 2022/5/7.
//

#ifndef KHPNL_LOOPTHREADPOOL_H
#define KHPNL_LOOPTHREADPOOL_H

#include "LoopThread.h"

#include <vector>
#include <memory>

using namespace std;

namespace event {
// 基于EventLoop的线程池
class LoopThreadPool {
public:
    LoopThreadPool(int nums);

    ~LoopThreadPool();

    EventLoop *getNextLoop();

private:
    int mThreadNums;
    vector<unique_ptr<LoopThread>> mLoopList;
};
}// namespace event

#endif //KHPNL_LOOPTHREADPOOL_H
