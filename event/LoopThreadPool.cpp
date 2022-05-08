//
// Created by 樱吹雪 on 2022/5/7.
//

#include "LoopThreadPool.h"
#include "LoopThread.h"

using namespace event;

LoopThreadPool::LoopThreadPool(int nums)
{
    mThreadNums = nums;
    mLoopList.resize(nums);
    for (int i = 0; i < nums; ++i) {
        mLoopList[i].reset(new LoopThread());
    }
}

LoopThreadPool::~LoopThreadPool()
{
    for (int i = 0; i < mThreadNums; ++i) {
        mLoopList[i]->Stop();
    }
}

EventLoop* LoopThreadPool::getNextLoop()
{
    static int index=0;
    if(index>=mThreadNums){
        index=0;
    }
    return mLoopList[index]->GetLoop();
}



