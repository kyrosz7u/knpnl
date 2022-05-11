//
// Created by 樱吹雪 on 2022/5/3.
//

#include "Channel.h"
#include "EventLoop.h"
#include "base/Logger.h"

using namespace event;

const uint32_t Channel::kNoneEvent = 0;
const uint32_t Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const uint32_t Channel::kWriteEvent = EPOLLOUT;
const uint32_t Channel::kEPollET = EPOLLET;

Channel::Channel(int fd, EventLoop *loop)
{
    mFd = fd;
    mLoop = loop;
    assert(mLoop!= nullptr);
//    loop->AddChannel(this);
    isInLoop= true;
    isHandling = false;
    isGuarded = false;
}

Channel::~Channel()
{
    assert(!isHandling);
    assert(!isInLoop);
}

void Channel::HandleEvent()
{
    if (isGuarded){
        // 延长mGuardPtr指向的底层对象的生命期，确保执行回调函数时不会被析构
        shared_ptr<void> sp = mGuardPtr.lock();
        if(sp){
            HandleEventWithGuard();
        }
    }else{
        HandleEventWithGuard();
    }
}

void Channel::HandleEventWithGuard()
{
    isHandling = true;

    /* 这种情况通常是对方的socket已经close
     * !(rEvent&EPOLLIN)是为了确定缓冲区已经没有新数据*/
    if((rEvent&EPOLLHUP)&&!(rEvent&EPOLLIN))
    {
        LOG_DEBUG<<"fd = " << mFd << " Channel::HandleEvent() EPOLLHUP";
        if(closeCallback) closeCallback();
    }else if(rEvent&EPOLLERR)
    {
        LOG_ERROR<<"fd = " << mFd << " Channel::HandleEvent() ERROR";
        if(errorCallback) errorCallback();
    }else if(rEvent&(EPOLLRDHUP|EPOLLPRI|EPOLLIN))
    {
        if(readCallback) readCallback();
    }else if(rEvent&EPOLLOUT)
    {
        if(writeCallback) writeCallback();
    }

    isHandling = false;
}

void Channel::Remove()
{
    assert(!isHandling);
    mLoop->DeleteChannel(this);
    isInLoop = false;
}

void Channel::Guard(const shared_ptr<void> &sp)
{
    mGuardPtr=sp;
    isGuarded= true;
}

void Channel::UnGuard()
{
    mGuardPtr.reset();
    isGuarded= false;
}

void Channel::EnableRead()
{
    mEvent= kReadEvent | kEPollET;
    mLoop->ModChannel(this,mEvent);
}

void Channel::DisableRead()
{
    mEvent&=~kReadEvent;
    mLoop->ModChannel(this,mEvent);
}

void Channel::EnableWrite()
{
    mEvent= kWriteEvent | kEPollET;
    mLoop->ModChannel(this,mEvent);
}

void Channel::DisableWrite()
{
    mEvent&=~kWriteEvent;
    mLoop->ModChannel(this,mEvent);
}

void Channel::DisableAll()
{
    mEvent=kNoneEvent;
    mLoop->ModChannel(this,mEvent);
}
