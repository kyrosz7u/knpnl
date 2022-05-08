//
// Created by 樱吹雪 on 2022/5/2.
//

#ifndef KYROSWEBSERVER_EVENT_TYPES_H
#define KYROSWEBSERVER_EVENT_TYPES_H

#include <vector>
#include <set>
#include <functional>
#include <memory>

using namespace std;

namespace event{
class Channel;
    typedef std::vector<Channel *> ChannelList;
    // 定义处理事件读写的回调函数
    typedef std::function<void()> EventCallback;
    typedef std::function<void()> TimerCallback;
    typedef uint64_t Timer_t;
}

#endif //KYROSWEBSERVER_EVENT_TYPES_H
