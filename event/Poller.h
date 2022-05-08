//
// Created by 樱吹雪 on 2022/5/3.
//

#ifndef KYROSWEBSERVER_POLLER_H
#define KYROSWEBSERVER_POLLER_H

#include "EventLoop.h"
#include "Channel.h"
#include <sys/epoll.h>
#include <vector>
#include <map>

namespace event {
#define MAX_EVENT_NUM 1000
class Poller {
public:
    Poller(EventLoop* loop)
    :mLoop(loop)
    {
        mEpollFd=epoll_create(5);
        eventList.resize(MAX_EVENT_NUM);
    }
    ~Poller()
    {
        close(mEpollFd);
    }
    void Mod(Channel *channel, uint32_t op);
    void Add(Channel *channel);
    void Delete(Channel *channel);
    bool Find(Channel *channel);
    void poll(vector<Channel *> *ActiveChannels);
private:
    EventLoop *mLoop;
    epoll_event epollEvent[MAX_EVENT_NUM];
    vector<epoll_event> eventList;
    map<int, Channel *> channelMap;
    int mEpollFd;
};
}//namespace event

#endif //KYROSWEBSERVER_POLLER_H
