//
// Created by 樱吹雪 on 2022/5/2.
//

#ifndef KHPNL_EVENT_TYPES_H
#define KHPNL_EVENT_TYPES_H

#include <vector>
#include <set>
#include <functional>
#include <memory>

using namespace std;

namespace event{
    // 定义处理事件读写的回调函数
    typedef std::function<void()> EventCallback;
    typedef std::function<void()> TimerCallback;
    typedef uint64_t Timer_t;
}

#endif //KHPNL_EVENT_TYPES_H
