//
// Created by 樱吹雪 on 2022/5/3.
//

#ifndef KHPNL_POLLER_H
#define KHPNL_POLLER_H

#include <sys/epoll.h>
#include <vector>
#include <map>

using namespace std;

namespace event {

class EventLoop;
class Channel;
#define MAX_EVENT_NUM 1000

typedef std::vector<Channel *> ChannelList;
class Poller {
public:
    Poller(EventLoop* loop);
    ~Poller();
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

#endif //KHPNL_POLLER_H
