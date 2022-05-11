//
// Created by 樱吹雪 on 2022/5/3.
//

#include "Poller.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Types.h"
#include "base/Logger.h"
#include <cstring>
#include <assert.h>

using namespace base;
using namespace event;

Poller::Poller(EventLoop* loop)
:mLoop(loop)
{
    mEpollFd=epoll_create(5);
    eventList.resize(MAX_EVENT_NUM);
}

Poller::~Poller()
{
    close(mEpollFd);
}

void Poller::poll(ChannelList *ActiveChannels) {
    size_t sz=eventList.size();
    int ep_nums = epoll_wait(mEpollFd, &eventList[0], sz, -1);
    if (ep_nums > 0) {
        for (int i = 0; i < ep_nums; ++i) {
            Channel *channel = static_cast<Channel *>(eventList[i].data.ptr);
            channel->SetEvent(eventList[i].events);
            ActiveChannels->push_back(channel);
        }
        if (static_cast<size_t>(ep_nums) == sz) {
            eventList.resize(sz * 2);
        }
    }else if(ep_nums==-1){
        LOG_ERROR << sys_errlist[errno];
    }
}

void Poller::Add(Channel* channel)
{
    int cFd = channel->GetFd();

    assert(channelMap.find(cFd)==channelMap.end());
    channelMap[cFd] = channel;
    epoll_event ev;
    memset(&ev, 0, sizeof ev);
    ev.data.ptr = channel;
    epoll_ctl(mEpollFd, EPOLL_CTL_ADD, cFd, &ev);
    LOG_TRACE<<"EPoller Add: "<<cFd;
}

void Poller::Mod(Channel* channel, uint32_t op)
{
    int cFd = channel->GetFd();

    assert(channelMap.find(cFd)!=channelMap.end());
    channelMap[cFd] = channel;
    epoll_event ev;
    memset(&ev, 0, sizeof ev);
    ev.events = op;
    ev.data.ptr = channel;
    epoll_ctl(mEpollFd, EPOLL_CTL_MOD, cFd, &ev);
}


void Poller::Delete(Channel* channel)
{
    int cFd = channel->GetFd();

    assert(channelMap.find(cFd)!=channelMap.end());
    assert(channelMap[cFd]->isIdle());
    channelMap.erase(cFd);
    epoll_ctl(mEpollFd, EPOLL_CTL_DEL, cFd, 0);
}

bool Poller::Find(Channel *channel)
{
    if(channel!= nullptr)
    {
        if(channelMap.find(channel->GetFd())!=channelMap.end()){
            return true;
        }
    }else{
        return false;
    }

}







