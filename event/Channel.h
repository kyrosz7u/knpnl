//
// Created by 樱吹雪 on 2022/5/3.
//

#ifndef KYROSWEBSERVER_CHANNEL_H
#define KYROSWEBSERVER_CHANNEL_H

#include "base/Copyable.h"
#include "Types.h"
#include <sys/epoll.h>
#include <assert.h>

namespace event {
class EventLoop;
class Channel{
public:
    Channel(int fd, EventLoop *loop);
    ~Channel();

    int GetFd(){ return mFd; }

    const EventLoop *GetLoop(){return mLoop;}
    bool isIdle(){return mEvent==kNoneEvent;}

    void EnableRead();
    void DisableRead();
    void EnableWrite();
    void DisableWrite();
    void DisableAll();
    void Remove();

    void SetEvent(int event){ rEvent = event; }
    void HandleEvent();

    void SetReadCallback(EventCallback cb){ readCallback = std::move(cb); }
    void SetWriteCallback(EventCallback cb){ writeCallback = std::move(cb); }
    void SetErrorCallback(EventCallback cb){ errorCallback = std::move(cb); }
    void SetCloseCallback(EventCallback cb){ closeCallback = std::move(cb); }

private:
    static const uint32_t kNoneEvent;
    static const uint32_t kReadEvent;
    static const uint32_t kWriteEvent;
    static const uint32_t kEPollET;

    EventCallback readCallback;
    EventCallback writeCallback;
    EventCallback errorCallback;
    EventCallback closeCallback;

    bool isHandling;
    bool isInLoop;
    uint32_t rEvent;
    uint32_t mEvent;
    EventLoop *mLoop;
    int mFd;
};
}//namespace event

#endif //KYROSWEBSERVER_CHANNEL_H
